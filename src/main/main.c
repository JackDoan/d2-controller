#include "plib_port.h"
#include "plib_clock.h"
#include "sercom_usart.h"
#include "plib_tcc0.h"
#include "sercom_spi_master.h"
#include "plib_nvic.h"

const char dingdong[] = "Ding dong\r\n";
void spi_callback(uintptr_t context) {

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
    uint8_t spi_tx[2] = {0x55, 0xaa};
    uint8_t spi_rx[2] = {0};

    SERCOM_SPI_WriteRead(SPI, spi_tx, sizeof(spi_tx), spi_rx, sizeof(spi_rx));
    for(;;) {
        if(SERCOM_USART_Read(FTDI, x, 1)) {
            SERCOM_USART_Write_Nonblock(FTDI, x, 1);
        }
    }
}