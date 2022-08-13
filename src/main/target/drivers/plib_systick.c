#include "device.h"
#include "interrupts.h"
#include "plib_systick.h"

static SYSTICK_OBJECT systick;

void SYSTICK_TimerInitialize(void) {
    SysTick->CTRL = 0U;
    SysTick->VAL = 0U;
    SysTick->LOAD = (SYSTICK_FREQ/1000) - 1U;  //defaults to millisecond period
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
//    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;

    systick.tickCounter = 0U;
    systick.callback = NULL;
}

void SYSTICK_TimerRestart(void) {
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
    SysTick->VAL = 0U;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void SYSTICK_TimerStart(void) {
    SysTick->VAL = 0U;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void SYSTICK_TimerStop(void) {
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
}

void SYSTICK_TimerPeriodSet(uint32_t period) {
    SysTick->LOAD = period - 1U;
}

uint32_t SYSTICK_TimerPeriodGet(void) {
    return(SysTick->LOAD);
}

uint32_t SYSTICK_TimerCounterGet(void) {
    return (SysTick->VAL);
}

uint32_t SYSTICK_TimerFrequencyGet(void) {
    return (SYSTICK_FREQ);
}

void SYSTICK_DelayMs(uint32_t delay_ms) {
   uint32_t elapsedCount=0U, delayCount;
   uint32_t deltaCount, oldCount, newCount, period;

   period = SysTick->LOAD + 1U;

   /* Calculate the count for the given delay */
   delayCount=(SYSTICK_FREQ/1000U)*delay_ms;

   if((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == SysTick_CTRL_ENABLE_Msk)
   {
       oldCount = SysTick->VAL;

       while (elapsedCount < delayCount)
       {
           newCount = SysTick->VAL;
           deltaCount = oldCount - newCount;

           if(newCount > oldCount)
           {
               deltaCount = period - newCount + oldCount;
           }

           oldCount = newCount;
           elapsedCount = elapsedCount + deltaCount;
       }
   }
}

void SYSTICK_DelayUs(uint32_t delay_us) {
   uint32_t elapsedCount=0U, delayCount;
   uint32_t deltaCount, oldCount, newCount, period;

   period = SysTick->LOAD + 1U;

    /* Calculate the count for the given delay */
   delayCount=(SYSTICK_FREQ/1000000U)*delay_us;

   if((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == SysTick_CTRL_ENABLE_Msk) {
       oldCount = SysTick->VAL;

       while (elapsedCount < delayCount) {
           newCount = SysTick->VAL;
           deltaCount = oldCount - newCount;

           if(newCount > oldCount)
           {
               deltaCount = period - newCount + oldCount;
           }

           oldCount = newCount;
           elapsedCount = elapsedCount + deltaCount;
       }
   }
}

uint32_t SYSTICK_GetTickCounter(void) {
	return systick.tickCounter; 
}

void SYSTICK_StartTimeOut(SYSTICK_TIMEOUT* timeout, uint32_t delay_ms) {
	timeout->start = SYSTICK_GetTickCounter();
	timeout->count = (delay_ms*1000U)/SYSTICK_INTERRUPT_PERIOD_IN_US; 
}

void SYSTICK_ResetTimeOut(SYSTICK_TIMEOUT* timeout) {
	timeout->start = SYSTICK_GetTickCounter(); 
}

bool SYSTICK_IsTimeoutReached(SYSTICK_TIMEOUT* timeout) {
	return (SYSTICK_GetTickCounter() - timeout->start) < timeout->count;
}
void SYSTICK_TimerCallbackSet(SYSTICK_CALLBACK callback, uintptr_t context) {
   systick.callback = callback;
   systick.context = context;
}

void SysTick_Handler(void) {
   /* Reading control register clears the count flag */
   uint32_t sysCtrl = SysTick->CTRL;
   systick.tickCounter++;
//   if(systick.callback != NULL) {
//       systick.callback(systick.context);
//   }
   (void)sysCtrl;
}


static const char rstc_syst[] = "System Reset";
static const char rstc_wdt[] = "Watchdog Reset";
static const char rstc_ext[] = "External Reset";
static const char rstc_bovdd[] = "Brownout VDD Reset";
static const char rstc_bovcore[] = "Brownout VCore Reset";
static const char rstc_por[] = "Power On Reset";
static const char rstc_unk[] = "Unknown";

static const char* reset_reasons[] = {
        rstc_por,
        rstc_bovcore,
        rstc_bovdd,
        rstc_unk,
        rstc_ext,
        rstc_wdt,
        rstc_syst,
        rstc_unk
};

const char* RSTC_ResetCauseGetStr(void) {
    union RSTC_Cause x = RSTC_ResetCauseGet();
    for(int i = 0; i < 8; i++) {
        if(x.byte & (1 << i))
            return reset_reasons[i];
    }
    return rstc_unk;
}

union RSTC_Cause RSTC_ResetCauseGet(void) {
    return (union RSTC_Cause)RSTC_REGS->RSTC_RCAUSE;
}