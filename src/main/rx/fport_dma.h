//
// Created by jack on 3/25/23.
//

#ifndef D2_FPORT_DMA_H
#define D2_FPORT_DMA_H

void fport_dma_register(void);
uint8_t fport_dma_get_byte(void);
void fport_trigger(size_t len);
void fport_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle);

#endif //D2_FPORT_DMA_H
