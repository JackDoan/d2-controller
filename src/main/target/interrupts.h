#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

void SERCOM_SPI_InterruptHandler(void);
void SERCOM1_USART_InterruptHandler(void);
void SERCOM0_USART_InterruptHandler(void);
//void SERCOM0_USART_InterruptHandler (void);

#endif // INTERRUPTS_H