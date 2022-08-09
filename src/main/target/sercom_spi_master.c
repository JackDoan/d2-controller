#include "interrupts.h"
#include "sercom_spi_master.h"

/* SERCOM1 clk freq value for the baud calculation */
#define SERCOM_Frequency      (48000000UL)/2 //todo fix board

/* SERCOM1 SPI baud value for 1000000 Hz baud rate */
#define SERCOM_SPIM_BAUD_VALUE         (23UL)*2 //todo fix board

/*Global object to save SPI Exchange related data  */
static SPI_OBJECT sercomSPIObj;

inline static void spi_sync(sercom_registers_t* sercom) {
    while((sercom->SPIM.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }
}

void SERCOM_SPI_Initialize(sercom_registers_t* sercom) {
    /* Instantiate the SERCOM1 SPI object */
    sercomSPIObj.callback = NULL ;
    sercomSPIObj.transferIsBusy = false ;
    sercomSPIObj.txSize = 0U;
    sercomSPIObj.rxSize = 0U;

    /* Selection of the Character Size and Receiver Enable */
    sercom->SPIM.SERCOM_CTRLB =
            SERCOM_SPIM_CTRLB_CHSIZE_8_BIT |
            SERCOM_SPIM_CTRLB_RXEN_Msk;
    spi_sync(sercom);

    /* Selection of the Baud Value */
    sercom->SPIM.SERCOM_BAUD = (uint8_t)SERCOM_SPIM_BAUD_BAUD(SERCOM_SPIM_BAUD_VALUE);

    /* Configure Data Out Pin Out , Master Mode,
     * Data In and Pin Out,Data Order and Standby mode if configured
     * and Selection of the Clock Phase and Polarity and Enable the SPI Module
     */
    sercom->SPIM.SERCOM_CTRLA =
            SERCOM_SPIM_CTRLA_MODE_SPI_MASTER |
            SERCOM_SPIM_CTRLA_DORD_LSB |
            SERCOM_SPIM_CTRLA_CPOL_IDLE_LOW |
            SERCOM_SPIM_CTRLA_CPHA_TRAILING_EDGE |
            SERCOM_SPIM_CTRLA_DOPO_0x2 | /*sck on pad 1, miso on pad3 */
            SERCOM_SPIM_CTRLA_DIPO_PAD0 | /*mosi on pad 0*/
            SERCOM_SPIM_CTRLA_ENABLE_Msk;

    spi_sync(sercom);
}

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_TransferSetup(SPI_TRANSFER_SETUP *setup,
                                                uint32_t spiSourceClock);

 Summary:
    Configure SERCOM SPI operational parameters at run time.

  Description:
    This function allows the application to change the SERCOM SPI operational
    parameter at run time. The application can thus override the MHC defined
    configuration for these parameters. The parameter are specified via the
    SPI_TRANSFER_SETUP type setup parameter. Each member of this parameter
    should be initialized to the desired value.

    The application may feel need to call this function in situation where
    multiple SPI slaves, each with different operation parameters, are connected
    to one SPI master. This function can thus be used to setup the SPI Master to
    meet the communication needs of the slave.

    Calling this function will affect any ongoing communication. The application
    must thus ensure that there is no on-going communication on the SPI before
    calling this function.

*/

bool SERCOM_SPI_TransferSetup(sercom_registers_t* sercom, SPI_TRANSFER_SETUP *setup, uint32_t spiSourceClock)
{
    uint32_t baudValue = 0U;

    bool statusValue = false;

    if(spiSourceClock == 0U)
    {
        /* Fetch Master Clock Frequency directly */
        spiSourceClock = SERCOM_Frequency;
    }

    /* Disable the SPI Module */
    sercom->SPIM.SERCOM_CTRLA &= ~(SERCOM_SPIM_CTRLA_ENABLE_Msk);

    spi_sync(sercom);

    if(setup != NULL)
    {
        if (setup->clockFrequency <= spiSourceClock/2U)
        {
            baudValue = (spiSourceClock/(2U*(setup->clockFrequency))) - 1U;

            /* Set the lowest possible baud */
            if (baudValue >= 255U)
            {
                baudValue = 255U;
            }

            /* Selection of the Clock Polarity and Clock Phase */
            sercom->SPIM.SERCOM_CTRLA &= ~(SERCOM_SPIM_CTRLA_CPOL_Msk | SERCOM_SPIM_CTRLA_CPHA_Msk);
            sercom->SPIM.SERCOM_CTRLA |= (uint32_t)setup->clockPolarity | (uint32_t)setup->clockPhase;

            /* Selection of the Baud Value */
            sercom->SPIM.SERCOM_BAUD = (uint8_t)baudValue;

            /* Selection of the Character Size */
            sercom->SPIM.SERCOM_CTRLB &= ~SERCOM_SPIM_CTRLB_CHSIZE_Msk;
            sercom->SPIM.SERCOM_CTRLB |= (uint32_t)setup->dataBits;

            spi_sync(sercom);

            statusValue = true;
        }
    }

    /* Enabling the SPI Module */
    sercom->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_ENABLE_Msk;

    spi_sync(sercom);

    return statusValue;
}


// *****************************************************************************
/* Function:
    void SERCOM_SPI_CallbackRegister(const SERCOM_SPI_CALLBACK* callBack,
                                                    uintptr_t context);

  Summary:
    Allows application to register callback with PLIB.

  Description:
    This function allows application to register an event handling function
    for the PLIB to call back when requested data exchange operation has
    completed or any error has occurred.
    The callback should be registered before the client performs exchange
    operation.
    At any point if application wants to stop the callback, it can use this
    function with "callBack" value as NULL.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

void SERCOM_SPI_CallbackRegister(SERCOM_SPI_CALLBACK callBack, uintptr_t context )
{
    sercomSPIObj.callback = callBack;
    sercomSPIObj.context = context;
}

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_IsBusy(void);

  Summary:
    Returns transfer status of SERCOM SERCOM1SPI.

  Description:
    This function ture if the SERCOM SERCOM1SPI module is busy with a transfer. The
    application can use the function to check if SERCOM SERCOM1SPI module is busy
    before calling any of the data transfer functions. The library does not
    allow a data transfer operation if another transfer operation is already in
    progress.

    This function can be used as an alternative to the callback function when
    the library is operating interrupt mode. The allow the application to
    implement a synchronous interface to the library.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

bool SERCOM_SPI_IsBusy(sercom_registers_t* sercom)
{
    if ((sercomSPIObj.txSize == 0U) && (sercomSPIObj.rxSize == 0U)) {
        /* This means no transfer has been requested yet; hence SPI is not busy. */
        return false;
    }
    /* if transmit is not complete or if the state flag is not set, SPI is busy */
    return SERCOM_SPI_IsTransmitterBusy(sercom) || sercomSPIObj.transferIsBusy;
}

