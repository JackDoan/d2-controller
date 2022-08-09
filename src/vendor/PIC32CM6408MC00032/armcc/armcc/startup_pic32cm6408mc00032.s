;/*****************************************************************************
; * \file     startup_pic32cm6408mc00032.s
; * \brief    ARMCC startup file for PIC32CM6408MC00032
; *
; * \note
; * Copyright (C) 2022 Microchip Technology Inc.
; *
; * \par
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the "License");
; * you may not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; *   http://www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an "AS IS" BASIS,
; * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; *
; ******************************************************************************/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NonMaskableInt_Handler    ; NonMaskableInt Handler
                DCD     HardFault_Handler         ; HardFault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVC Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     SYSTEM_Handler            ; 0  Main Clock Handler
                DCD     WDT_Handler               ; 1  Watchdog Timer Handler
                DCD     RTC_Handler               ; 2  Real-Time Counter Handler
                DCD     EIC_Handler               ; 3  External Interrupt Controller Handler
                DCD     FREQM_Handler             ; 4  Frequency Meter Handler
                DCD     TSENS_Handler             ; 5  Temperature Sensor Handler
                DCD     NVMCTRL_Handler           ; 6  Non-Volatile Memory Controller Handler
                DCD     DMAC_Handler              ; 7  Direct Memory Access Controller Handler
                DCD     EVSYS_Handler             ; 8  Event System Interface Handler
                DCD     SERCOM0_Handler           ; 9  Serial Communication Interface Handler
                DCD     SERCOM1_Handler           ; 10 Serial Communication Interface Handler
                DCD     SERCOM2_Handler           ; 11 Serial Communication Interface Handler
                DCD     SERCOM3_Handler           ; 12 Serial Communication Interface Handler
                DCD     TCC0_Handler              ; 13 Timer Counter for Control Applications Handler
                DCD     TCC1_Handler              ; 14 Timer Counter for Control Applications Handler
                DCD     TCC2_Handler              ; 15 Timer Counter for Control Applications Handler
                DCD     TC0_Handler               ; 16 Basic Timer Counter Handler
                DCD     TC1_Handler               ; 17 Basic Timer Counter Handler
                DCD     TC2_Handler               ; 18 Basic Timer Counter Handler
                DCD     TC3_Handler               ; 19 Basic Timer Counter Handler
                DCD     TC4_Handler               ; 20 Basic Timer Counter Handler
                DCD     ADC0_Handler              ; 21 Analog Digital Converter Handler
                DCD     ADC1_Handler              ; 22 Analog Digital Converter Handler
                DCD     AC_Handler                ; 23 Analog Comparators Handler
                DCD     DAC_Handler               ; 24 Digital Analog Converter Handler
                DCD     SDADC_Handler             ; 25 Sigma-Delta Analog Digital Converter Handler
                DCD     PDEC_Handler              ; 26 Quadrature Decodeur Handler
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NonMaskableInt_Handler    PROC
                          EXPORT NonMaskableInt_Handler    [WEAK] 
                          B       .
                          ENDP
HardFault_Handler         PROC
                          EXPORT HardFault_Handler         [WEAK] 
                          B       .
                          ENDP
SVC_Handler               PROC
                          EXPORT SVC_Handler               [WEAK] 
                          B       .
                          ENDP
PendSV_Handler            PROC
                          EXPORT PendSV_Handler            [WEAK] 
                          B       .
                          ENDP
SysTick_Handler           PROC
                          EXPORT SysTick_Handler           [WEAK] 
                          B       .
                          ENDP

Default_Handler PROC
                EXPORT  SYSTEM_Handler            [WEAK]
                EXPORT  WDT_Handler               [WEAK]
                EXPORT  RTC_Handler               [WEAK]
                EXPORT  EIC_Handler               [WEAK]
                EXPORT  FREQM_Handler             [WEAK]
                EXPORT  TSENS_Handler             [WEAK]
                EXPORT  NVMCTRL_Handler           [WEAK]
                EXPORT  DMAC_Handler              [WEAK]
                EXPORT  EVSYS_Handler             [WEAK]
                EXPORT  SERCOM0_Handler           [WEAK]
                EXPORT  SERCOM1_Handler           [WEAK]
                EXPORT  SERCOM2_Handler           [WEAK]
                EXPORT  SERCOM3_Handler           [WEAK]
                EXPORT  TCC0_Handler              [WEAK]
                EXPORT  TCC1_Handler              [WEAK]
                EXPORT  TCC2_Handler              [WEAK]
                EXPORT  TC0_Handler               [WEAK]
                EXPORT  TC1_Handler               [WEAK]
                EXPORT  TC2_Handler               [WEAK]
                EXPORT  TC3_Handler               [WEAK]
                EXPORT  TC4_Handler               [WEAK]
                EXPORT  ADC0_Handler              [WEAK]
                EXPORT  ADC1_Handler              [WEAK]
                EXPORT  AC_Handler                [WEAK]
                EXPORT  DAC_Handler               [WEAK]
                EXPORT  SDADC_Handler             [WEAK]
                EXPORT  PDEC_Handler              [WEAK]


SYSTEM_Handler           
WDT_Handler              
RTC_Handler              
EIC_Handler              
FREQM_Handler            
TSENS_Handler            
NVMCTRL_Handler          
DMAC_Handler             
EVSYS_Handler            
SERCOM0_Handler          
SERCOM1_Handler          
SERCOM2_Handler          
SERCOM3_Handler          
TCC0_Handler             
TCC1_Handler             
TCC2_Handler             
TC0_Handler              
TC1_Handler              
TC2_Handler              
TC3_Handler              
TC4_Handler              
ADC0_Handler             
ADC1_Handler             
AC_Handler               
DAC_Handler              
SDADC_Handler            
PDEC_Handler             
                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END

