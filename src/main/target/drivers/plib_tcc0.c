#include "interrupts.h"
#include "plib_tcc0.h"


/* Object to hold callback function and context */
static TCC_CALLBACK_OBJECT TCC0_CallbackObj;

static inline void TCC0_Sync(uint32_t mask) {
    while ((TCC0_REGS->TCC_SYNCBUSY & mask) != 0U)
    {
        /* Wait for sync */
    }
}

/* Initialize TCC module */
void TCC0_PWMInitialize(void)
{
    /* Reset TCC */
    TCC0_REGS->TCC_CTRLA = TCC_CTRLA_SWRST_Msk;
    TCC0_Sync(TCC_SYNCBUSY_SWRST_Msk);

    /* Clock prescaler */
    TCC0_REGS->TCC_CTRLA = TCC_CTRLA_PRESCALER_DIV1 ;
    TCC0_REGS->TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0UL);
    /* Dead time configurations */
    TCC0_REGS->TCC_WEXCTRL |=
            TCC_WEXCTRL_DTIEN0_Msk |
            TCC_WEXCTRL_DTIEN1_Msk |
            TCC_WEXCTRL_DTIEN2_Msk |
            TCC_WEXCTRL_DTIEN3_Msk |
            TCC_WEXCTRL_DTLS(64UL) |
            TCC_WEXCTRL_DTHS(64UL);

    TCC0_REGS->TCC_WAVE = TCC_WAVE_WAVEGEN_DSTOP;

    /* Configure duty cycle values */
    TCC0_REGS->TCC_CC[0] = 1000U; //todo default these to 0
    TCC0_REGS->TCC_CC[1] = 1000U;
    TCC0_REGS->TCC_CC[2] = 1000U;
    TCC0_REGS->TCC_CC[3] = 1000U;
    TCC0_REGS->TCC_PER = 2047U;  //hmm making this the same as sbus seems... handy

    TCC0_REGS->TCC_INTENSET = TCC_INTENSET_OVF_Msk;

    TCC0_Sync(0xffffffff);
}


/* Start the PWM generation */
void TCC0_PWMStart(void)
{
    TCC0_REGS->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    TCC0_Sync(TCC_SYNCBUSY_ENABLE_Msk);
}

/* Stop the PWM generation */
void TCC0_PWMStop (void)
{
    TCC0_REGS->TCC_CTRLA &= ~TCC_CTRLA_ENABLE_Msk;
    TCC0_Sync(TCC_SYNCBUSY_ENABLE_Msk);
}

void TCC0_PWM_Enable(bool enable)
{
    if(enable)
        TCC0_REGS->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    else
        TCC0_REGS->TCC_CTRLA &= ~TCC_CTRLA_ENABLE_Msk;
    TCC0_Sync(TCC_SYNCBUSY_ENABLE_Msk);
}

/* Configure PWM period */
bool TCC0_PWM24bitPeriodSet (uint32_t period)
{
    bool status = false;
    if ((TCC0_REGS->TCC_STATUS & (TCC_STATUS_PERBUFV_Msk)) == 0U)
    {
        TCC0_REGS->TCC_PERBUF = period & 0xFFFFFFU;
        status = true;
    }    
    return status;
}


/* Read TCC period */
uint32_t TCC0_PWM24bitPeriodGet (void)
{
    TCC0_Sync(TCC_SYNCBUSY_PER_Msk);
    return (TCC0_REGS->TCC_PER & 0xFFFFFFU);
}

/* Configure dead time */
void TCC0_PWMDeadTimeSet (uint8_t deadtime_high, uint8_t deadtime_low)
{
    TCC0_REGS->TCC_WEXCTRL &= ~(TCC_WEXCTRL_DTHS_Msk | TCC_WEXCTRL_DTLS_Msk);
    TCC0_REGS->TCC_WEXCTRL |= TCC_WEXCTRL_DTHS((uint32_t)deadtime_high) | TCC_WEXCTRL_DTLS((uint32_t)deadtime_low);
}

bool TCC0_PWMPatternSet(uint8_t pattern_enable, uint8_t pattern_output)
{
    bool status = false;
    if ((TCC0_REGS->TCC_STATUS & (TCC_STATUS_PATTBUFV_Msk)) == 0U)
    {
        TCC0_REGS->TCC_PATTBUF = (uint16_t)(pattern_enable | ((uint32_t)pattern_output << 8U));
        status = true;
    }   
    return status; 
}


/* Set the counter*/
void TCC0_PWM24bitCounterSet (uint32_t count)
{
    TCC0_REGS->TCC_COUNT = count & 0xFFFFFFU;
    while ((TCC0_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_COUNT_Msk) != 0U)
    {
        /* Wait for sync */
    }
}

/* Enable forced synchronous update */
void TCC0_PWMForceUpdate(void)
{
    TCC0_REGS->TCC_CTRLBSET |= (uint8_t)TCC_CTRLBCLR_CMD_UPDATE;
    while ((TCC0_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_CTRLB_Msk) != 0U)
    {
        /* Wait for sync */
    }
}

/* Enable the period interrupt - overflow or underflow interrupt */
void TCC0_PWMPeriodInterruptEnable(void)
{
    TCC0_REGS->TCC_INTENSET = TCC_INTENSET_OVF_Msk;
}

/* Disable the period interrupt - overflow or underflow interrupt */
void TCC0_PWMPeriodInterruptDisable(void)
{
    TCC0_REGS->TCC_INTENCLR = TCC_INTENCLR_OVF_Msk;
}

 /* Register callback function */
void TCC0_PWMCallbackRegister(TCC_CALLBACK callback, uintptr_t context)
{
    TCC0_CallbackObj.callback_fn = callback;
    TCC0_CallbackObj.context = context;
}

  
/* Interrupt Handler */
void TCC0_InterruptHandler(void)
{
    uint32_t status;
    status = TCC0_REGS->TCC_INTFLAG;
    /* Clear interrupt flags */
    TCC0_REGS->TCC_INTFLAG = TCC_INTFLAG_Msk;
    (void)TCC0_REGS->TCC_INTFLAG;
    if (TCC0_CallbackObj.callback_fn != NULL)
    {
        TCC0_CallbackObj.callback_fn(status, TCC0_CallbackObj.context);
    }

}
