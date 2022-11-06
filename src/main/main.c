#include <stdio.h>
#include "plib_port.h"
#include "plib_clock.h"
#include "sercom_usart.h"
#include "plib_tcc0.h"
#include "plib_nvic.h"
#include "plib_dmac.h"
#include "rx/fport.h"
#include "timer.h"
#include "plib_adc0.h"
#include "plib_systick.h"
#include "l9958.h"
#include "plib_wdt.h"
#include "plib_nvmctrl.h"
#include "plib_pdec.h"

char cmd_resp_buf[64] = {0};

void cmd_prompt(char cmd) {
    switch(cmd) {
        case 'e':
            PORT_PinWrite(EN1, true);
            break;
        case 'E':
            PORT_PinWrite(EN1, false);
            break;
        case 'f':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "FAULT: %04x\r\n", PORT_PinRead(FAULT));
            serial_puts(cmd_resp_buf);
            break;
        case 'p':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "PDEC: %08lx\r\n", PDEC_Counts());
            serial_puts(cmd_resp_buf);
            break;
        case '1':
//            motor_enable(MOTOR1, true);
//            motor_set_speed(MOTOR1, 1200);
            PORT_PinWrite(EN1, true);
            PORT_PinWrite(M1_OUT, false);
            PORT_PinWrite(M2_OUT, true);
            break;
        case '!':
//            motor_enable(MOTOR1, false);
//            motor_set_speed(MOTOR1, 0);
            PORT_PinWrite(EN1, false);
            PORT_PinWrite(M1_OUT, false);
            PORT_PinWrite(M2_OUT, false);
            break;
        case '2':
            motor_enable(MOTOR2, true);
            motor_set_speed(MOTOR2, 0);
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
        case 'a':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "VBatt: %d\r\nTSens: %lu\r\n", ADC0_Convert_mV(), TSENS_Get());
            serial_puts(cmd_resp_buf);
            break;
        case 't':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "SysTick: %lu\r\n", SYSTICK_GetTickCounter());
            serial_puts(cmd_resp_buf);
            break;
        case 'r':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "Reset Cause: %s\r\n", RSTC_ResetCauseGetStr());
            serial_puts(cmd_resp_buf);
            break;
        case 'R':
            serial_puts("reset test disabled\r\n");
            //todo safeguard? NVIC_SystemReset();
            break;
        case 'W':
            serial_puts("watchdog test disabled\r\n");
            //todo safeguard? for(;;) {}
            break;
        default:
            break;
    }
}

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
    NVMCTRL_Initialize();
    PORT_Initialize();
    CLOCK_Initialize();
    SYSTICK_TimerInitialize();
    SYSTICK_TimerStart();
    DMAC_Initialize();
    DMAC_ChannelCallbackRegister(FTDI_RX_DMA_CHANNEL, uartRxCallback, (uintptr_t) &ftdiRead);
    //fport_dma_register();
    NVIC_Initialize();
    //Timer_Init(TC2_REGS);
    TCC_PWMInitialize(TCC0_REGS);
    TCC_PWMInitialize(TCC1_REGS);
    PDEC_Init();

    ADC0_Initialize(); //todo re-order some of this stuff so we start connecting to the RX faster?
    ADC0_Enable();
    ADC0_ConversionStart();

    TSENS_Init();

    WDT_Enable();

    SERCOM_USART_Initialize(FTDI);
    //SERCOM_USART_Initialize(RX);
    //L9958_Init();

    //todo set PAC after configuring peripherals
    //todo calibration of pulse len
    //todo improve CRC error rate?
    //todo configure brown-out detector
    //todo read more than one byte at a time

    serial_puts("D2 Motherboard\r\n");
    serial_gets(x, 1);
    for(uint32_t i = 0; true; i++) {
        //fport_tick();
        if(ftdiRead) {
            ftdiRead = false;
            cmd_prompt(x[0]);
            serial_gets(x, 1);
        }
//        SERCOM_USART_WriteByte(FTDI, 'U');
        if(i & 1) {  // don't need to do this every cycle
            //L9958_Tick();
        }
        WDT_Clear();
    }
}

