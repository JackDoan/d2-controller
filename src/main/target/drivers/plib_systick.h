#ifndef PLIB_SYSTICK_H
#define PLIB_SYSTICK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus // Provide C++ Compatibility
    extern "C" {
#endif

#define SYSTICK_FREQ   48000000U //todo define in terms of core

#define SYSTICK_INTERRUPT_PERIOD_IN_US  (1000U)

typedef void (*SYSTICK_CALLBACK)(uintptr_t context);

typedef struct {
    uint32_t start; 
    uint32_t count; 
}SYSTICK_TIMEOUT;

typedef struct {
   SYSTICK_CALLBACK          callback;
   uintptr_t                 context;
   volatile uint32_t         tickCounter;
} SYSTICK_OBJECT ;

void SYSTICK_TimerInitialize ( void );
void SYSTICK_TimerRestart ( void );
void SYSTICK_TimerStart ( void );
void SYSTICK_TimerStop ( void );
void SYSTICK_TimerPeriodSet ( uint32_t period );
uint32_t SYSTICK_TimerPeriodGet ( void );
uint32_t SYSTICK_TimerCounterGet ( void );
uint32_t SYSTICK_TimerFrequencyGet ( void );
void SYSTICK_DelayMs ( uint32_t delay_ms );
void SYSTICK_DelayUs ( uint32_t delay_us );

void SYSTICK_TimerCallbackSet ( SYSTICK_CALLBACK callback, uintptr_t context );
uint32_t SYSTICK_GetTickCounter(void);
void SYSTICK_StartTimeOut (SYSTICK_TIMEOUT* timeout, uint32_t delay_ms);
void SYSTICK_ResetTimeOut (SYSTICK_TIMEOUT* timeout);
bool SYSTICK_IsTimeoutReached (SYSTICK_TIMEOUT* timeout);

union RSTC_Cause {
    struct {
        bool system_reset: 1;
        bool wdt: 1;
        bool external: 1;
        bool brown_out_vdd: 1;
        bool brown_out_core: 1;
        bool power_on: 1;
    };
    uint8_t byte;
};

union RSTC_Cause RSTC_ResetCauseGet(void);
const char* RSTC_ResetCauseGetStr(void);
#ifdef __cplusplus // Provide C++ Compatibility
 }
#endif

#endif
