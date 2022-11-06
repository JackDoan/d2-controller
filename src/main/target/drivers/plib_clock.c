#include "plib_clock.h"
#include "device.h"
#include "interrupts.h"

static void OSCCTRL_Initialize(void) {
    uint32_t calibValue = (uint32_t)(((*(uint64_t*)0x806020UL) >> 19 ) & 0x3fffffUL);
    OSCCTRL_REGS->OSCCTRL_CAL48M = calibValue;
    OSCCTRL_REGS->OSCCTRL_OSC48MDIV = (uint8_t)OSCCTRL_OSC48MDIV_DIV(0UL);

    while((OSCCTRL_REGS->OSCCTRL_OSC48MSYNCBUSY & OSCCTRL_OSC48MSYNCBUSY_OSC48MDIV_Msk) == OSCCTRL_OSC48MSYNCBUSY_OSC48MDIV_Msk){ }
    while((OSCCTRL_REGS->OSCCTRL_STATUS & OSCCTRL_STATUS_OSC48MRDY_Msk) != OSCCTRL_STATUS_OSC48MRDY_Msk) { }
}

static void OSC32KCTRL_Initialize(void) {
    OSC32KCTRL_REGS->OSC32KCTRL_OSC32K = 0x0UL;
    OSC32KCTRL_REGS->OSC32KCTRL_RTCCTRL = OSC32KCTRL_RTCCTRL_RTCSEL(0UL);
    OSC32KCTRL_REGS->OSC32KCTRL_OSCULP32K |= OSC32KCTRL_OSCULP32K_WRTLOCK_Msk | OSC32KCTRL_OSCULP32K_EN1K_Msk;
}


static void GCLK0_Initialize(void) {
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1UL) | GCLK_GENCTRL_SRC(6UL) | GCLK_GENCTRL_GENEN_Msk;
    GCLK_REGS->GCLK_GENCTRL[2] = GCLK_GENCTRL_DIV(0UL) | GCLK_GENCTRL_SRC(6UL) | GCLK_GENCTRL_GENEN_Msk;
    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk) { }
    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL2_Msk) == GCLK_SYNCBUSY_GENCTRL2_Msk) { }
}

enum gclk_router_channels {
    GCLK_TSENS = 5,
    GCLK_SERCOM0_CORE = 19,
    GCLK_SERCOM1_CORE = 20,
    GCLK_SERCOM2_CORE = 21,
    GCLK_SERCOM3_CORE = 22,
    GCLK_TCC0_TCC1 = 23,
    GCLK_TC0_TC1 = 25,
    GCLK_TC2_TC3 = 26,
    GCLK_ADC0 = 28,
    GCLK_PDEC = 34,
};

void CLOCK_Initialize (void) {
    /* Function to Initialize the Oscillators */
    OSCCTRL_Initialize();

    /* Function to Initialize the 32KHz Oscillators */
    OSC32KCTRL_Initialize();

    GCLK0_Initialize();

    GCLK_REGS->GCLK_PCHCTRL[GCLK_TSENS] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_SERCOM0_CORE] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_SERCOM1_CORE] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_SERCOM2_CORE] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_SERCOM3_CORE] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_TCC0_TCC1] = GCLK_PCHCTRL_GEN(0x2UL) | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_TC2_TC3] = GCLK_PCHCTRL_GEN(0x2UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_ADC0] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    GCLK_REGS->GCLK_PCHCTRL[GCLK_PDEC] = GCLK_PCHCTRL_GEN(0x0UL)  | GCLK_PCHCTRL_CHEN_Msk;
    //todo set WRTLOCK?

    enum gclk_router_channels regs_to_sync[] = {
            GCLK_TSENS,
            GCLK_SERCOM0_CORE,
            GCLK_SERCOM1_CORE,
            GCLK_SERCOM2_CORE,
            GCLK_SERCOM3_CORE,
            GCLK_TCC0_TCC1,
            GCLK_TC2_TC3,
            GCLK_ADC0
    };

    for(unsigned int i = 0; i < sizeof(regs_to_sync) / sizeof(volatile uint32_t*) ; i++) {
        while((GCLK_REGS->GCLK_PCHCTRL[regs_to_sync[i]] & GCLK_PCHCTRL_CHEN_Msk) != GCLK_PCHCTRL_CHEN_Msk) {}
    }

    /* Configure the APBC Bridge Clocks */
    MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_ADC0_Msk |
            MCLK_APBCMASK_TC2_Msk |
            MCLK_APBCMASK_TCC0_Msk |
            MCLK_APBCMASK_TCC1_Msk |
            MCLK_APBCMASK_EVSYS_Msk |
            MCLK_APBCMASK_PDEC_Msk;
    MCLK_REGS->MCLK_APBAMASK |= MCLK_APBAMASK_TSENS_Msk;
    MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_SERCOM0_Msk | MCLK_APBCMASK_SERCOM1_Msk;
    MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_SERCOM2_Msk | MCLK_APBCMASK_SERCOM3_Msk;
    //0xfe1f;
}
