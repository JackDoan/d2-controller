#include <stdint.h>

#ifndef D2_DSHOT_H
#define D2_DSHOT_H


#define DSHOT_MAX 2000
#define DSHOT_DMA_CHANNEL DMAC_CHANNEL_2

void dshot_set_speed(uint16_t speed);
void dshot_init(void);
void dshot_tick(void);
#endif //D2_DSHOT_H
