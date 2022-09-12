#ifndef D2_D2_REV1_H
#define D2_D2_REV1_H

#include "plib_port.h"
#include "plib_tcc0.h"

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

#define VBATT_SENSE PORT_PIN_PA07

#define RX_SERCOM_RX PORT_PIN_PA09
#define RX_SERCOM_TX PORT_PIN_PA09
#define RX_SERCOM_TX_FUNC PERIPHERAL_FUNCTION_C

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

#define MOTOR_1_CONFIG {\
    .enable = EN1, \
    .direction = DIR1, \
    .output = PORT_PIN_PA10, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL2 \
    }

#define MOTOR_2_CONFIG {\
    .enable = EN2, \
    .direction = DIR2, \
    .output = PORT_PIN_PA11, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL3 \
    }

#define MOTOR_3_CONFIG {\
    .enable = EN3, \
    .direction = DIR3, \
    .output = PORT_PIN_PA15, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL1 \
    }

#define MOTOR_4_CONFIG {\
    .enable = EN4, \
    .direction = DIR4, \
    .output = PORT_PIN_PA14, \
    .output_func = PERIPHERAL_FUNCTION_F, \
    .pwm_bank = TCC0_REGS, \
    .pwm_channel = TCC_CHANNEL0 \
    }

#endif //D2_D2_REV1_H
