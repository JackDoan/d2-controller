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

char dingdong[32] = {0};
uint8_t spi_rx[2] = {0};

void spi_callback(uintptr_t context) {
    sprintf(dingdong, "%02x %02x\r\n", spi_rx[0], spi_rx[1]);
    serial_puts(dingdong);
}
char welcome_str[] = "D2 Motherboard\r\n";
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
        default:
            break;
    }
}

static bool ftdiRead = false;
static void ftdiRxCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    ftdiRead = true;
}

static bool rxRead = false;
static void fportRxCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    rxRead = true;
}

int main(void) {
    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_RWS(3);
    PORT_Initialize();
    CLOCK_Initialize();
    DMAC_Initialize();
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, ftdiRxCallback, 0);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_3, fportRxCallback, 0);
    NVIC_Initialize();
    Timer_Init(TC2_REGS);
    TCC0_PWMInitialize();

    SERCOM_USART_Initialize(FTDI);
    SERCOM_USART_Initialize(RX);
    SERCOM_SPI_Initialize(SPI);

    SERCOM_SPI_CallbackRegister(spi_callback, (uintptr_t)FTDI);
    //todo read reset-cause?
    //todo set PAC after configuring peripherals
    //todo watchdog
    //todo periodic spi polling

    serial_puts(welcome_str);
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

