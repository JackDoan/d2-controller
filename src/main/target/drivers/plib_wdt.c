#include "interrupts.h"
#include "plib_wdt.h"

bool WDT_IsEnabled(void) {
    return ((WDT_REGS->WDT_CTRLA & (WDT_CTRLA_ALWAYSON_Msk | WDT_CTRLA_ENABLE_Msk)) != 0U);
}

bool WDT_IsAlwaysOn(void) {
    return ((WDT_REGS->WDT_CTRLA & WDT_CTRLA_ALWAYSON_Msk) != 0U);
}

void WDT_Enable(void) {
    /* Checking if Always On Bit is Enabled */
    if(WDT_IsAlwaysOn()) {
        return;
    }
    /* Enable Watchdog Timer */
    WDT_REGS->WDT_CTRLA |= (uint8_t)WDT_CTRLA_ENABLE_Msk | WDT_CTRLA_ALWAYSON_Msk;
    WDT_REGS->WDT_CONFIG |= WDT_CONFIG_PER_CYC8;
    while(WDT_REGS->WDT_SYNCBUSY != 0U) {}
}

/* This function is used to disable the Watchdog Timer */
void WDT_Disable(void) {
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
    WDT_REGS->WDT_CTRLA &= (uint8_t)(~WDT_CTRLA_ENABLE_Msk);
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
}

void WDT_EnableWindowMode(void) {
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
    /* Window mode can be changed only if peripheral is disabled or ALWAYS ON bit is set */
    if(((WDT_REGS->WDT_CTRLA & WDT_CTRLA_ENABLE_Msk) == 0U) || ((WDT_REGS->WDT_CTRLA & WDT_CTRLA_ALWAYSON_Msk) != 0U)) {
        /* Enable window mode */
        WDT_REGS->WDT_CTRLA |= (uint8_t)WDT_CTRLA_WEN_Msk;
    }
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
}

void WDT_DisableWindowMode(void) {
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
    /* Window mode can be changed only if peripheral is disabled or ALWAYS ON bit is set */
    if(((WDT_REGS->WDT_CTRLA & WDT_CTRLA_ENABLE_Msk) == 0U) || ((WDT_REGS->WDT_CTRLA & WDT_CTRLA_ALWAYSON_Msk) != 0U)) {
        /* Disable window mode */
        WDT_REGS->WDT_CTRLA &= (uint8_t)(~WDT_CTRLA_WEN_Msk);
    }
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
}

void WDT_TimeoutPeriodSet(uint8_t TimeoutPeriod) {
    /* Set WDT timeout period */
    WDT_REGS->WDT_CONFIG = (WDT_REGS->WDT_CONFIG & ~WDT_CONFIG_PER_Msk) | (TimeoutPeriod & WDT_CONFIG_PER_Msk);
}

/* If application intends to stay in active mode after clearing WDT, then use WDT_Clear API to clear the WDT. This avoids CPU from waiting or stalling for Synchronization.
 * If application intends to enter low power mode after clearing WDT, then use the WDT_ClearWithSync API to clear the WDT.
 */
void WDT_Clear(void) {
    if ((WDT_REGS->WDT_SYNCBUSY & WDT_SYNCBUSY_CLEAR_Msk) != WDT_SYNCBUSY_CLEAR_Msk) {
        WDT_REGS->WDT_CLEAR = (uint8_t)WDT_CLEAR_CLEAR_KEY;
    }
}

/* This API must be used if application intends to enter low power mode after clearing WDT.
 * It waits for write synchronization to complete as the device must not enter low power mode
 * while write sync is in progress.
 */
void WDT_ClearWithSync(void) {
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
    /* Clear WDT and reset the WDT timer before the timeout occurs */
    WDT_REGS->WDT_CLEAR = (uint8_t)WDT_CLEAR_CLEAR_KEY;
    while(WDT_REGS->WDT_SYNCBUSY != 0U) { }
}

