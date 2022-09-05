//
// Created by jack on 8/13/22.
//

#ifndef D2_L9958_H
#define D2_L9958_H

#include "device.h"
#include <stdint.h>

void L9958_Init(void);
uint16_t L9958_Diag_Read(enum motor_channel channel);
void L9958_Tick(void);
uint32_t L9958_has_problems(void);

#endif //D2_L9958_H
