#include <stdlib.h>
#include <string.h>
#include "device.h"
#include "sercom_usart.h"
#include "plib_nvmctrl.h"

static struct motor_t g_motors[] = {
        MOTOR_1_CONFIG,
        MOTOR_2_CONFIG,
        MOTOR_3_CONFIG,
        MOTOR_4_CONFIG
};

struct sign_magnitude sbus_to_duty_cycle(int sbus_val, struct motor_t *motor) {
    struct sign_magnitude out = { 0 };
    int max_scaled = motor->sbus_config.max - motor->sbus_config.min;
    uint32_t period = motor->pwm_bank->TCC_PER;
    int val = sbus_val - motor->sbus_config.min;

    if(motor->is_direct) {
        if (motor->value_disabled == sbus_val) {
            out.magnitude = 0;
            return out;
        }
        //50HZ, 29760 total counts
        uint32_t period_scaled = period/20; //total travel of 1ms, or 5%
        //need to scale the available counts between min-stick and max-stick
        out.magnitude = ((val * period_scaled) / max_scaled);
        //lowest output still needs to be 1ms, or 5%
        out.magnitude += period_scaled;
    }
    else {
        int mid_scaled = motor->sbus_config.mid - motor->sbus_config.min;
        val -= mid_scaled;
        out.sign = (val > 0) & !motor->is_direct;
        int abs_val = abs(val);
        bool outside_deadband = abs_val > motor->sbus_config.deadband;
        if(outside_deadband) {
            if(out.sign) //need to scale the available counts between middle-stick and max-stick
                out.magnitude = (abs_val * period) / (max_scaled - mid_scaled);
            else //need to scale the available counts between middle-stick and min-stick
                out.magnitude = (abs_val * period) / mid_scaled;
        }
    }

    return out;
}

static bool motors_enabled = false;
void motors_set_enable(bool enabled) {
    if(enabled && !motors_enabled) {
        serial_puts("OK");
    }
    motors_enabled = enabled;
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
        motor_set_speed(i, g_motors[i].value_disabled);
    }
}

bool failsafe_active(void) {
    return !motors_enabled;
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

struct motor_t* get_motor(enum motor_channel channel) {
    return &g_motors[channel];
}

void motor_cal(enum motor_channel channel, int sbus_val) {
    struct motor_t* motor = &g_motors[channel];
    if(motor->output == PORT_PIN_NONE)
        return;
    if (sbus_val < motor->sbus_config.min) {
        motor->sbus_config.min = sbus_val;
    }
    else if (sbus_val > motor->sbus_config.max) {
        motor->sbus_config.max = sbus_val;
    }
    else {
        motor->sbus_config.mid = (motor->sbus_config.mid+sbus_val)/2;
    }
}

void motor_set_speed(enum motor_channel channel, int sbus_val) {
    struct motor_t* motor = &g_motors[channel];
    if(motor->output == PORT_PIN_NONE)
        return;
    struct sign_magnitude out = sbus_to_duty_cycle(sbus_val, motor);
    TCC_PWM24bitDutySet(motor->pwm_bank, motor->pwm_channel, out.magnitude);
    PORT_PinWrite(motor->direction, out.sign);
}

volatile uint32_t packet_timeout_counter = 0;
void packet_timer_watchdog_feed(void) {
    packet_timeout_counter = 0;
}

void packet_timer_watchdog_tick(void) {
    static char packet_timer_watchdog_tick_buf[32] = {0};
    memset(packet_timer_watchdog_tick_buf, 0, sizeof(packet_timer_watchdog_tick_buf));
    bool do_print = false;
    if(failsafe_active()) {
        strcat(packet_timer_watchdog_tick_buf, "FAILSAFE\r\n");
        do_print = true;
    }
    if(packet_timeout_counter++ >= PACKET_TIMEOUT_MAX_COUNT) {
        failsafe_activate();
        strcat(packet_timer_watchdog_tick_buf, "Packet timeout!\r\n");
        do_print = true;
    }
    if(do_print) {
        serial_puts(packet_timer_watchdog_tick_buf);
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
    struct sbus_params motor_cal[MOTOR_COUNT] = {0};
    //read into stack first to avoid packed-struct alignment BS
    NVMCTRL_DATA_FLASH_Read((uint32_t *) &motor_cal[0], CAL_NVM_ADDR, sizeof(struct sbus_params) * MOTOR_COUNT);
    for(int i = 0; i < MOTOR_COUNT; i++) {
        PORT_PinOutputEnable(g_motors[i].enable);
        PORT_PinClear(g_motors[i].enable);
        PORT_PinOutputEnable(g_motors[i].direction);
        PORT_PinClear(g_motors[i].direction);
        PORT_PinPeripheralFunctionConfig(g_motors[i].output, g_motors[i].output_func);
        if((uint32_t)motor_cal[i].max < 4096U) {
            //only load from NVM if data is remotely sane
            memcpy(&g_motors[i].sbus_config, &motor_cal[i], sizeof(struct sbus_params));
        }
    }
}

void motor_cal_save(void) {
    uint32_t pagebuf[NVMCTRL_DATAFLASH_PAGESIZE] = {0};
    for(int i = 0; i < MOTOR_COUNT; i++) {
        memcpy(
            &pagebuf[i*(sizeof(struct sbus_params)/sizeof(uint32_t))],
            &g_motors[i].sbus_config,
            sizeof(struct sbus_params)
        );
    }
    NVMCTRL_DATA_FLASH_RowErase(CAL_NVM_ADDR);
    while (NVMCTRL_IsBusy()) {}
    NVMCTRL_DATA_FLASH_PageWrite(pagebuf, CAL_NVM_ADDR);
    while (NVMCTRL_IsBusy()) {}
}