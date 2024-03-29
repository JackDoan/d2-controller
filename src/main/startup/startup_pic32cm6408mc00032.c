/**
 * \file
 *
 * \brief GCC startup file for PIC32CM6408MC00032
 *
 * Copyright (c) 2022 Microchip Technology Inc.
 *
 * \license_start
 *
 * \page License
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \license_stop
 *
 */

#include "pic32cm6408mc00032.h"
#include "interrupts.h"

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

/* Optional application-provided functions */
extern void __attribute__((weak,long_call)) _on_reset(void);
extern void __attribute__((weak,long_call)) _on_bootstrap(void);

int main(void);

void __libc_init_array(void);

/* Reset handler */
void Reset_Handler(void);

void Dummy_Handler(void) {
    while (1) {}
}

/* Cortex-M0PLUS core handlers */
void NonMaskableInt_Handler ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void HardFault_Handler    ( void ) {
    while (1) {}
}

/* Exception Table */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

        /* Configure Initial Stack Pointer, using linker-generated symbols */
        .pvStack = (void*) (&_estack),

        .pfnReset_Handler              = (void*) Reset_Handler,
        .pfnNonMaskableInt_Handler     = (void*) NonMaskableInt_Handler,
        .pfnHardFault_Handler          = (void*) HardFault_Handler,
        .pvReservedC12                 = (void*) (0UL), /* Reserved */
        .pvReservedC11                 = (void*) (0UL), /* Reserved */
        .pvReservedC10                 = (void*) (0UL), /* Reserved */
        .pvReservedC9                  = (void*) (0UL), /* Reserved */
        .pvReservedC8                  = (void*) (0UL), /* Reserved */
        .pvReservedC7                  = (void*) (0UL), /* Reserved */
        .pvReservedC6                  = (void*) (0UL), /* Reserved */
        .pfnSVCall_Handler             = (void*) Dummy_Handler,
        .pvReservedC4                  = (void*) (0UL), /* Reserved */
        .pvReservedC3                  = (void*) (0UL), /* Reserved */
        .pfnPendSV_Handler             = (void*) Dummy_Handler,
        .pfnSysTick_Handler            = (void*) SysTick_Handler,

        /* Configurable interrupts */
        .pfnSYSTEM_Handler             = (void*) Dummy_Handler, /* 0  Main Clock */
        .pfnWDT_Handler                = (void*) Dummy_Handler,    /* 1  Watchdog Timer */
        .pfnRTC_Handler                = (void*) Dummy_Handler,    /* 2  Real-Time Counter */
        .pfnEIC_Handler                = (void*) Dummy_Handler,    /* 3  External Interrupt Controller */
        .pfnFREQM_Handler              = (void*) Dummy_Handler,  /* 4  Frequency Meter */
        .pfnTSENS_Handler              = (void*) Dummy_Handler,  /* 5  Temperature Sensor */
        .pfnNVMCTRL_Handler            = (void*) Dummy_Handler, /* 6  Non-Volatile Memory Controller */
        .pfnDMAC_Handler               = (void*) DMAC_InterruptHandler,   /* 7  Direct Memory Access Controller */
        .pfnEVSYS_Handler              = (void*) Dummy_Handler,  /* 8  Event System Interface */
        .pfnSERCOM0_Handler            = (void*) SERCOM0_USART_InterruptHandler, /* 9  Serial Communication Interface */
        .pfnSERCOM1_Handler            = (void*) SERCOM1_USART_InterruptHandler, /* 10 Serial Communication Interface */
        .pfnSERCOM2_Handler            = (void*) Dummy_Handler, /* 11 Serial Communication Interface */
        .pfnSERCOM3_Handler            = (void*) SERCOM_SPI_InterruptHandler, /* 12 Serial Communication Interface */
        .pfnTCC0_Handler               = (void*) Dummy_Handler,   /* 13 Timer Counter for Control Applications */
        .pfnTCC1_Handler               = (void*) Dummy_Handler,   /* 14 Timer Counter for Control Applications */
        .pfnTCC2_Handler               = (void*) Dummy_Handler,   /* 15 Timer Counter for Control Applications */
        .pfnTC0_Handler                = (void*) Dummy_Handler,    /* 16 Basic Timer Counter */
        .pfnTC1_Handler                = (void*) Dummy_Handler,    /* 17 Basic Timer Counter */
        .pfnTC2_Handler                = (void*) TC2_InterruptHandler,    /* 18 Basic Timer Counter */
        .pfnTC3_Handler                = (void*) Dummy_Handler,    /* 19 Basic Timer Counter */
        .pfnTC4_Handler                = (void*) Dummy_Handler,    /* 20 Basic Timer Counter */
        .pfnADC0_Handler               = (void*) Dummy_Handler,   /* 21 Analog Digital Converter */
        .pfnADC1_Handler               = (void*) Dummy_Handler,   /* 22 Analog Digital Converter */
        .pfnAC_Handler                 = (void*) Dummy_Handler,     /* 23 Analog Comparators */
        .pfnDAC_Handler                = (void*) Dummy_Handler,    /* 24 Digital Analog Converter */
        .pfnSDADC_Handler              = (void*) Dummy_Handler,  /* 25 Sigma-Delta Analog Digital Converter */
        .pfnPDEC_Handler               = (void*) Dummy_Handler    /* 26 Quadrature Decodeur */
};

/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */
void Reset_Handler(void) {
    uint32_t *pSrc, *pDest;

    /* Initialize the relocate segment */
    pSrc = &_etext;
    pDest = &_srelocate;

    if (pSrc != pDest) {
            for (; pDest < &_erelocate;) {
                    *pDest++ = *pSrc++;
            }
    }

    /* Clear the zero segment */
    for (pDest = &_szero; pDest < &_ezero;) {
            *pDest++ = 0;
    }

    /* Set the vector table base address */
    pSrc = (uint32_t *) & _sfixed;
    SCB->VTOR = ((uint32_t) pSrc & SCB_VTOR_TBLOFF_Msk);

    /* Initialize the C library */
    __libc_init_array();

    main();
    while (1);
}

void end(void) {}

void _init(void) {;}