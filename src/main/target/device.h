#ifndef DEVICE_H
#define DEVICE_H

#include <stdbool.h>
#include "pic32cm6408mc00032.h"

#ifdef BOARD_NAME_D2_rev2
#include "boards/d2-rev2.h"
#endif
#ifdef BOARD_NAME_D2_rev1
#include "boards/d2-rev1.h"
#endif
#ifdef BOARD_NAME_MEANBEAN_rev1
#include "boards/meanbean-rev1.h"
#endif

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

#endif //DEVICE_H
