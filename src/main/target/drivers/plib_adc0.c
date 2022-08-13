#include "interrupts.h"
#include "plib_adc0.h"

#define ADC0_LINEARITY_POS  (0U)
#define ADC0_LINEARITY_Msk   (0x7UL << ADC0_LINEARITY_POS)

#define ADC0_BIASCAL_POS  (3U)
#define ADC0_BIASCAL_Msk   (0x7UL << ADC0_BIASCAL_POS)

#define ADC_SCALE 6.06f

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
    ADC0_REGS->ADC_CTRLB = (uint8_t)ADC_CTRLB_PRESCALER_DIV8;
    /* Sampling length */
    ADC0_REGS->ADC_SAMPCTRL = (uint8_t)ADC_SAMPCTRL_SAMPLEN(3UL);

    /* Reference */
    ADC0_REGS->ADC_REFCTRL = (uint8_t)ADC_REFCTRL_REFSEL_INTVCC2;

    /* Input pin */
    ADC0_REGS->ADC_INPUTCTRL = (uint16_t) ADC_POSINPUT_AIN2;

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
    ADC0_REGS->ADC_SWTRIG |= (uint8_t)ADC_SWTRIG_START_Msk;
    while((ADC0_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_SWTRIG_Msk) == ADC_SYNCBUSY_SWTRIG_Msk) {}
}

/* Check whether auto sequence conversion is done */
bool ADC0_ConversionSequenceIsFinished(void) {
    return (ADC0_REGS->ADC_SEQSTATUS & ADC_SEQSTATUS_SEQBUSY_Msk) != ADC_SEQSTATUS_SEQBUSY_Msk;
}

/* Configure window comparison threshold values */
void ADC0_ComparisonWindowSet(uint16_t low_threshold, uint16_t high_threshold)
{
    ADC0_REGS->ADC_WINLT = low_threshold;
    ADC0_REGS->ADC_WINUT = high_threshold;
    while(0U != (ADC0_REGS->ADC_SYNCBUSY))
    {
        /* Wait for Synchronization */
    }
}

void ADC0_WindowModeSet(ADC_WINMODE mode)
{
	ADC0_REGS->ADC_CTRLC =  (ADC0_REGS->ADC_CTRLC & (uint16_t)(~ADC_CTRLC_WINMODE_Msk)) | (uint16_t)((uint32_t)mode << ADC_CTRLC_WINMODE_Pos);
    while(0U != (ADC0_REGS->ADC_SYNCBUSY))
    {
        /* Wait for Synchronization */
    }
}

/* Read the conversion result */
uint16_t ADC0_ConversionResultGet(void) {
    return (uint16_t)ADC0_REGS->ADC_RESULT;
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

float ADC0_Convert_mV(void) {
    ADC0_ConversionStart();
    while(!ADC0_ConversionStatusGet()) {};

    uint16_t adc_count = ADC0_ConversionResultGet();
    float input_voltage = (float)adc_count * ADC_SCALE * ADC_VREF / (41) /*was 4096, but I want millivolts */;
    return input_voltage;
}
