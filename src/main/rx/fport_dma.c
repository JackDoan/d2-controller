#include <stdio.h>
#include <string.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"
#include "plib_adc0.h"
#include "l9958.h"
#include "plib_dmac.h"
#include "helpers.h"

#include "fport_dma.h"

struct fport_dma_context {
    bool got_bytes;
    bool byte_stuffed;
    uint8_t dma_rx[30];
};

static struct fport_dma_context g_context = {0};

void fport_trigger(size_t len) {
    fport_gets(g_context.dma_rx, len);
}

void fport_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    struct fport_dma_context *context = (struct fport_dma_context*)contextHandle;
    switch(event) {
        case DMAC_TRANSFER_EVENT_COMPLETE: {
            uint8_t x = context->dma_rx[0];
            if(x == FPORT_STUFF_MARK) {
                context->byte_stuffed = true;
                fport_trigger(1);
                return;
            }
            else if(context->byte_stuffed) {
                context->dma_rx[0] ^= FPORT_XOR_VAL;
                context->byte_stuffed = false;
            }
            context->got_bytes = true;
            fport_tick();
            break;
        }
        case DMAC_TRANSFER_EVENT_NONE:
        case DMAC_TRANSFER_EVENT_ERROR:
        default:
            break;
    }
}

void fport_dma_register(void) {
    DMAC_ChannelCallbackRegister(FPORT_DMA_CHANNEL, fport_dma_callback, (uintptr_t) &g_context);
    fport_trigger(1);
}

uint8_t fport_dma_get_byte(void) {
    g_context.got_bytes = false;
    return g_context.dma_rx[0];
}