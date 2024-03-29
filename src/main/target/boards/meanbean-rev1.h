#ifndef D2_MEANBEAN_REV1_H
#define D2_MEANBEAN_REV1_H

#include "device.h"

#define DIR1 PORT_PIN_PA05
#define DIR2 PORT_PIN_PA00
#define DIR3 PORT_PIN_PA27
#define DIR4 PORT_PIN_PA18

#define CS1 PORT_PIN_NONE
#define CS2 PORT_PIN_NONE
#define CS3 PORT_PIN_PA24
#define CS4 PORT_PIN_PA19

#define EN1 PORT_PIN_PA01
#define EN2 PORT_PIN_PA01
#define EN3 PORT_PIN_PA28
#define EN4 PORT_PIN_PA11

#define VBATT_SENSE PORT_PIN_PA07

#define RX_SERCOM_RX PORT_PIN_PA09
#define RX_SERCOM_TX PORT_PIN_PA04
#define RX_SERCOM_TX_FUNC PERIPHERAL_FUNCTION_D

#define FTDI_SERCOM_RXI PORT_PIN_PA16
#define FTDI_SERCOM_TXO PORT_PIN_PA17

#define PIN_MOSI PORT_PIN_PA22
#define PIN_MISO PORT_PIN_PA25
#define PIN_SCK PORT_PIN_PA23

#define FTDI SERCOM1_REGS
#define RX SERCOM0_REGS
#define SPI SERCOM3_REGS

#define FTDI_DMA_CHANNEL DMAC_CHANNEL_1
#define FPORT_DMA_CHANNEL DMAC_CHANNEL_3
#define FPORT_TX_DMA_CHANNEL DMAC_CHANNEL_4

#define MOTOR_1_CONFIG {\
    .enable = PORT_PIN_NONE, \
    .direction = PORT_PIN_NONE, \
    .output = PORT_PIN_NONE, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL2, \
    .sbus_config = DRIVE_PARAMS_DEFAULT, \
    .value_disabled = 16}

//weapon on physical pin 11, TCC[1] WO[2]
#define MOTOR_2_CONFIG {\
    .enable = PORT_PIN_NONE, \
    .direction = PORT_PIN_NONE, \
    .output = PORT_PIN_PA08, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC1_REGS, \
    .pwm_channel = TCC_CHANNEL0, \
    .sbus_config = WEAPON_PARAMS_DEFAULT,    \
    .is_direct = true,                    \
    .value_disabled = 16}

//left
#define MOTOR_3_CONFIG {\
    .enable = EN3, \
    .direction = DIR3, \
    .output = PORT_PIN_PA15, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL1, \
    .sbus_config = DRIVE_PARAMS_DEFAULT, \
    .value_disabled = 16}

//right
#define MOTOR_4_CONFIG {\
    .enable = EN4, \
    .direction = DIR4, \
    .output = PORT_PIN_PA14, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL0, \
    .sbus_config = DRIVE_PARAMS_DEFAULT, \
    .value_disabled = 16}

#endif //D2_MEANBEAN_REV1_H
