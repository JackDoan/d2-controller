#include <stddef.h>
#include "interrupts.h"
#include "plib_tcc0.h"

struct tcc_config {
    uint32_t DRVCTRL;
    uint32_t TCC_PER;
    uint32_t TCC_CTRLA;
    uint32_t TCC_WEXCTRL;
};

//(48M / 2 / TCC_PER = freq)
static struct tcc_config configs[] = {
        {
            .DRVCTRL = TCC_DRVCTRL_INVEN2_Msk |
                    TCC_DRVCTRL_INVEN3_Msk |
                    TCC_DRVCTRL_INVEN4_Msk |
                    TCC_DRVCTRL_INVEN5_Msk,
            .TCC_PER = 1300,  //~19Khz
            .TCC_CTRLA = TCC_CTRLA_PRESCALER_DIV1,
            .TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0UL),
        },
        {
            .DRVCTRL = TCC_DRVCTRL_INVEN_ALL_Msk,
            .TCC_PER = 1860*16, //50Hz
            .TCC_CTRLA = TCC_CTRLA_PRESCALER_DIV16,
            .TCC_WEXCTRL = TCC_WEXCTRL_OTMX(2UL), //cc0 is all channels
        },
};

static inline struct tcc_config* get_config(tcc_registers_t* regs) {
    switch((uint32_t)regs) {
        case (uint32_t)TCC0_REGS:
            return &configs[0];
        case (uint32_t)TCC1_REGS:
            return &configs[1];
    }
    return NULL;
}

static inline void TCC_Sync(tcc_registers_t* regs, uint32_t mask) {
    while ((regs->TCC_SYNCBUSY & mask) != 0U) { /* Wait for sync */ }
}

/* Initialize TCC module */
void TCC_PWMInitialize(tcc_registers_t* regs) {
    struct tcc_config* config = get_config(regs);
    /* Reset TCC */
    regs->TCC_CTRLA = TCC_CTRLA_SWRST_Msk;
    TCC_Sync(regs, TCC_SYNCBUSY_SWRST_Msk);

    /* Clock prescaler */
    regs->TCC_CTRLA = config->TCC_CTRLA ;
    regs->TCC_WEXCTRL = config->TCC_WEXCTRL;

    regs->TCC_DRVCTRL |= config->DRVCTRL;

    regs->TCC_WAVE = TCC_WAVE_WAVEGEN_DSTOP;
    TCC_Sync(regs, TCC_SYNCBUSY_WAVE_Msk);

    /* Configure duty cycle values */
    regs->TCC_CC[0] = 0;
    TCC_Sync(regs, TCC_SYNCBUSY_CC0_Msk);
    regs->TCC_CC[1] = 0;
    TCC_Sync(regs, TCC_SYNCBUSY_CC1_Msk);
    regs->TCC_CC[2] = 0;
    TCC_Sync(regs, TCC_SYNCBUSY_CC2_Msk);
    regs->TCC_CC[3] = 0;
    TCC_Sync(regs, TCC_SYNCBUSY_CC3_Msk);
    //1200 works out to like exactly 20kHz
    //2000 1420
    //3000 1440
    regs->TCC_PER = config->TCC_PER;
    TCC_Sync(regs, TCC_SYNCBUSY_PER_Msk);

//    regs->TCC_INTENSET = TCC_INTENSET_OVF_Msk;
    regs->TCC_DBGCTRL |= TCC_DBGCTRL_DBGRUN_Msk;

    regs->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    TCC_Sync(regs, TCC_SYNCBUSY_ENABLE_Msk);
}

void TCC_PWMStart(tcc_registers_t* regs) {
    regs->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    TCC_Sync(regs, TCC_SYNCBUSY_ENABLE_Msk);
}

void TCC_PWMStop(tcc_registers_t* regs) {
    regs->TCC_CTRLA &= ~TCC_CTRLA_ENABLE_Msk;
    TCC_Sync(regs, TCC_SYNCBUSY_ENABLE_Msk);
}

void TCC_PWM_Enable(tcc_registers_t* regs, bool enable) {
    if(enable)
        regs->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    else
        regs->TCC_CTRLA &= ~TCC_CTRLA_ENABLE_Msk;
    TCC_Sync(regs, TCC_SYNCBUSY_ENABLE_Msk);
}

bool TCC_PWM24bitPeriodSet(tcc_registers_t* regs, uint32_t period) {
    if ((regs->TCC_STATUS & (TCC_STATUS_PERBUFV_Msk)) != 0U)
        return false;

    regs->TCC_PERBUF = period & 0xFFFFFFU;
    return true;
}

uint32_t TCC_PWM24bitPeriodGet(tcc_registers_t* regs) {
    TCC_Sync(regs, TCC_SYNCBUSY_PER_Msk);
    return (regs->TCC_PER & 0xFFFFFFU);
}

void TCC_PWM24bitCounterSet(tcc_registers_t* regs, uint32_t count) {
    regs->TCC_COUNT = count & 0xFFFFFFU;
    TCC_Sync(TCC0_REGS, TCC_SYNCBUSY_COUNT_Msk);
}