bool SERCOM_SPI_IsTransmitterBusy(sercom_registers_t* sercom)
{
    return (sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == 0U;
}

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_WriteRead (void* pTransmitData, size_t txSize
                                        void* pReceiveData, size_t rxSize);

  Summary:
    Write and Read data on SERCOM SERCOM1 SPI peripheral.

  Description:
    This function transmits "txSize" number of bytes and receives "rxSize"
    number of bytes on SERCOM SERCOM1 SPI module. Data pointed by pTransmitData is
    transmitted and received data is saved in the location pointed by
    pReceiveData. The function will transfer the maximum of "txSize" or "rxSize"
    data units towards completion.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    transferring all the data.  This indicates that the operation has been
    completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    transmit and receive buffer  are ownerd by the library until the data
    transfer is complete and should not be modified by the application till the
    transfer is complete.  Only one transfer is allowed at any time. The
    Application can use a callback function or a polling function to check for
    completion of the transfer. If a callback is required, this should be
    registered prior to calling the SERCOM_SPI_WriteRead() function. The
    application can use the SERCOM_SPI_IsBusy() to poll for completion.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

bool SERCOM_SPI_WriteRead(sercom_registers_t* sercom, void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize)
{
    /* Verify the request */
    if (
        (txSize <= 0U) ||
        (pTransmitData == NULL) ||
        (pReceiveData == NULL) ||
        (rxSize <= 0U) ||
        (sercomSPIObj.transferIsBusy == true)
    )
        return false;

    sercomSPIObj.txBuffer = pTransmitData;
    sercomSPIObj.rxBuffer = pReceiveData;
    sercomSPIObj.rxCount = 0U;
    sercomSPIObj.txCount = 0U;
    sercomSPIObj.dummySize = 0U;

    sercomSPIObj.txSize = txSize;
    sercomSPIObj.rxSize = rxSize;

    sercomSPIObj.transferIsBusy = true;

    /* Flush out any unread data in SPI read buffer */
    while((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk)
    {
        uint32_t dummyData = sercom->SPIM.SERCOM_DATA;
        (void)dummyData;
    }

    sercom->SPIM.SERCOM_STATUS |= SERCOM_SPIM_STATUS_BUFOVF_Msk;
    sercom->SPIM.SERCOM_INTFLAG |= (uint8_t)SERCOM_SPIM_INTFLAG_ERROR_Msk;

    if(sercomSPIObj.rxSize > sercomSPIObj.txSize) {
        sercomSPIObj.dummySize = sercomSPIObj.rxSize - sercomSPIObj.txSize;
    }

    /* Start the first write here itself, rest will happen in ISR context */
    if(sercomSPIObj.txCount < sercomSPIObj.txSize) {
        sercom->SPIM.SERCOM_DATA = *((uint8_t*)sercomSPIObj.txBuffer);
        sercomSPIObj.txCount++;
    }
    else if(sercomSPIObj.dummySize > 0U) {
        sercom->SPIM.SERCOM_DATA = 0xFFU;
        sercomSPIObj.dummySize--;
    }
    else {
        /* Do nothing */
    }

    /* Enable ReceiveComplete  */
    sercom->SPIM.SERCOM_INTENSET = (uint8_t)SERCOM_SPIM_INTENSET_RXC_Msk;

    return true;
}

bool SERCOM_SPI_Write(sercom_registers_t* sercom, void* pTransmitData, size_t txSize)
{
    return SERCOM_SPI_WriteRead(sercom, pTransmitData, txSize, NULL, 0U);
}

bool SERCOM_SPI_Read(sercom_registers_t* sercom, void* pReceiveData, size_t rxSize)
{
    return SERCOM_SPI_WriteRead(sercom, NULL, 0U, pReceiveData, rxSize);
}

// *****************************************************************************
/* Function:
    void SERCOM_SPI_InterruptHandler(void);

  Summary:
    Handler that handles the SPI interrupts

  Description:
    This Function is called from the handler to handle the exchange based on the
    Interrupts.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

void SERCOM_SPI_InterruptHandler(void) {
    static bool isLastByteTransferInProgress = false;
    sercom_registers_t* sercom = SPI;

    if(sercom->SPIM.SERCOM_INTENSET != 0U) {

        if((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk) {
            uint8_t receivedData = (uint8_t)(sercom->SPIM.SERCOM_DATA);

            if(sercomSPIObj.rxCount < sercomSPIObj.rxSize) {
                ((uint8_t*)sercomSPIObj.rxBuffer)[sercomSPIObj.rxCount] = (uint8_t)receivedData;
                sercomSPIObj.rxCount++;
            }
        }

        /* If there are more words to be transmitted, then transmit them here and keep track of the count */
        if((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk) == SERCOM_SPIM_INTFLAG_DRE_Msk)
        {
            /* Disable the DRE interrupt. This will be enabled back if more than
             * one byte is pending to be transmitted */
            sercom->SPIM.SERCOM_INTENCLR = (uint8_t)SERCOM_SPIM_INTENCLR_DRE_Msk;

            if(sercomSPIObj.txCount < sercomSPIObj.txSize) {
                sercom->SPIM.SERCOM_DATA = ((uint8_t*)sercomSPIObj.txBuffer)[sercomSPIObj.txCount];
                sercomSPIObj.txCount++;
            }
            else if(sercomSPIObj.dummySize > 0U) {
                sercom->SPIM.SERCOM_DATA = 0xFFU;
                sercomSPIObj.dummySize--;
            }
            else {
                /* Do nothing */
            }

            if((sercomSPIObj.txCount == sercomSPIObj.txSize) && (sercomSPIObj.dummySize == 0U)) {
                 /* At higher baud rates, the data in the shift register can be
                 * shifted out and TXC flag can get set resulting in a
                 * callback been given to the application with the SPI interrupt
                 * pending with the application. This will then result in the
                 * interrupt handler being called again with nothing to transmit.
                 * To avoid this, a software flag is set, but
                 * the TXC interrupt is not enabled until the very end.
                 */

                isLastByteTransferInProgress = true;
            }
            else if(sercomSPIObj.rxCount == sercomSPIObj.rxSize) {
                sercom->SPIM.SERCOM_INTENSET = (uint8_t)SERCOM_SPIM_INTENSET_DRE_Msk;
                sercom->SPIM.SERCOM_INTENCLR = (uint8_t)SERCOM_SPIM_INTENCLR_RXC_Msk;
            }
            else {
                /* Do nothing */
            }
        }

        if(
                ((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == SERCOM_SPIM_INTFLAG_TXC_Msk) &&
                (isLastByteTransferInProgress == true)
        ) {
            if(sercomSPIObj.rxCount == sercomSPIObj.rxSize) {
                sercomSPIObj.transferIsBusy = false;

                /* Disable the Data Register empty and Receive Complete Interrupt flags */
                sercom->SPIM.SERCOM_INTENCLR = (uint8_t)(SERCOM_SPIM_INTENCLR_DRE_Msk | SERCOM_SPIM_INTENCLR_RXC_Msk | SERCOM_SPIM_INTENSET_TXC_Msk);

                isLastByteTransferInProgress = false;

                if(sercomSPIObj.callback != NULL) {
                    sercomSPIObj.callback(sercomSPIObj.context);
                }
            }
        }

        if(isLastByteTransferInProgress == true)
        {
            /* For the last byte transfer, the DRE interrupt is already disabled.
             * Enable TXC interrupt to ensure no data is present in the shift
             * register before application callback is called.
             */
            sercom->SPIM.SERCOM_INTENSET = (uint8_t)SERCOM_SPIM_INTENSET_TXC_Msk;
        }
    }
}
