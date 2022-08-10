#include <string.h>
#include "interrupts.h"
#include "sercom_usart.h"
#include "plib_dmac.h"

/* SERCOM USART baud value for 115200 Hz baud rate */
#define SERCOM_USART_INT_BAUD_VALUE            (63019UL)

static SERCOM_USART_OBJECT ftdiUSARTObj;
static SERCOM_USART_OBJECT rxUSARTObj;

static SERCOM_USART_OBJECT* get_object(sercom_registers_t* sercom) {
    switch ((uint32_t)sercom) {
        case (uint32_t)FTDI:
            return &ftdiUSARTObj;
        case (uint32_t)RX:
            return &rxUSARTObj;
        default:
            return NULL;
    }
}

static bool data_register_empty(sercom_registers_t* sercom) {
    return (sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk) == SERCOM_USART_INT_INTFLAG_DRE_Msk;
}

static void SERCOM_USART_ErrorClear( sercom_registers_t* sercom )
{
    uint8_t  u8dummyData = 0U;
    USART_ERROR errorStatus = (USART_ERROR) (sercom->USART_INT.SERCOM_STATUS & (uint16_t)(SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk ));

    if(errorStatus != USART_ERROR_NONE)
    {
        /* Clear error flag */
        sercom->USART_INT.SERCOM_INTFLAG = (uint8_t)SERCOM_USART_INT_INTFLAG_ERROR_Msk;
        /* Clear all errors */
        sercom->USART_INT.SERCOM_STATUS = (uint16_t)(SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk);

        /* Flush existing error bytes from the RX FIFO */
        while((sercom->USART_INT.SERCOM_INTFLAG & (uint8_t)SERCOM_USART_INT_INTFLAG_RXC_Msk) == (uint8_t)SERCOM_USART_INT_INTFLAG_RXC_Msk)
        {
            u8dummyData = (uint8_t)sercom->USART_INT.SERCOM_DATA;
        }
    }

    /* Ignore the warning */
    (void)u8dummyData;
}

static void SERCOM_Sync(sercom_registers_t* sercom) {
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U) { }
}

void SERCOM_USART_Initialize(sercom_registers_t* sercom)
{
    /*
     * Configures USART Clock Mode
     * Configures TXPO and RXPO
     * Configures Data Order
     * Configures Standby Mode
     * Configures Sampling rate
     * Configures IBON
     */
    sercom->USART_INT.SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_MODE_USART_INT_CLK | SERCOM_USART_INT_CTRLA_RXPO(0x1UL) | SERCOM_USART_INT_CTRLA_TXPO(0x0UL) | SERCOM_USART_INT_CTRLA_DORD_Msk | SERCOM_USART_INT_CTRLA_IBON_Msk | SERCOM_USART_INT_CTRLA_FORM(0x0UL) | SERCOM_USART_INT_CTRLA_SAMPR(0UL) ;

    /* Configure Baud Rate */
    sercom->USART_INT.SERCOM_BAUD = (uint16_t)SERCOM_USART_INT_BAUD_BAUD(SERCOM_USART_INT_BAUD_VALUE);

    /*
     * Configures RXEN
     * Configures TXEN
     * Configures CHSIZE
     * Configures Parity
     * Configures Stop bits
     */
    sercom->USART_INT.SERCOM_CTRLB = SERCOM_USART_INT_CTRLB_CHSIZE_8_BIT | SERCOM_USART_INT_CTRLB_SBMODE_1_BIT | SERCOM_USART_INT_CTRLB_RXEN_Msk | SERCOM_USART_INT_CTRLB_TXEN_Msk;
    SERCOM_Sync(sercom);

    /* Enable the UART after the configurations */
    sercom->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;
    SERCOM_Sync(sercom);

    SERCOM_USART_OBJECT* obj = get_object(sercom);
    /* Initialize instance object */
    obj->rxBuffer = NULL;
    obj->rxSize = 0;
    obj->rxProcessedSize = 0;
    obj->rxBusyStatus = false;
    obj->rxCallback = NULL;
    obj->txBuffer = NULL;
    obj->txSize = 0;
    obj->txProcessedSize = 0;
    obj->txBusyStatus = false;
    obj->txCallback = NULL;
    obj->errorStatus = USART_ERROR_NONE;

    //todo enable PAC
}

USART_ERROR SERCOM_USART_ErrorGet( sercom_registers_t* sercom )
{
    USART_ERROR errorStatus = (USART_ERROR) (sercom->USART_INT.SERCOM_STATUS & (uint16_t)(SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk ));

    if(errorStatus != USART_ERROR_NONE) {
        SERCOM_USART_ErrorClear(sercom);
    }

    return errorStatus;
}


void SERCOM_USART_TransmitterEnable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB |= SERCOM_USART_INT_CTRLB_TXEN_Msk;
    SERCOM_Sync(sercom);
}

void SERCOM_USART_TransmitterDisable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB &= ~SERCOM_USART_INT_CTRLB_TXEN_Msk;
    SERCOM_Sync(sercom);
}

