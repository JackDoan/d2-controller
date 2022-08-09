//
// Created by jack on 8/8/22.
//

#ifndef INAV_DEVICE_H
#define INAV_DEVICE_H

#include <stdbool.h>
#include "pic32cm6408mc00032.h"

#define DIR1 PORT_PIN_PA05
#define DIR2 PORT_PIN_PA00
#define DIR3 PORT_PIN_PA02
#define DIR4 PORT_PIN_PA18

#define CS1 PORT_PIN_PA04
#define CS2 PORT_PIN_PA01
#define CS3 PORT_PIN_PA27
#define CS4 PORT_PIN_PA19

#define EN1 PORT_PIN_PA06
#define EN2 PORT_PIN_PA03
#define EN3 PORT_PIN_PA28
#define EN4 PORT_PIN_PA24

#define FTDI SERCOM1_REGS
#define RX SERCOM0_REGS
#define SPI SERCOM3_REGS

void motors_set_enable(bool enabled);

#endif //INAV_DEVICE_H