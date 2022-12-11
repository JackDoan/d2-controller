#ifndef PLIB_ADC0_H
#define PLIB_ADC0_H

#define ADC_VREF (3.3f)

#include "device.h"
#include "plib_adc_common.h"

#ifdef __cplusplus // Provide C Compatibility
    extern "C" {
#endif

void ADC0_Initialize(void);
void ADC0_Enable(void);
void ADC0_Disable(void);
void ADC0_ChannelSelect( ADC_POSINPUT positiveInput, ADC_NEGINPUT negativeInput );
void ADC0_ConversionStart(void);
void ADC0_ComparisonWindowSet(uint16_t low_threshold, uint16_t high_threshold);
void ADC0_WindowModeSet(ADC_WINMODE mode);
bool ADC0_ConversionSequenceIsFinished(void);
void ADC0_InterruptsClear(ADC_STATUS interruptMask);
void ADC0_InterruptsEnable(ADC_STATUS interruptMask);
void ADC0_InterruptsDisable(ADC_STATUS interruptMask);
bool ADC0_ConversionStatusGet(void);
void ADC0_Tick(void);
int ADC0_Get(int idx);

void TSENS_Init(void);
uint32_t TSENS_Get(void);

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif /* PLIB_ADC0_H */
