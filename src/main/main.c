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

char dingdong[32] = {0};
uint8_t spi_rx[2] = {0};

void spi_callback(uintptr_t context) {
    sprintf(dingdong, "%02x %02x\r\n", spi_rx[0], spi_rx[1]);
    serial_puts(dingdong);
}
uint8_t x[4] = {0};

static uint8_t spi_tx[4] = {0b00001110, 0b00000000};

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
            SERCOM_SPI_WriteRead(CS3, spi_tx, 2, spi_rx, 2);
            while(SERCOM_SPI_IsBusy(SPI)) {}
            break;
        case 'p':
            fport_enable_printing(true);
            break;
        case 'P':
            fport_enable_printing(false);
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
    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_RWS(3); //needed so when we clock up we don't outrun flash
    PORT_Initialize();
    CLOCK_Initialize();
    SYSTICK_TimerStart();
    DMAC_Initialize();
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, uartRxCallback, (uintptr_t) &ftdiRead);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_3, uartRxCallback, (uintptr_t) &rxRead);
    NVIC_Initialize();
    Timer_Init(TC2_REGS);
    TCC0_PWMInitialize();

    ADC0_Initialize(); //todo re-order some of this stuff so we start connecting to the RX faster?

    SERCOM_USART_Initialize(FTDI);
    SERCOM_USART_Initialize(RX);
    SERCOM_SPI_Initialize(SPI);

    SERCOM_SPI_CallbackRegister(spi_callback, (uintptr_t)FTDI);
    //todo read reset-cause?
    //todo set PAC after configuring peripherals
    //todo watchdog
    //todo periodic spi polling
    //todo ADC for vbat
    //todo telemetry
    //todo mcu temp sensor

    serial_puts("D2 Motherboard\r\n");
    serial_gets(x, 1);
    fport_trigger(1);
    for(;;) {
        if(ftdiRead) {
            ftdiRead = false;
            serial_puts(x);
            cmd_prompt(x[0]);
            serial_gets(x, 1);
        }
        if(rxRead) {
            rxRead = false;
            proc_fport_rx();
        }
    }
}

