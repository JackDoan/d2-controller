#include "timer.h"
#include "device.h"
#include "interrupts.h"

static inline void TC_Sync(tc_count32_registers_t* timer_regs, uint32_t mask) {
    while ((timer_regs->TC_SYNCBUSY & mask) != 0U)
    {
        /* Wait for sync */
    }
}

void Timer_Init(tc_registers_t * regs)
{
    tc_count32_registers_t* timer_regs = (tc_count32_registers_t*)regs;
    timer_regs->TC_CTRLA = TC_CTRLA_SWRST_Msk;
    TC_Sync(timer_regs, TC_SYNCBUSY_SWRST_Msk);

    /* Clock prescaler */
    timer_regs->TC_CTRLA = TC_CTRLA_PRESCALER_DIV16 |
            TC_CTRLA_MODE_COUNT32; //todo configure source


    //note: in some modes, CC0 is a period
    timer_regs->TC_CC[1] = 50000U;
    TC_Sync(timer_regs, TC_SYNCBUSY_CC1_Msk);

    timer_regs->TC_INTENSET = TC_INTENSET_MC1_Msk;
    timer_regs->TC_DBGCTRL |= TC_DBGCTRL_DBGRUN_Msk;

    timer_regs->TC_CTRLA |= TC_CTRLA_ENABLE_Msk;
    TC_Sync(timer_regs, 0xffffffff);

    timer_regs->TC_CTRLBSET = TC_CTRLBSET_CMD_RETRIGGER;
    TC_Sync(timer_regs, 0xffffffff);

}

static void TC_InterruptHandler(tc_count32_registers_t* timer_regs) {
    packet_timer_watchdog_tick();
    //reset the counter
    timer_regs->TC_COUNT = 0;
    timer_regs->TC_CTRLBSET = TC_CTRLBSET_CMD_READSYNC;
    TC_Sync(timer_regs, 0xffffffff);
    timer_regs->TC_CTRLBSET = TC_CTRLBSET_CMD_RETRIGGER;
    /* Clear interrupt flags */
    timer_regs->TC_INTFLAG = TC_INTFLAG_Msk;
    (void)timer_regs->TC_INTFLAG;
}

void TC2_InterruptHandler(void) {
    TC_InterruptHandler((tc_count32_registers_t*)TC2_REGS);
}