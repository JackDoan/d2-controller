#include "interrupts.h"
#include "sercom_spi_master.h"

/* SERCOM1 SPI baud value for 1000000 Hz baud rate */
#define SERCOM_SPIM_BAUD_VALUE (23UL)

/*Global object to save SPI Exchange related data  */
static SPI_OBJECT sercomSPIObj;

inline static void spi_sync(sercom_registers_t* sercom) {
    while((sercom->SPIM.SERCOM_SYNCBUSY) != 0U) {}
}

void SERCOM_SPI_Initialize(sercom_registers_t* sercom) {
    /* Instantiate the SERCOM1 SPI object */
    sercomSPIObj.callback = NULL ;
    sercomSPIObj.transferIsBusy = false ;
    sercomSPIObj.txSize = 0U;
    sercomSPIObj.rxSize = 0U;
    sercomSPIObj.cs_line = 0;

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
            SERCOM_SPIM_CTRLA_DOPO_0x0 | /*sck on pad 1, mosi on pad0 */
            SERCOM_SPIM_CTRLA_DIPO_PAD3 | /*miso on pad 3*/
            SERCOM_SPIM_CTRLA_ENABLE_Msk;

    spi_sync(sercom);
}

void SERCOM_SPI_CallbackRegister(SERCOM_SPI_CALLBACK callBack, uintptr_t context) {
    sercomSPIObj.callback = callBack;
    sercomSPIObj.context = context;
}

bool SERCOM_SPI_IsBusy(sercom_registers_t* sercom) {
    if ((sercomSPIObj.txSize == 0U) && (sercomSPIObj.rxSize == 0U)) {
        /* This means no transfer has been requested yet; hence SPI is not busy. */
        return false;
    }
    /* if transmit is not complete or if the state flag is not set, SPI is busy */
    return SERCOM_SPI_IsTransmitterBusy(sercom) || sercomSPIObj.transferIsBusy;
}

bool SERCOM_SPI_IsTransmitterBusy(sercom_registers_t* sercom) {
    return (sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == 0U;
}

bool SERCOM_SPI_WriteRead(PORT_PIN cs_line, void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize) {
    sercom_registers_t* sercom = SPI;
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
    sercomSPIObj.cs_line = cs_line;

    sercomSPIObj.txSize = txSize;
    sercomSPIObj.rxSize = rxSize;

    sercomSPIObj.transferIsBusy = true;

    /* Flush out any unread data in SPI read buffer */
    while((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk) {
        uint32_t dummyData = sercom->SPIM.SERCOM_DATA;
        (void)dummyData;
    }

    sercom->SPIM.SERCOM_STATUS |= SERCOM_SPIM_STATUS_BUFOVF_Msk;
    sercom->SPIM.SERCOM_INTFLAG |= (uint8_t)SERCOM_SPIM_INTFLAG_ERROR_Msk;

    if(sercomSPIObj.rxSize > sercomSPIObj.txSize) {
        sercomSPIObj.dummySize = sercomSPIObj.rxSize - sercomSPIObj.txSize;
    }

    PORT_PinClear(sercomSPIObj.cs_line);

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
                PORT_PinSet(sercomSPIObj.cs_line);
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
