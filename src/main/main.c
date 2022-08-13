#include <stdio.h>
#include "plib_port.h"
#include "plib_clock.h"
#include "sercom_usart.h"
#include "plib_tcc0.h"
#include "sercom_spi_master.h"
#include "plib_nvic.h"
#include "plib_dmac.h"
#include "common/fport.h"
#include "timer.h"
#include "plib_adc0.h"
#include "plib_systick.h"
#include "l9958.h"


char cmd_resp_buf[64] = {0};

void cmd_prompt(char cmd) {
    switch(cmd) {
        case '1':
            motor_enable(MOTOR1, true);
            motor_set_speed(MOTOR1, 1200);
            break;
        case '!':
            motor_enable(MOTOR1, false);
            motor_set_speed(MOTOR1, 0);
            break;
        case '2':
            motor_enable(MOTOR2, true);
            motor_set_speed(MOTOR2, 1200);
            break;
        case '@':
            motor_enable(MOTOR2, false);
            motor_set_speed(MOTOR2, 0);
            break;
        case '3':
            motor_enable(MOTOR3, true);
            motor_set_speed(MOTOR3, 1200);
            break;
        case '#':
            motor_enable(MOTOR3, false);
            motor_set_speed(MOTOR3, 0);
            break;
        case '4':
            motor_enable(MOTOR4, true);
            motor_set_speed(MOTOR4, 1200);
            break;
        case '$':
            motor_enable(MOTOR4, false);
            motor_set_speed(MOTOR4, 0);
            break;
        case 'd':
            PORT_PinToggle(DIR3);
            break;
        case 's':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "1: %04x\r\n2: %04x\r\n3: %04x\r\n4: %04x\r\n",
                     L9958_Diag_Read(MOTOR1), L9958_Diag_Read(MOTOR2),
                     L9958_Diag_Read(MOTOR3), L9958_Diag_Read(MOTOR4));
            serial_puts(cmd_resp_buf);
            break;
        case 'p':
            fport_enable_printing(true);
            break;
        case 'P':
            fport_enable_printing(false);
            break;
        case 'a':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "VBatt: %d\r\n", (int) ADC0_Convert_mV());
            serial_puts(cmd_resp_buf);
            break;
        case 't':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "SysTick: %lu\r\n", SYSTICK_GetTickCounter());
            serial_puts(cmd_resp_buf);
            break;
        default:
            break;
    }
}

static bool rxRead = false;
static bool ftdiRead = false;
static void uartRxCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    switch(event) {
        case DMAC_TRANSFER_EVENT_COMPLETE:
            *(bool*)contextHandle = true;
            break;
        case DMAC_TRANSFER_EVENT_NONE:
        case DMAC_TRANSFER_EVENT_ERROR:
        default:
            break;
    }

}


int main(void) {
    uint8_t x[2] = {0, 0};
    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_RWS(3); //needed so when we clock up we don't outrun flash
    PORT_Initialize();
    CLOCK_Initialize();
    SYSTICK_TimerInitialize();
    SYSTICK_TimerStart();
    DMAC_Initialize();
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, uartRxCallback, (uintptr_t) &ftdiRead);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_3, uartRxCallback, (uintptr_t) &rxRead);
    NVIC_Initialize();
    Timer_Init(TC2_REGS);
    TCC0_PWMInitialize();

    ADC0_Initialize(); //todo re-order some of this stuff so we start connecting to the RX faster?
    ADC0_Enable();

    SERCOM_USART_Initialize(FTDI);
    SERCOM_USART_Initialize(RX);
    L9958_Init();

    //todo read reset-cause?
    //todo set PAC after configuring peripherals
    //todo watchdog
    //todo poll ADC for vbat
    //todo telemetry
    //todo mcu temp sensor

    serial_puts("D2 Motherboard\r\n");
    serial_gets(x, 1);
    fport_trigger(1);
    for(;;) {
        if(ftdiRead) {
            ftdiRead = false;
            cmd_prompt(x[0]);
            serial_gets(x, 1);
        }
        if(rxRead) {
            rxRead = false;
            proc_fport_rx();
        }
        L9958_Tick();
    }
}

