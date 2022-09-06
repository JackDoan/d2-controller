#include <stdlib.h>
#include "device.h"
#include "plib_port.h"
#include "sercom_usart.h"

struct sbus_params channel_defaults = {
        .max = 1800,
        .mid = 992,
        .min = 173,
        .deadband = 3,
        .value_disabled = 8
};

struct sign_magnitude sbus_to_duty_cycle(int sbus_val, uint32_t period, struct sbus_params* params) {
    struct sign_magnitude out = {
        .sign = false,
        .magnitude = 0
    };

    if(sbus_val == params->value_disabled) {
        return out;
    }

    struct sbus_params params_scaled = {
            .max = params->max - params->min,
            .mid = params->mid - params->min,
            .min = 0,
            .deadband = params->deadband
    };

    int val = (sbus_val - params->min) - params_scaled.mid;
    out.sign = val > 0;
    int abs_val = abs(val);
    bool outside_deadband = abs_val > params_scaled.deadband;
    if(outside_deadband) {
        if(out.sign)
            out.magnitude = (abs_val * period) / (params_scaled.max - params_scaled.mid);
        else
            out.magnitude = (abs_val * period) / (params_scaled.mid);
    }
    return out;
}

void motors_set_enable(bool enabled) {
    //const uint32_t en_mask = GET_PIN_MASK(EN1) | GET_PIN_MASK(EN2) | GET_PIN_MASK(EN3) | GET_PIN_MASK(EN4);
    //PORT_GroupWrite(PORT_GROUP_0,en_mask,enabled & en_mask);
    motor_enable(MOTOR1, enabled);
    motor_enable(MOTOR2, enabled);
    motor_enable(MOTOR3, enabled);
    motor_enable(MOTOR4, enabled);
}

void failsafe_activate(void) {
    motors_set_enable(false);
    //set PWM to midpoint (off)
    motor_set_speed(MOTOR1, channel_defaults.value_disabled);
    motor_set_speed(MOTOR2, channel_defaults.value_disabled);
    motor_set_speed(MOTOR3, channel_defaults.value_disabled);
    motor_set_speed(MOTOR4, channel_defaults.value_disabled);
}

static PORT_PIN enable_pins[] = {EN1, EN2, EN3, EN4};
static PORT_PIN dir_pins[] = {DIR1, DIR2, DIR3, DIR4};
static volatile uint32_t * duty_cycles[] = {
        &TCC0_REGS->TCC_CC[2],
        &TCC0_REGS->TCC_CC[3],
        &TCC0_REGS->TCC_CC[1],
        &TCC0_REGS->TCC_CC[0]
};

void motor_enable(enum motor_channel channel, bool enable) {
    PORT_PinWrite(enable_pins[channel], enable);
}

void do_brakes(int sbus_val) {
    //if pressed -> ~1800
    //unpressed -> 992
    bool button_not_pressed = sbus_val < 1100;
    motors_set_enable(button_not_pressed);
}

void motor_set_speed(enum motor_channel channel, int sbus_val) {
    struct sign_magnitude out = sbus_to_duty_cycle(sbus_val, TCC0_REGS->TCC_PER, &channel_defaults);
    *duty_cycles[channel] = out.magnitude;
    PORT_PinWrite(dir_pins[channel], out.sign);
}

#define PACKET_TIMEOUT_MAX_COUNT 500
volatile uint32_t packet_timeout_counter = 0;
void packet_timer_watchdog_feed(void) {
    packet_timeout_counter = 0;
}

void packet_timer_watchdog_tick(void) {
    if(packet_timeout_counter++ >= PACKET_TIMEOUT_MAX_COUNT) {
        failsafe_activate();
        serial_puts("Packet timeout!\r\n");
    }
}

void fport_enable_tx(bool enable) {
    if(enable) {
        SERCOM_USART_ReceiverDisable(RX);
        PORT_PinPeripheralFunctionConfig(RX_SERCOM_TX, RX_SERCOM_TX_FUNC);
        SERCOM_USART_TransmitterEnable(RX);
    }
    else {
        SERCOM_USART_TransmitterDisable(RX);
        PORT_PinGPIOConfig(RX_SERCOM_TX);
        SERCOM_USART_ReceiverEnable(RX);
    }
}

void end(void) {
}