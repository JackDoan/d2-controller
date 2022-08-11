#include <stdlib.h>
#include "device.h"
#include "plib_port.h"

inline void motors_set_enable(bool enabled) {
    const uint32_t en_mask = GET_PIN_MASK(EN1) | GET_PIN_MASK(EN2) | GET_PIN_MASK(EN3) | GET_PIN_MASK(EN4);
    PORT_GroupWrite(PORT_GROUP_0,en_mask,enabled & en_mask);
}

const int sbus_max = 2047;
const int sbus_mid = 992;
const int sbus_min = 0;
const int sbus_center_deadband = 5;

static PORT_PIN enable_pins[] = {EN1, EN2, EN3, EN4};
static PORT_PIN dir_pins[] = {DIR1, DIR2, DIR3, DIR4};
static volatile uint32_t * duty_cycles[] = {
        &TCC0_REGS->TCC_CC[2],
        &TCC0_REGS->TCC_CC[3],
        &TCC0_REGS->TCC_CC[1],
        &TCC0_REGS->TCC_CC[0]
};
uint32_t pwm_dc_from_perthou(int sbus_val) {
    return sbus_val;
}

void motor_enable(enum motor_channel channel, bool enable) {
    PORT_PinWrite(enable_pins[channel], enable);
}

void motor_set_speed(enum motor_channel channel, int sbus_val) {
    //todo deadbanding
    int val = sbus_val-sbus_mid;
    bool dir = val > 0;
    uint32_t abs_val = abs(val);
    motor_enable(channel, abs_val > sbus_center_deadband);
    PORT_PinWrite(dir_pins[channel], dir);
    *duty_cycles[channel] = pwm_dc_from_perthou(abs_val);
}



void end(void) {
}