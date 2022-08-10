#include <stdio.h>
#include "plib_port.h"
#include "plib_clock.h"
#include "sercom_usart.h"
#include "plib_tcc0.h"
#include "sercom_spi_master.h"
#include "plib_nvic.h"
char dingdong[] = "Ding dong    \r\n";
uint8_t spi_rx[4] = {0};

void spi_callback(uintptr_t context) {
    PORT_PinSet(CS3);
    sprintf(dingdong, "%x %x %x %x", spi_rx[0], spi_rx[1], spi_rx[2], spi_rx[3]);
    SERCOM_USART_Write_Nonblock((sercom_registers_t*)context, dingdong, sizeof(dingdong)-1);
}

uint8_t x[4] = {0};
int main(void) {
    PORT_Initialize();
    CLOCK_Initialize();

    NVIC_Initialize();

    TCC0_PWMInitialize();
    TCC0_PWM_Enable(true); //todo remove

    SERCOM_USART_Initialize(FTDI);
    SERCOM_SPI_Initialize(SPI);

    SERCOM_SPI_CallbackRegister(spi_callback, (uintptr_t)FTDI);

    const char welcome_str[] = "D2 Motherboard\r\n";

    SERCOM_USART_Write_Nonblock(FTDI, welcome_str, sizeof(welcome_str)-1);
    SERCOM_USART_TX_Wait(FTDI);
    uint8_t spi_tx[4] = {0b00001110, 0b00000000, 0, 0};

    PORT_PinClear(CS3);
    SERCOM_SPI_WriteRead(SPI, spi_tx, sizeof(spi_tx), spi_rx, sizeof(spi_rx));
    for(;;) {
        if(SERCOM_USART_Read(FTDI, x, 1)) {
            SERCOM_USART_Write_Nonblock(FTDI, x, 1);
            switch(x[0]) {
                case '3':
                    motor_enable(MOTOR3, true);
                    TCC0_REGS->TCC_CC[2] = 1200;
                    break;
                case '#':
                    motor_enable(MOTOR3, false);
                    TCC0_REGS->TCC_CC[2] = 0;
                    break;
                case 'd':
                    PORT_PinToggle(DIR3);
                    break;
            }
        }
    }
}