bool SERCOM_USART_Write(sercom_registers_t* sercom, const void *buffer, const size_t size ) {
    if(buffer == NULL)
        return false;
    //todo use SERCOM_USART_OBJECT* obj = get_object(sercom);
    /* Blocks while buffer is being transferred */
    for(size_t i = 0; i < size; i++) {
        /* Check if USART is ready for new data */
        while(!data_register_empty(sercom)) {}
        sercom->USART_INT.SERCOM_DATA = ((uint8_t*)buffer)[i];
    }

    return true;
}

void SERCOM_USART_WriteByte(sercom_registers_t* sercom, int data ) {
    /* Check if USART is ready for new data */
    while(!data_register_empty(sercom)) {}
    sercom->USART_INT.SERCOM_DATA = (uint16_t)data;
}

bool SERCOM_USART_TransmitComplete( sercom_registers_t* sercom ) {
    return (sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_TXC_Msk) == SERCOM_USART_INT_INTFLAG_TXC_Msk;
}

void SERCOM_USART_ReceiverEnable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB |= SERCOM_USART_INT_CTRLB_RXEN_Msk;
    SERCOM_Sync(sercom);
}

void SERCOM_USART_ReceiverDisable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB &= ~SERCOM_USART_INT_CTRLB_RXEN_Msk;
    SERCOM_Sync(sercom);
}

