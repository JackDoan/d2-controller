#include "interrupts.h"
#include "sercom_usart.h"

/* SERCOM USART baud value for 115200 Hz baud rate */
#define SERCOM_USART_INT_BAUD_VALUE            (63019UL)*2
//todo running the board at 24MHz right now, had to double this

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

    /* Wait for sync */
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }

    /* Enable the UART after the configurations */
    sercom->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;

    /* Wait for sync */
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }
    //todo enable PAC
}

uint32_t SERCOM_USART_FrequencyGet( void )
{
    return 48000000UL/2; //todo freq at 24MHz rn
}

bool SERCOM_USART_SerialSetup(sercom_registers_t* sercom, USART_SERIAL_SETUP * serialSetup, uint32_t clkFrequency )
{
    bool setupStatus       = false;
    uint32_t baudValue     = 0U;
    uint32_t sampleRate    = 0U;

    if((serialSetup != NULL) && (serialSetup->baudRate != 0U))
    {
        if(clkFrequency == 0U)
        {
            clkFrequency = SERCOM_USART_FrequencyGet();
        }

        if(clkFrequency >= (16U * serialSetup->baudRate))
        {
            baudValue = 65536U - (uint32_t)(((uint64_t)65536U * 16U * serialSetup->baudRate) / clkFrequency);
            sampleRate = 0U;
        }
        else if(clkFrequency >= (8U * serialSetup->baudRate))
        {
            baudValue = 65536U - (uint32_t)(((uint64_t)65536U * 8U * serialSetup->baudRate) / clkFrequency);
            sampleRate = 2U;
        }
        else if(clkFrequency >= (3U * serialSetup->baudRate))
        {
            baudValue = 65536U - (uint32_t)(((uint64_t)65536U * 3U * serialSetup->baudRate) / clkFrequency);
            sampleRate = 4U;
        }
        else
        {
            /* Do nothing */
        }

        /* Disable the USART before configurations */
        sercom->USART_INT.SERCOM_CTRLA &= ~SERCOM_USART_INT_CTRLA_ENABLE_Msk;

        /* Wait for sync */
        while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
        {
            /* Do nothing */
        }

        /* Configure Baud Rate */
        sercom->USART_INT.SERCOM_BAUD = (uint16_t)SERCOM_USART_INT_BAUD_BAUD(baudValue);

        /* Configure Parity Options */
        if(serialSetup->parity == USART_PARITY_NONE)
        {
            sercom->USART_INT.SERCOM_CTRLA =  (sercom->USART_INT.SERCOM_CTRLA & ~(SERCOM_USART_INT_CTRLA_SAMPR_Msk | SERCOM_USART_INT_CTRLA_FORM_Msk)) | SERCOM_USART_INT_CTRLA_FORM(0x0UL) | SERCOM_USART_INT_CTRLA_SAMPR((uint32_t)sampleRate); 
            sercom->USART_INT.SERCOM_CTRLB = (sercom->USART_INT.SERCOM_CTRLB & ~(SERCOM_USART_INT_CTRLB_CHSIZE_Msk | SERCOM_USART_INT_CTRLB_SBMODE_Msk)) | ((uint32_t) serialSetup->dataWidth | (uint32_t) serialSetup->stopBits);
        }
        else
        {
            sercom->USART_INT.SERCOM_CTRLA =  (sercom->USART_INT.SERCOM_CTRLA & ~(SERCOM_USART_INT_CTRLA_SAMPR_Msk | SERCOM_USART_INT_CTRLA_FORM_Msk)) | SERCOM_USART_INT_CTRLA_FORM(0x1UL) | SERCOM_USART_INT_CTRLA_SAMPR((uint32_t)sampleRate); 
            sercom->USART_INT.SERCOM_CTRLB = (sercom->USART_INT.SERCOM_CTRLB & ~(SERCOM_USART_INT_CTRLB_CHSIZE_Msk | SERCOM_USART_INT_CTRLB_SBMODE_Msk | SERCOM_USART_INT_CTRLB_PMODE_Msk)) | (uint32_t) serialSetup->dataWidth | (uint32_t) serialSetup->stopBits | (uint32_t) serialSetup->parity ;
        }

        /* Wait for sync */
        while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
        {
            /* Do nothing */
        }

        /* Enable the USART after the configurations */
        sercom->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;

        /* Wait for sync */
        while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
        {
            /* Do nothing */
        }

        setupStatus = true;
    }

    return setupStatus;
}

USART_ERROR SERCOM_USART_ErrorGet( sercom_registers_t* sercom )
{
    USART_ERROR errorStatus = (USART_ERROR) (sercom->USART_INT.SERCOM_STATUS & (uint16_t)(SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk ));

    if(errorStatus != USART_ERROR_NONE)
    {
        SERCOM_USART_ErrorClear(sercom);
    }

    return errorStatus;
}


void SERCOM_USART_TransmitterEnable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB |= SERCOM_USART_INT_CTRLB_TXEN_Msk;

    /* Wait for sync */
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }
}

void SERCOM_USART_TransmitterDisable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB &= ~SERCOM_USART_INT_CTRLB_TXEN_Msk;

    /* Wait for sync */
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U) {}
}

bool SERCOM_USART_Write(sercom_registers_t* sercom, const void *buffer, const size_t size ) {
    if(buffer == NULL)
        return false;

    /* Blocks while buffer is being transferred */
    for(size_t i = 0; i < size; i++) {
        /* Check if USART is ready for new data */
        while(!SERCOM_USART_TransmitterIsReady(sercom)) {}
        sercom->USART_INT.SERCOM_DATA = ((uint8_t*)buffer)[i];
    }

    return true;
}


bool SERCOM_USART_TransmitterIsReady( sercom_registers_t* sercom ) {
    return (sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk) == SERCOM_USART_INT_INTFLAG_DRE_Msk;
}

void SERCOM_USART_WriteByte(sercom_registers_t* sercom, int data ) {
    /* Check if USART is ready for new data */
    while(!SERCOM_USART_TransmitterIsReady(sercom)) {}
    sercom->USART_INT.SERCOM_DATA = (uint16_t)data;
}

bool SERCOM_USART_TransmitComplete( sercom_registers_t* sercom )
{
    return (sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_TXC_Msk) == SERCOM_USART_INT_INTFLAG_TXC_Msk;
}

void SERCOM_USART_ReceiverEnable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB |= SERCOM_USART_INT_CTRLB_RXEN_Msk;

    /* Wait for sync */
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }
}

void SERCOM_USART_ReceiverDisable( sercom_registers_t* sercom )
{
    sercom->USART_INT.SERCOM_CTRLB &= ~SERCOM_USART_INT_CTRLB_RXEN_Msk;

    /* Wait for sync */
    while((sercom->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }
}

bool SERCOM_USART_Read( sercom_registers_t* sercom, void *buffer, const size_t size )
{
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

bool SERCOM_USART_ReceiverIsReady( sercom_registers_t* sercom )
{
    return (sercom->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk;
}

int SERCOM_USART_ReadByte( sercom_registers_t* sercom )
{
    return (int)sercom->USART_INT.SERCOM_DATA;
}

