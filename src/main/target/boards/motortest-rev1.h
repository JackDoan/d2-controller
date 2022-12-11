#ifndef D2_D2_REV2_H
#define D2_D2_REV2_H

#include "device.h"

#define DIR1 PORT_PIN_NONE
#define DIR2 PORT_PIN_NONE
#define DIR3 PORT_PIN_NONE
#define DIR4 PORT_PIN_NONE

#define CS1 PORT_PIN_NONE
#define CS2 PORT_PIN_NONE
#define CS3 PORT_PIN_NONE
#define CS4 PORT_PIN_NONE

#define EN1 PORT_PIN_PA27
#define EN2 PORT_PIN_NONE
#define EN3 PORT_PIN_NONE
#define EN4 PORT_PIN_NONE

#define VBATT_SENSE PORT_PIN_PA07
#define ISENS1 PORT_PIN_PA02

#define RX_SERCOM_RX PORT_PIN_NONE
#define RX_SERCOM_TX PORT_PIN_NONE
#define RX_SERCOM_TX_FUNC PERIPHERAL_FUNCTION_C

#define FTDI_SERCOM_RXI PORT_PIN_PA08
#define FTDI_SERCOM_TXO PORT_PIN_PA09

#define PIN_MOSI PORT_PIN_NONE
#define PIN_MISO PORT_PIN_NONE
#define PIN_SCK PORT_PIN_NONE

#define FTDI SERCOM0_REGS
#define RX SERCOM3_REGS
#define SPI SERCOM3_REGS

#define FTDI_RX_DMA_CHANNEL DMAC_CHANNEL_1
#define FPORT_DMA_CHANNEL DMAC_CHANNEL_3

#define PDEC_A PORT_PIN_PA16
#define PDEC_B PORT_PIN_PA17
#define FAULT PORT_PIN_PA18
#define M1_OUT PORT_PIN_PA24
#define M2_OUT PORT_PIN_PA25

/*
 * pwm1 TCC0/ WO[2] = CC1
 * pwm2 TCC0/ WO[3] = CC0
 * pwm3 TCC0/ WO[5] = CC2
 * pwm4 TCC0/ WO[4] = CC3
 */

#define MOTOR_1_CONFIG {\
    .enable = EN1, \
    .direction = DIR1, \
    .output = M1_OUT, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC1_REGS, \
    .pwm_channel = TCC_CHANNEL0,          \
    .pwm_channel2 = TCC_CHANNEL1,          \
    .sbus_config = &drive_sbus_params,     \
    .is_direct = true                    \
    }

#define MOTOR_2_CONFIG {\
    .enable = EN1, \
    .direction = DIR2, \
    .output = M2_OUT, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC1_REGS, \
    .pwm_channel = TCC_CHANNEL1, \
    .sbus_config = &drive_sbus_params \
    }

#define MOTOR_3_CONFIG {\
    .enable = EN3, \
    .direction = DIR3, \
    .output = PORT_PIN_NONE, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL1, \
    .sbus_config = &drive_sbus_params \
    }

#define MOTOR_4_CONFIG {\
    .enable = EN4, \
    .direction = DIR4, \
    .output = PORT_PIN_NONE, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL0, \
    .sbus_config = &drive_sbus_params \
    }

#endif //D2_D2_REV2_H
