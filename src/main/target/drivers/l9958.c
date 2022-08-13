#include "l9958.h"
#include "device.h"
#include "sercom_spi_master.h"


struct l9958_context {
    enum motor_channel active;
    uint8_t rx_buf[MOTOR_COUNT][2];
};

static struct l9958_context g_context = {0};
static uint8_t config_data[] = {0b00001110, 0b00000000};
static PORT_PIN cs_lines[] = {CS1, CS2, CS3, CS4};

void do_config_transaction(enum motor_channel channel) {
    SERCOM_SPI_WriteRead(
        cs_lines[channel],
        config_data,
        2,
        g_context.rx_buf[channel],
        2
    );
}

static void spi_callback(uintptr_t context) {
    g_context.active = (g_context.active + 1) % MOTOR_COUNT;
}

uint16_t L9958_Diag_Read(enum motor_channel channel) {
    return g_context.rx_buf[channel][0] | (g_context.rx_buf[channel][1] << 8);
}

void L9958_Init(void) {
    SERCOM_SPI_Initialize(SPI);
    SERCOM_SPI_CallbackRegister(spi_callback, (uintptr_t)FTDI);
}

void L9958_Tick(void) {
    //todo check register contents for errors and act on them
    //todo kick off reads on a regular interval (not as fast as possible though?)
    do_config_transaction(g_context.active);
}