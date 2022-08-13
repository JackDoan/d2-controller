#include "device.h"
#include "interrupts.h"
#include "plib_systick.h"

static SYSTICK_OBJECT systick;

void SYSTICK_TimerInitialize(void) {
    SysTick->CTRL = 0U;
    SysTick->VAL = 0U;
    SysTick->LOAD = 0xBB80U - 1U;
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;

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

void SYSTICK_StartTimeOut (SYSTICK_TIMEOUT* timeout, uint32_t delay_ms) {
	timeout->start = SYSTICK_GetTickCounter();
	timeout->count = (delay_ms*1000U)/SYSTICK_INTERRUPT_PERIOD_IN_US; 
}

void SYSTICK_ResetTimeOut (SYSTICK_TIMEOUT* timeout) {
	timeout->start = SYSTICK_GetTickCounter(); 
}

bool SYSTICK_IsTimeoutReached (SYSTICK_TIMEOUT* timeout) {
    bool valTimeout  = true;
	if ((SYSTICK_GetTickCounter() - timeout->start) < timeout->count) {
		valTimeout = false;
	}
	
	return valTimeout;
	
}
void SYSTICK_TimerCallbackSet ( SYSTICK_CALLBACK callback, uintptr_t context ) {
   systick.callback = callback;
   systick.context = context;
}

void SysTick_Handler(void) {
   /* Reading control register clears the count flag */
   uint32_t sysCtrl = SysTick->CTRL;
   systick.tickCounter++;
   if(systick.callback != NULL) {
       systick.callback(systick.context);
   }
   (void)sysCtrl;
}
