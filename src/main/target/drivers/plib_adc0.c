#include "interrupts.h"
#include "plib_adc0.h"

#define ADC0_LINEARITY_POS  (0U)
#define ADC0_LINEARITY_Msk   (0x7UL << ADC0_LINEARITY_POS)

#define ADC0_BIASCAL_POS  (3U)
#define ADC0_BIASCAL_Msk   (0x7UL << ADC0_BIASCAL_POS)

#define ADC_SCALE 6.06f

static const ADC_POSINPUT adc_inputs[] = {
        ADC_POSINPUT_AIN7, ADC_POSINPUT_AIN0
};

#define NUM_INPUTS sizeof(adc_inputs)/sizeof(ADC_POSINPUT)

static int adc_results[NUM_INPUTS] = {};

static unsigned int adc_idx = 0;

void ADC0_Initialize(void) {
    /* Reset ADC */
    ADC0_REGS->ADC_CTRLA = (uint8_t)ADC_CTRLA_SWRST_Msk;

    while((ADC0_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_SWRST_Msk) == ADC_SYNCBUSY_SWRST_Msk) {
        /* Wait for Synchronization */
    }
    /* Write linearity calibration in BIASREFBUF and bias calibration in BIASCOMP */
    uint32_t calib_low_word = (uint32_t)(*(uint64_t*)OTP5_ADDR);
    ADC0_REGS->ADC_CALIB = (uint16_t)((ADC_CALIB_BIASREFBUF((calib_low_word & ADC0_LINEARITY_Msk) >> ADC0_LINEARITY_POS)) | 
                                      (ADC_CALIB_BIASCOMP((calib_low_word & ADC0_BIASCAL_Msk) >> ADC0_BIASCAL_POS)));

    /* Prescaler */
    ADC0_REGS->ADC_CTRLB = (uint8_t)ADC_CTRLB_PRESCALER_DIV16;
    /* Sampling length */
    ADC0_REGS->ADC_SAMPCTRL = ADC_SAMPCTRL_Msk;//(uint8_t)ADC_SAMPCTRL_SAMPLEN(3UL);

    /* Reference */
    ADC0_REGS->ADC_REFCTRL = (uint8_t)ADC_REFCTRL_REFSEL_INTVCC2 | ADC_REFCTRL_REFCOMP_Msk;

    /* Input pin */
    ADC0_REGS->ADC_INPUTCTRL = (uint16_t) adc_inputs[0];

    /* Resolution & Operation Mode */
    ADC0_REGS->ADC_CTRLC = (uint16_t)(ADC_CTRLC_RESSEL_12BIT | ADC_CTRLC_WINMODE(0UL) );

    /* Clear all interrupt flags */
    ADC0_REGS->ADC_INTFLAG = (uint8_t)ADC_INTFLAG_Msk;

    while(0U != ADC0_REGS->ADC_SYNCBUSY) {}
}

void ADC0_Enable(void) {
    ADC0_REGS->ADC_CTRLA |= (uint8_t)ADC_CTRLA_ENABLE_Msk;
    while(0U != ADC0_REGS->ADC_SYNCBUSY) {}
}


void ADC0_Disable(void) {
    ADC0_REGS->ADC_CTRLA &= (uint8_t)(~ADC_CTRLA_ENABLE_Msk);
    while(0U != ADC0_REGS->ADC_SYNCBUSY) {}
}

/* Configure channel input */
void ADC0_ChannelSelect( ADC_POSINPUT positiveInput, ADC_NEGINPUT negativeInput )
{
    /* Configure pin scan mode and positive and negative input pins */
    ADC0_REGS->ADC_INPUTCTRL = (uint16_t) positiveInput | (uint16_t) negativeInput;
    while((ADC0_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_INPUTCTRL_Msk) == ADC_SYNCBUSY_INPUTCTRL_Msk) {}
}

/* Start the ADC conversion by SW */
void ADC0_ConversionStart(void) {
    adc_idx = (adc_idx+1) % NUM_INPUTS;
    ADC0_ChannelSelect(adc_inputs[adc_idx], ADC_NEGINPUT_GND);
    ADC0_REGS->ADC_SWTRIG |= (uint8_t)ADC_SWTRIG_START_Msk;
    while((ADC0_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_SWTRIG_Msk) == ADC_SYNCBUSY_SWTRIG_Msk) {}
}