bool SERCOM_USART_Read(sercom_registers_t* sercom, void *buffer, const size_t size) {
    uint8_t* pu8Data        = (uint8_t*)buffer;
    USART_ERROR errorStatus = USART_ERROR_NONE;

    if(buffer == NULL)
        return false;

    /* Clear error flags and flush out error data that may have been received when no active request was pending */
    SERCOM_USART_ErrorClear(sercom);

    for(size_t i = 0; i < size; i++) {
        /* Check if USART has new data */
        while((sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == 0U) {}

        errorStatus = (USART_ERROR) (sercom->USART_INT.SERCOM_STATUS & (uint16_t)(SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk));

        if(errorStatus != USART_ERROR_NONE)
            return false;

        pu8Data[i] = (uint8_t)sercom->USART_INT.SERCOM_DATA;
    }

    return true;
}

bool SERCOM_USART_Read_new(sercom_registers_t* sercom, void *buffer, const size_t size) {
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    if(buffer == NULL)
        return false;
    if (obj->rxBusyStatus == true)
        return false;
    /* Clear error flags and flush out error data that may have been received when no active request was pending */
    SERCOM_USART_ErrorClear(sercom);

    obj->rxBuffer = buffer;
    obj->rxSize = size;
    obj->rxProcessedSize = 0U;
    obj->rxBusyStatus = true;
    obj->errorStatus = USART_ERROR_NONE;

    /* Enable receive and error interrupt */
    SERCOM0_REGS->USART_INT.SERCOM_INTENSET = (uint8_t) (SERCOM_USART_INT_INTENSET_ERROR_Msk |
                                                         SERCOM_USART_INT_INTENSET_RXC_Msk);

    return true;
}

bool SERCOM_USART_ReadIsBusy(sercom_registers_t* sercom)
{
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    return obj->rxBusyStatus;
}

size_t SERCOM_USART_ReadCountGet(sercom_registers_t* sercom)
{
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    return obj->rxProcessedSize;
}

bool SERCOM_USART_ReadAbort(sercom_registers_t* sercom)
{
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    if (obj->rxBusyStatus == true)
    {
        /* Disable receive and error interrupt */
        SERCOM0_REGS->USART_INT.SERCOM_INTENCLR = (uint8_t)(SERCOM_USART_INT_INTENCLR_ERROR_Msk | SERCOM_USART_INT_INTENCLR_RXC_Msk);

        obj->rxBusyStatus = false;

        /* If required application should read the num bytes processed prior to calling the read abort API */
        obj->rxSize = 0U;
        obj->rxProcessedSize = 0U;
    }

    return true;
}

bool SERCOM_USART_ReceiverIsReady( sercom_registers_t* sercom )
{
    return (sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk;
}

int SERCOM_USART_ReadByte( sercom_registers_t* sercom )
{
    return (int)sercom->USART_INT.SERCOM_DATA;
}


static void SERCOM0_USART_ISR_ERR_Handler(sercom_registers_t* sercom) {
    USART_ERROR errorStatus = USART_ERROR_NONE;
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    errorStatus = (USART_ERROR) (sercom->USART_INT.SERCOM_STATUS & (uint16_t)(SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk));

    if(errorStatus != USART_ERROR_NONE) {
        /* Save the error to be reported later */
        obj->errorStatus = errorStatus;

        /* Clear the error flags and flush out the error bytes */
        SERCOM_USART_ErrorClear(sercom);

        /* Disable error and receive interrupt to abort on-going transfer */
        sercom->USART_INT.SERCOM_INTENCLR = (uint8_t)(SERCOM_USART_INT_INTENCLR_ERROR_Msk | SERCOM_USART_INT_INTENCLR_RXC_Msk);

        /* Clear the RX busy flag */
        obj->rxBusyStatus = false;

        if(obj->rxCallback != NULL) {
            obj->rxCallback(obj->rxContext);
        }
    }
}

static void SERCOM0_USART_ISR_RX_Handler(sercom_registers_t* sercom){
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    if(obj->rxBusyStatus != true)
        return;
    if(obj->rxProcessedSize < obj->rxSize) {
        ((uint8_t*)obj->rxBuffer)[obj->rxProcessedSize] = (uint8_t)sercom->USART_INT.SERCOM_DATA;
        obj->rxProcessedSize++;

        if(obj->rxProcessedSize == obj->rxSize) {
            obj->rxBusyStatus = false;
            obj->rxSize = 0U;
            sercom->USART_INT.SERCOM_INTENCLR = (uint8_t)(SERCOM_USART_INT_INTENCLR_RXC_Msk | SERCOM_USART_INT_INTENCLR_ERROR_Msk);

            if(obj->rxCallback != NULL) {
                obj->rxCallback(obj->rxContext);
            }
        }
    }

}

static void usart_tx_work(sercom_registers_t* sercom, SERCOM_USART_OBJECT* obj) {
    while (data_register_empty(sercom) && (obj->txProcessedSize < obj->txSize)) {
        sercom->USART_INT.SERCOM_DATA = ((uint8_t*)obj->txBuffer)[obj->txProcessedSize];
        obj->txProcessedSize++;
    }
}

bool SERCOM_USART_Write_Nonblock(sercom_registers_t* sercom, void *buffer, const size_t size) {
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    if((buffer == NULL) || (obj->txBusyStatus == true))
        return false;
    obj->txBuffer = buffer;
    obj->txSize = size;
    obj->txBusyStatus = true;
    obj->txProcessedSize = 0;
    /* Initiate the transfer by sending first byte */
    usart_tx_work(sercom, obj);
    sercom->USART_INT.SERCOM_INTENSET = (uint8_t) SERCOM_USART_INT_INTFLAG_DRE_Msk;
    return true;
}

void serial_puts(void *buffer) {
    //SERCOM_USART_Write_Nonblock(FTDI, buffer, strlen(buffer));
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, buffer, (const void *)&FTDI->USART_INT.SERCOM_DATA, strlen(buffer));
}

void serial_gets(void *buffer, size_t len) {
    DMAC_ChannelTransfer(DMAC_CHANNEL_1, (const void *)&FTDI->USART_INT.SERCOM_DATA, buffer, len);
}

void SERCOM_USART_TX_Wait(sercom_registers_t* sercom) {
    SERCOM_USART_OBJECT *obj = get_object(sercom);
    while(obj->txBusyStatus == true) {}
}

void SERCOM_USART_RX_Wait(sercom_registers_t* sercom) {
    while(SERCOM_USART_ReadIsBusy(sercom)) {}
}

static void SERCOM0_USART_ISR_TX_Handler( sercom_registers_t* sercom ) {
    SERCOM_USART_OBJECT* obj = get_object(sercom);
    if(obj->txBusyStatus != true)
        return;

    usart_tx_work(sercom, obj);

    if(obj->txProcessedSize >= obj->txSize) {
        obj->txBusyStatus = false;
        obj->txSize = 0U;
        sercom->USART_INT.SERCOM_INTENCLR = (uint8_t)SERCOM_USART_INT_INTENCLR_DRE_Msk;

        if(obj->txCallback != NULL) {
            obj->txCallback(obj->txContext);
        }
    }

}

static void SERCOM_USART_InterruptHandler( sercom_registers_t* sercom ) {
    bool testCondition = false;
    if(sercom->USART_INT.SERCOM_INTENSET != 0U)
    {
        /* Checks for error flag */
        testCondition = ((sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_ERROR_Msk) == SERCOM_USART_INT_INTFLAG_ERROR_Msk);
        testCondition = ((sercom->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_ERROR_Msk) == SERCOM_USART_INT_INTENSET_ERROR_Msk) && testCondition;
        if(testCondition) {
            SERCOM0_USART_ISR_ERR_Handler(sercom);
        }

        //todo see if RX actually needs to be handled first?
        testCondition = data_register_empty(sercom);
        testCondition = ((sercom->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_DRE_Msk) == SERCOM_USART_INT_INTENSET_DRE_Msk) && testCondition;
        /* Checks for data register empty flag */
        if(testCondition) {
            SERCOM0_USART_ISR_TX_Handler(sercom);
        }

        testCondition = ((sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk);
        testCondition = ((sercom->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_RXC_Msk) == SERCOM_USART_INT_INTENSET_RXC_Msk) && testCondition;
        /* Checks for receive complete empty flag */
        if(testCondition) {
            SERCOM0_USART_ISR_RX_Handler(sercom);
        }
        

    }
}

void SERCOM1_USART_InterruptHandler( void ) {
    SERCOM_USART_InterruptHandler(FTDI);
}

void SERCOM0_USART_InterruptHandler( void ) {
    SERCOM_USART_InterruptHandler(RX);
}