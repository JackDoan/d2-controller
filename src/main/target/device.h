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

#define VBATT_SENSE PORT_PIN_PA07

#define RX_SERCOM_RX PORT_PIN_PA09
#define RX_SERCOM_TX PORT_PIN_PA08
#define RX_SERCOM_TX_FUNC PERIPHERAL_FUNCTION_C

#define FTDI_SERCOM_RXI PORT_PIN_PA16
#define FTDI_SERCOM_TXO PORT_PIN_PA17

#define PIN_PWM1 PORT_PIN_PA10
#define PIN_PWM2 PORT_PIN_PA11
#define PIN_PWM3 PORT_PIN_PA15
#define PIN_PWM4 PORT_PIN_PA14

#define PIN_MOSI PORT_PIN_PA22
#define PIN_MISO PORT_PIN_PA25
#define PIN_SCK PORT_PIN_PA23

#define FTDI SERCOM1_REGS
#define RX SERCOM0_REGS
#define SPI SERCOM3_REGS

#define FTDI_DMA_CHANNEL DMAC_CHANNEL_1
#define FPORT_DMA_CHANNEL DMAC_CHANNEL_3

void motors_set_enable(bool enabled);

enum motor_channel {
    MOTOR1 = 0,
    MOTOR2,
    MOTOR3,
    MOTOR4,
    MOTOR_COUNT
};

struct sbus_params {
    int max;
    int mid;
    int min;
    int deadband;
    int value_disabled;
};

struct sign_magnitude {
    bool sign;
    uint32_t magnitude;
};

extern struct sbus_params channel_defaults;

void motor_enable(enum motor_channel channel, bool enable);
void motor_set_speed(enum motor_channel channel, int sbus_val);
void do_brakes(int sbus_val);
void failsafe_activate(void);
void packet_timer_watchdog_tick(void);
void packet_timer_watchdog_feed(void);
void fport_enable_tx(bool enable);
struct sign_magnitude sbus_to_duty_cycle(int sbus_val, uint32_t period, struct sbus_params* params);

#endif //INAV_DEVICE_H
