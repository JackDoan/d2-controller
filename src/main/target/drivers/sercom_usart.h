/*******************************************************************************
  SERCOM Universal Synchronous/Asynchrnous Receiver/Transmitter PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom0_usart.h

  Summary
    USART peripheral library interface.

  Description
    This file defines the interface to the USART peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef PLIB_SERCOM0_USART_H // Guards against multiple inclusion
#define PLIB_SERCOM0_USART_H


#include "plib_sercom_usart_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

void SERCOM_USART_Initialize(sercom_registers_t* sercom);

void SERCOM_USART_TransmitterEnable(sercom_registers_t* sercom);
void SERCOM_USART_TransmitterDisable(sercom_registers_t* sercom);

bool SERCOM_USART_Write(sercom_registers_t* sercom, const void *buffer, const size_t size);
bool SERCOM_USART_Write_Nonblock(sercom_registers_t* sercom, void *buffer, const size_t size);
bool serial_busy(void);
void serial_puts(void *buffer);
void debug_puts(void *buffer);
void serial_gets(void *buffer, size_t len);
void fport_gets(void *buffer, size_t len);
void SERCOM_USART_TX_Wait(sercom_registers_t* sercom);
void SERCOM_USART_RX_Wait(sercom_registers_t* sercom);
bool SERCOM_USART_TransmitComplete(sercom_registers_t* sercom);


bool SERCOM_USART_TransmitterIsReady(sercom_registers_t* sercom);

void SERCOM_USART_WriteByte(sercom_registers_t* sercom, int data);


void SERCOM_USART_ReceiverEnable(sercom_registers_t* sercom);

void SERCOM_USART_ReceiverDisable(sercom_registers_t* sercom);

bool SERCOM_USART_Read(sercom_registers_t* sercom, void *buffer, const size_t size);

bool SERCOM_USART_ReadIsBusy(sercom_registers_t* sercom);
size_t SERCOM_USART_ReadCountGet(sercom_registers_t* sercom);
bool SERCOM_USART_ReadAbort(sercom_registers_t* sercom);




bool SERCOM_USART_ReceiverIsReady(sercom_registers_t* sercom);

int SERCOM_USART_ReadByte(sercom_registers_t* sercom);

USART_ERROR SERCOM0_USART_ErrorGet(void);

uint32_t SERCOM0_USART_FrequencyGet(void);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //PLIB_SERCOM0_USART_H
