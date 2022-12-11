#include <stdlib.h>
#include "device.h"
#include "sercom_usart.h"

struct sbus_params drive_sbus_params = {
        .max = 1800,
        .mid = 992,
        .min = 173,
        .deadband = 3,
        .value_disabled = 8,
};

struct sbus_params weapon_sbus_params = {
        .max = 1800,
        .mid = 992,
        .min = 173,
        .deadband = 3,
        .value_disabled = 8,
};

static struct motor_t g_motors[] = {
        MOTOR_1_CONFIG,
        MOTOR_2_CONFIG,
        MOTOR_3_CONFIG,
        MOTOR_4_CONFIG
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
    for(int i = 0; i < MOTOR_COUNT; i++) {
        motor_set_speed(i, g_motors[i].sbus_config->value_disabled);
    }
}

void motor_enable(enum motor_channel channel, bool enable) {
    PORT_PinWrite(g_motors[channel].enable, enable);
}

void do_brakes(int sbus_val) {
    //if pressed -> ~1800
    //unpressed -> 992
    bool button_not_pressed = sbus_val < 1100;
    motors_set_enable(button_not_pressed);
}

void motor_set_speed(enum motor_channel channel, int sbus_val) {
    struct motor_t* motor = &g_motors[channel];
    if(motor->output == PORT_PIN_NONE)
        return;
    if(motor->is_direct) {
        TCC_PWM24bitDutySet(motor->pwm_bank, motor->pwm_channel, (sbus_val));
        TCC_PWM24bitDutySet(motor->pwm_bank, motor->pwm_channel2, (sbus_val));
    } else {
        struct sign_magnitude out = sbus_to_duty_cycle(sbus_val, motor->pwm_bank->TCC_PER, motor->sbus_config);
        TCC_PWM24bitDutySet(motor->pwm_bank, motor->pwm_channel, out.magnitude);
        PORT_PinWrite(motor->direction, out.sign);
    }
}

#define PACKET_TIMEOUT_MAX_COUNT 500
volatile uint32_t packet_timeout_counter = 0;
void packet_timer_watchdog_feed(void) {
    packet_timeout_counter = 0;
}

void packet_timer_watchdog_tick(void) {
    if(packet_timeout_counter++ >= PACKET_TIMEOUT_MAX_COUNT) {
        //TODO remove failsafe_activate();
        //serial_puts("Packet timeout!\r\n");
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

void motors_init(void) {
    for(int i = 0; i < MOTOR_COUNT; i++) {
        PORT_PinOutputEnable(g_motors[i].enable);
        PORT_PinClear(g_motors[i].enable);
        PORT_PinOutputEnable(g_motors[i].direction);
        PORT_PinClear(g_motors[i].direction);
        PORT_PinPeripheralFunctionConfig(g_motors[i].output, g_motors[i].output_func);
    }
}
