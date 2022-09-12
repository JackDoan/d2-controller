#ifndef PLIB_TCC0_H
#define PLIB_TCC0_H

enum tcc_channel {
    TCC_CHANNEL0,
    TCC_CHANNEL1,
    TCC_CHANNEL2,
    TCC_CHANNEL3,
    TCC_NUM_CHANNELS
};

#include "device.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef enum
{
    TCC0_PWM_STATUS_OVF = TCC_INTFLAG_OVF_Msk,
    TCC0_PWM_STATUS_FAULT_0 = TCC_INTFLAG_FAULT0_Msk,
    TCC0_PWM_STATUS_FAULT_1 = TCC_INTFLAG_FAULT1_Msk,
    TCC0_PWM_STATUS_MC_0 = TCC_INTFLAG_MC0_Msk,
    TCC0_PWM_STATUS_MC_1 = TCC_INTFLAG_MC1_Msk,
    TCC0_PWM_STATUS_MC_2 = TCC_INTFLAG_MC2_Msk,
    TCC0_PWM_STATUS_MC_3 = TCC_INTFLAG_MC3_Msk,
} TCC0_PWM_STATUS;

void TCC_PWMInitialize(tcc_registers_t* regs);
void TCC_PWMStart(tcc_registers_t* regs);
void TCC_PWMStop(tcc_registers_t* regs);
void TCC_PWM_Enable(tcc_registers_t* regs, bool enable);
bool TCC_PWM24bitPeriodSet(tcc_registers_t* regs, uint32_t period);
uint32_t TCC_PWM24bitPeriodGet(tcc_registers_t* regs);
void TCC_PWM24bitCounterSet(tcc_registers_t* regs, uint32_t count);

static inline bool TCC_PWM24bitDutySet(tcc_registers_t* regs, enum tcc_channel channel, uint32_t duty) {
    if ((regs->TCC_STATUS & (1UL << (TCC_STATUS_CCBUFV0_Pos + (uint32_t)channel))) != 0U)
        return false;
    regs->TCC_CCBUF[channel] = duty & 0xFFFFFFU;
    return true;
}

#ifdef __cplusplus
    }
#endif

#endif /* PLIB_TCC0_H */
