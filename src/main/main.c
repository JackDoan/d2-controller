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

char cmd_resp_buf[128] = {0};

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
            motor_set_speed(MOTOR2, 1800);
            break;
        case '@':
            motor_enable(MOTOR2, false);
            motor_set_speed(MOTOR2, 173);
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
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "1: %04x / 2: %04x / 3: %04x / 4: %04x\r\n",
                     L9958_Diag_Read(MOTOR1), L9958_Diag_Read(MOTOR2),
                     L9958_Diag_Read(MOTOR3), L9958_Diag_Read(MOTOR4));
            serial_puts(cmd_resp_buf);
            break;
        case 'q': {
            struct packet_stats *stats = fport_get_stats();
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "B: %lu/%lu | P: %lu/%lu | CRC: %lu | DMA: %lu | RSSI: %lu | FS: %lu | sig: %lu | timeouts: %lu\r\n",
                     stats->total_bytes, stats->discarded_bytes,
                     stats->total_packets, stats->total_packets-stats->valid_packets,
                     stats->crc_fail,
                     stats->dma_error,
                     stats->rssi_invalid,
                     stats->failsafe_active,
                     stats->signal_loss,
                     stats->packet_timeouts
             );
            serial_puts(cmd_resp_buf);
            break;
        }

        case 'p':
            fport_enable_printing(true);
            fport_enable_printing_bytes(false);
            break;
        case 'P':
            fport_enable_printing(false);
            fport_enable_printing_bytes(false);
            break;
        case 'o':
            fport_enable_printing(false);
            fport_enable_printing_bytes(true);
            break;
        case 'O':
            fport_enable_printing(false);
            fport_enable_printing_bytes(false);
            break;
        case 'a':
            snprintf(cmd_resp_buf, sizeof(cmd_resp_buf), "VBatt: %d / TSens: %lu\r\n", ADC0_Convert_mV(), TSENS_Get());
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
            debug_puts("?\r\n");
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
    DMAC_ChannelCallbackRegister(FTDI_DMA_CHANNEL, uartRxCallback, (uintptr_t) &ftdiRead);
    NVIC_Initialize();
    Timer_Init(TC2_REGS);
    TCC_PWMInitialize(TCC0_REGS);
    TCC_PWMInitialize(TCC1_REGS);

    ADC0_Initialize();
    ADC0_Enable();
    ADC0_ConversionStart();
    TSENS_Init();
    WDT_Enable();

    SERCOM_USART_Initialize(FTDI);
    SERCOM_USART_Initialize(RX);
    fport_dma_register();

    L9958_Init();
    //todo set PAC after configuring peripherals
    //todo configure brown-out detector

    serial_puts("D2 Motherboard\r\n");
    serial_gets(x, 1);
    for(;;) {
        if(ftdiRead) {
            ftdiRead = false;
            cmd_prompt(x[0]);
            serial_gets(x, 1);
        }
        L9958_Tick();
        WDT_ClearWithSync();
        //__WFI();
        SYSTICK_DelayMs(1);
    }
}
