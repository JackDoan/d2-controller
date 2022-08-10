#include <stdio.h>
#include "plib_port.h"
#include "plib_clock.h"
#include "sercom_usart.h"
#include "plib_tcc0.h"
#include "sercom_spi_master.h"
#include "plib_nvic.h"
#include "plib_dmac.h"

char dingdong[32] = {0};
uint8_t spi_rx[2] = {0};

void spi_callback(uintptr_t context) {
    sprintf(dingdong, "%02x %02x\r\n", spi_rx[0], spi_rx[1]);
    serial_puts(dingdong);
}
char welcome_str[] = "D2 Motherboard\r\n";
uint8_t x[4] = {0};
int main(void) {
    //todo do I need this to clock higher?
    //NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_RWS(3);
    PORT_Initialize();
    CLOCK_Initialize();
    DMAC_Initialize();
    NVIC_Initialize();

    TCC0_PWMInitialize();
    TCC0_PWM_Enable(true); //todo remove?

    SERCOM_USART_Initialize(FTDI);
    SERCOM_SPI_Initialize(SPI);

    SERCOM_SPI_CallbackRegister(spi_callback, (uintptr_t)FTDI);
    //todo read reset-cause?
    //todo set PAC after configuring peripherals
    //todo watchdog


    //SERCOM_USART_Write_Nonblock(FTDI, welcome_str, sizeof(welcome_str)-1);
    serial_puts(welcome_str);
    //DMAC_ChannelTransfer(DMAC_CHANNEL_0, welcome_str, (const void *)&FTDI->USART_INT.SERCOM_DATA, sizeof(welcome_str)-1);
    //SERCOM_USART_TX_Wait(FTDI);
    uint8_t spi_tx[4] = {0b00001110, 0b00000000, 0, 0};

    while(SERCOM_SPI_IsBusy(SPI)) {}
    for(;;) {
        if(SERCOM_USART_Read(FTDI, x, 1)) {
            SERCOM_USART_Write_Nonblock(FTDI, x, 1);
            switch(x[0]) {
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
                    break;
            }
        }
    }
}

//static void TransmitCompleteCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
//{
//    writeStatus = true;
//}
//
//static void ReceiveCompleteCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
//{
//    readStatus = true;
//}

//int dma_main(void) {
//    DMAC_Initialize();
//    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, TransmitCompleteCallback,0);
//    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, ReceiveCompleteCallback,0);
//    DMAC_ChannelTransfer(DMAC_CHANNEL_0, &messageStart, (const void *)&SERCOM0_REGS->USART_INT.SERCOM_DATA, sizeof(messageStart));
//
//    while ( true ) {
//        if(readStatus == true) {
//            /* Echo back received buffer and Toggle LED */
//            readStatus = false;
//
//            memcpy(echoBuffer, receiveBuffer,sizeof(receiveBuffer));
//            echoBuffer[sizeof(receiveBuffer)]='\r';
//            echoBuffer[sizeof(receiveBuffer)+1]='\n';
//
//            DMAC_ChannelTransfer(DMAC_CHANNEL_0, &echoBuffer, (const void *)&SERCOM0_REGS->USART_INT.SERCOM_DATA, sizeof(echoBuffer));
//            LED_Toggle();
//        }
//        else if(writeStatus == true) {
//            /* Submit buffer to read user data */
//            writeStatus = false;
//            DMAC_ChannelTransfer(DMAC_CHANNEL_1, (const void *)&SERCOM0_REGS->USART_INT.SERCOM_DATA, &receiveBuffer, sizeof(receiveBuffer));
//        }
//    }
//}

