#include "interrupts.h"
#include "sercom_spi_master.h"

static SPI_OBJECT contexts[2] = {
        {
            .callback = NULL,
            .baud = (23UL), /* SERCOM1 SPI baud value for 1MHz baud rate */
        },
        {
            .callback = NULL,
            .baud = (79UL), /* SERCOM1 SPI baud value for 300kHz baud rate */
        },
};

static SPI_OBJECT* getContext(sercom_registers_t* sercom) {
    if(sercom == SPI)
        return &contexts[0];

    //DSHOT, and default I guess
    return &contexts[1];
}

inline static void spi_sync(sercom_registers_t* sercom) {
    while((sercom->SPIM.SERCOM_SYNCBUSY) != 0U) {}
}

void SERCOM_SPI_Initialize(sercom_registers_t* sercom) {
    SPI_OBJECT* ctx = getContext(sercom);
    ctx->callback = NULL;
    ctx->transferIsBusy = false;
    ctx->txSize = 0U;
    ctx->rxSize = 0U;
    ctx->cs_line = 0;

    /* Selection of the Character Size and Receiver Enable */
    sercom->SPIM.SERCOM_CTRLB =
            SERCOM_SPIM_CTRLB_CHSIZE_8_BIT |
            SERCOM_SPIM_CTRLB_RXEN_Msk;
    spi_sync(sercom);

    /* Selection of the Baud Value */
    sercom->SPIM.SERCOM_BAUD = (uint8_t)SERCOM_SPIM_BAUD_BAUD(ctx->baud);

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

void SERCOM_SPI_CallbackRegister(sercom_registers_t* sercom, SERCOM_SPI_CALLBACK callBack) {
    SPI_OBJECT *ctx = getContext(sercom);
    ctx->callback = callBack;
}

bool SERCOM_SPI_IsBusy(sercom_registers_t* sercom) {
    SPI_OBJECT *ctx = getContext(sercom);
    if ((ctx->txSize == 0U) && (ctx->rxSize == 0U)) {
        /* This means no transfer has been requested yet; hence SPI is not busy. */
        return false;
    }
    /* if transmit is not complete or if the state flag is not set, SPI is busy */
    return SERCOM_SPI_IsTransmitterBusy(sercom) || ctx->transferIsBusy;
}

bool SERCOM_SPI_IsTransmitterBusy(sercom_registers_t* sercom) {
    return (sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == 0U;
}

bool SERCOM_SPI_WriteRead(sercom_registers_t* sercom, PORT_PIN cs_line, void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize) {
    SPI_OBJECT *ctx = getContext(sercom);
    /* Verify the request */
    if (
        (txSize <= 0U) ||
        (pTransmitData == NULL) ||
        (pReceiveData == NULL) ||
        (rxSize <= 0U) ||
        (ctx->transferIsBusy == true)
    )
        return false;

    ctx->txBuffer = pTransmitData;
    ctx->rxBuffer = pReceiveData;
    ctx->rxCount = 0U;
    ctx->txCount = 0U;
    ctx->dummySize = 0U;
    ctx->cs_line = cs_line;

    ctx->txSize = txSize;
    ctx->rxSize = rxSize;

    ctx->transferIsBusy = true;

    /* Flush out any unread data in SPI read buffer */
    while((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk) {
        uint32_t dummyData = sercom->SPIM.SERCOM_DATA;
        (void)dummyData;
    }

    sercom->SPIM.SERCOM_STATUS |= SERCOM_SPIM_STATUS_BUFOVF_Msk;
    sercom->SPIM.SERCOM_INTFLAG |= (uint8_t)SERCOM_SPIM_INTFLAG_ERROR_Msk;

    if(ctx->rxSize > ctx->txSize) {
        ctx->dummySize = ctx->rxSize - ctx->txSize;
    }

    PORT_PinClear(ctx->cs_line);

    /* Start the first write here itself, rest will happen in ISR context */
    if(ctx->txCount < ctx->txSize) {
        sercom->SPIM.SERCOM_DATA = *((uint8_t*)ctx->txBuffer);
        ctx->txCount++;
    }
    else if(ctx->dummySize > 0U) {
        sercom->SPIM.SERCOM_DATA = 0xFFU;
        ctx->dummySize--;
    }
    else {
        /* Do nothing */
    }

    /* Enable ReceiveComplete  */
    sercom->SPIM.SERCOM_INTENSET = (uint8_t)SERCOM_SPIM_INTENSET_RXC_Msk;

    return true;
}

//todo handle something other
static void SERCOM_SPI_InterruptHandler(sercom_registers_t* sercom) {
    static bool isLastByteTransferInProgress = false;
    SPI_OBJECT *ctx = getContext(sercom);

    if(sercom->SPIM.SERCOM_INTENSET != 0U) {

        if((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk) {
            uint8_t receivedData = (uint8_t)(sercom->SPIM.SERCOM_DATA);

            if(ctx->rxCount < ctx->rxSize) {
                ((uint8_t*)ctx->rxBuffer)[ctx->rxCount] = (uint8_t)receivedData;
                ctx->rxCount++;
            }
        }

        /* If there are more words to be transmitted, then transmit them here and keep track of the count */
        if((sercom->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk) == SERCOM_SPIM_INTFLAG_DRE_Msk)
        {
            /* Disable the DRE interrupt. This will be enabled back if more than
             * one byte is pending to be transmitted */
            sercom->SPIM.SERCOM_INTENCLR = (uint8_t)SERCOM_SPIM_INTENCLR_DRE_Msk;

            if(ctx->txCount < ctx->txSize) {
                sercom->SPIM.SERCOM_DATA = ((uint8_t*)ctx->txBuffer)[ctx->txCount];
                ctx->txCount++;
            }
            else if(ctx->dummySize > 0U) {
                sercom->SPIM.SERCOM_DATA = 0xFFU;
                ctx->dummySize--;
            }
            else {
                /* Do nothing */
            }

            if((ctx->txCount == ctx->txSize) && (ctx->dummySize == 0U)) {
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
            else if(ctx->rxCount == ctx->rxSize) {
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
            if(ctx->rxCount == ctx->rxSize) {
                ctx->transferIsBusy = false;
                PORT_PinSet(ctx->cs_line);
                /* Disable the Data Register empty and Receive Complete Interrupt flags */
                sercom->SPIM.SERCOM_INTENCLR = (uint8_t)(SERCOM_SPIM_INTENCLR_DRE_Msk | SERCOM_SPIM_INTENCLR_RXC_Msk | SERCOM_SPIM_INTENSET_TXC_Msk);

                isLastByteTransferInProgress = false;

                if(ctx->callback != NULL) {
                    ctx->callback();
                }
            }
        }

        if(isLastByteTransferInProgress == true) {
            /* For the last byte transfer, the DRE interrupt is already disabled.
             * Enable TXC interrupt to ensure no data is present in the shift
             * register before application callback is called.
             */
            sercom->SPIM.SERCOM_INTENSET = (uint8_t)SERCOM_SPIM_INTENSET_TXC_Msk;
        }
    }
}

void SERCOM3_SPI_InterruptHandler(void) {
    SERCOM_SPI_InterruptHandler(SPI);
}

void SERCOM2_DSHOT_InterruptHandler(void) {
    SERCOM_SPI_InterruptHandler(DSHOT);
}