/* Check whether auto sequence conversion is done */
bool ADC0_ConversionSequenceIsFinished(void) {
    return (ADC0_REGS->ADC_SEQSTATUS & ADC_SEQSTATUS_SEQBUSY_Msk) != ADC_SEQSTATUS_SEQBUSY_Msk;
}

/* Configure window comparison threshold values */
void ADC0_ComparisonWindowSet(uint16_t low_threshold, uint16_t high_threshold) {
    ADC0_REGS->ADC_WINLT = low_threshold;
    ADC0_REGS->ADC_WINUT = high_threshold;
    while(0U != (ADC0_REGS->ADC_SYNCBUSY)) { }
}

void ADC0_WindowModeSet(ADC_WINMODE mode) {
	ADC0_REGS->ADC_CTRLC =  (ADC0_REGS->ADC_CTRLC & (uint16_t)(~ADC_CTRLC_WINMODE_Msk)) | (uint16_t)((uint32_t)mode << ADC_CTRLC_WINMODE_Pos);
    while(0U != (ADC0_REGS->ADC_SYNCBUSY)) { }
}

void ADC0_InterruptsClear(ADC_STATUS interruptMask) {
    ADC0_REGS->ADC_INTFLAG = (uint8_t)interruptMask;
}

void ADC0_InterruptsEnable(ADC_STATUS interruptMask) {
    ADC0_REGS->ADC_INTENSET = (uint8_t)interruptMask;
}

void ADC0_InterruptsDisable(ADC_STATUS interruptMask) {
    ADC0_REGS->ADC_INTENCLR = (uint8_t)interruptMask;
}

/* Check whether result is ready */
bool ADC0_ConversionStatusGet(void) {
    bool status =  (((ADC0_REGS->ADC_INTFLAG & ADC_INTFLAG_RESRDY_Msk) >> ADC_INTFLAG_RESRDY_Pos) != 0U);
    if (status) {
        ADC0_REGS->ADC_INTFLAG = (uint8_t)ADC_INTFLAG_RESRDY_Msk;
    }
    return status;
}

void ADC0_Tick(void) {
    unsigned int current_idx = adc_idx;
    if(ADC0_ConversionStatusGet()) {
        uint16_t adc_count = ADC0_REGS->ADC_RESULT;
        ADC0_ConversionStart();
        float input_voltage = (float)adc_count * ADC_SCALE * ADC_VREF / (41) /*was 4096, but I want millivolts */;
        adc_results[current_idx] = ((int)input_voltage+ adc_results[current_idx])/2;
    }
}

int ADC0_Get(int idx) {
    return adc_results[idx];
}

__PACKED_STRUCT tsens_cal{
    uint16_t tcal:6;
    uint16_t fcal:6;
    uint32_t gain:24;
    uint32_t offset:24;
    uint8_t rsvd:4;
};

void TSENS_Init(void) {
    TSENS_REGS->TSENS_CTRLA = TSENS_CTRLA_SWRST_Msk;
    while(0U != (TSENS_REGS->TSENS_SYNCBUSY)) { }
    TSENS_REGS->TSENS_DBGCTRL = TSENS_DBGCTRL_Msk;
    TSENS_REGS->TSENS_CTRLC |= TSENS_CTRLC_FREERUN_Msk;

    struct tsens_cal* cal = (struct tsens_cal*)TEMP_LOG_ADDR;

    TSENS_REGS->TSENS_CAL = (cal->tcal << 8) | cal->fcal;
    TSENS_REGS->TSENS_GAIN = cal->gain;
    TSENS_REGS->TSENS_OFFSET = cal->offset;

    TSENS_REGS->TSENS_CTRLA |= TSENS_CTRLA_ENABLE_Msk;
    TSENS_REGS->TSENS_CTRLB |= TSENS_CTRLB_START_Msk;
}

uint32_t TSENS_Get(void) {
    return TSENS_REGS->TSENS_VALUE;
}
