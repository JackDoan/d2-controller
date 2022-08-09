#include "plib_port.h"
#include "plib_clock.h"
#include "sercom_usart.h"
#include "plib_tcc0.h"
#include "sercom_spi_master.h"
#include "plib_nvic.h"

void spi_callback(uintptr_t context) {
    const char welcome_str[] = "Ding dong\r\n";
    SERCOM_USART_Write(FTDI, welcome_str, sizeof(welcome_str)-1);
}

int main(void) {
    PORT_Initialize();
    CLOCK_Initialize();

    NVIC_Initialize();

    TCC0_PWMInitialize();
    TCC0_PWM_Enable(true); //todo remove

    SERCOM_USART_Initialize(FTDI);
    SERCOM_SPI_Initialize(SPI);

    SERCOM_SPI_CallbackRegister(spi_callback, NULL);

    const char welcome_str[] = "D2 Motherboard\r\n";
    SERCOM_USART_Write(FTDI, welcome_str, sizeof(welcome_str)-1);

    uint8_t spi_tx[2] = {0x55, 0xaa};
    uint8_t spi_rx[2] = {0};

    SERCOM_SPI_WriteRead(SPI, spi_tx, sizeof(spi_tx), spi_rx, sizeof(spi_rx));
    for(;;) {
        int x = 0;
        if(SERCOM_USART_TransmitComplete(FTDI)) {
            if(SERCOM_USART_Read(FTDI, &x, 1)) {
                SERCOM_USART_WriteByte(FTDI, x);
                do {} while (!SERCOM_USART_TransmitComplete(FTDI));
            }
        }
    }
}