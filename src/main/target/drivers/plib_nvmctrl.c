#include <string.h>
#include "plib_nvmctrl.h"
#include "interrupts.h"

void NVMCTRL_Initialize(void) {
    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_CACHEDIS_CACHE_DF_DIS_MAIN_EN |
            NVMCTRL_CTRLB_READMODE_NO_MISS_PENALTY |
            NVMCTRL_CTRLB_SLEEPPRM_WAKEONACCESS |
            NVMCTRL_CTRLB_RWS(2U) |
            NVMCTRL_CTRLB_MANW_Msk;
}

void NVMCTRL_CacheInvalidate(void) {
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_INVALL | NVMCTRL_CTRLA_CMDEX_KEY;
}

bool NVMCTRL_DATA_FLASH_Read( uint32_t *data, uint32_t length, const uint32_t address ) {
    (void) memcpy(data, (uint32_t *)address, length);
    return true;
}

bool NVMCTRL_DATA_FLASH_PageWrite(const uint32_t *data, const uint32_t address) {
    uint32_t * paddress = (uint32_t *)address;

    /* Writing 32-bit words in the given address */
    for (uint32_t i = 0U; i < (NVMCTRL_DATAFLASH_PAGESIZE/4U); i++) {
        *paddress = data[i];
         paddress++;
    }

    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_DFWP | NVMCTRL_CTRLA_CMDEX_KEY;
    return true;
}

bool NVMCTRL_DATA_FLASH_RowErase(uint32_t address) {
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_DFER | NVMCTRL_CTRLA_CMDEX_KEY;
    return true;
}

bool NVMCTRL_Read(uint32_t *data, uint32_t length, const uint32_t address) {
    (void) memcpy(data, (uint32_t *)address, length);
    return true;
}

bool NVMCTRL_PageBufferWrite(const uint32_t *data, const uint32_t address) {
    uint32_t * paddress = (uint32_t *)address;

    /* writing 32-bit data into the given address */
    for (uint32_t i = 0U; i < (NVMCTRL_FLASH_PAGESIZE/4U); i++)
    {
        *paddress = data[i];
         paddress++;
    }

    return true;
}

bool NVMCTRL_PageBufferCommit(const uint32_t address) {
    uint16_t command = NVMCTRL_CTRLA_CMD_WP_Val;

    /* Set address and command */
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;

    if (address >= NVMCTRL_DATAFLASH_START_ADDRESS) {
        command = NVMCTRL_CTRLA_CMD_DFWP;
    }

    NVMCTRL_REGS->NVMCTRL_CTRLA = (uint16_t)(command | NVMCTRL_CTRLA_CMDEX_KEY);

    return true;
}

bool NVMCTRL_PageWrite(const uint32_t *data, const uint32_t address ) {
    uint32_t * paddress = (uint32_t *)address;

    /* writing 32-bit data into the given address */
    for (uint32_t i = 0U; i < (NVMCTRL_FLASH_PAGESIZE/4U); i++)
    {
        *paddress = data[i];
         paddress++;
    }

    /* Set address and command */
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_WP_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    return true;
}

bool NVMCTRL_RowErase( uint32_t address ) {
    /* Set address and command */
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_ER_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    return true;
}

bool NVMCTRL_USER_ROW_PageWrite(const uint32_t *data, const uint32_t address ) {
    uint32_t * paddress = (uint32_t *)address;
    if (!(
            (address >= NVMCTRL_USERROW_START_ADDRESS) && 
            (address <= ((NVMCTRL_USERROW_START_ADDRESS + NVMCTRL_USERROW_SIZE) - NVMCTRL_USERROW_PAGESIZE))
    )) {
        return false;
    }
    /* writing 32-bit data into the given address */
    for (uint32_t i = 0U; i < (NVMCTRL_USERROW_PAGESIZE/4U); i++) {
        *paddress = data[i];
        paddress++;
    }

    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_WAP_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    return true;
}

bool NVMCTRL_USER_ROW_RowErase( uint32_t address ) {
    if (!(
            (address >= NVMCTRL_USERROW_START_ADDRESS) && 
            (address <= (NVMCTRL_USERROW_START_ADDRESS + NVMCTRL_USERROW_SIZE))
        )) {
        return false;
    }

    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_EAR_Val | NVMCTRL_CTRLA_CMDEX_KEY;
    return true;
}

NVMCTRL_ERROR NVMCTRL_ErrorGet(void) {
    volatile uint16_t nvm_error = (
            NVMCTRL_REGS->NVMCTRL_STATUS &
                ((uint8_t)NVMCTRL_STATUS_NVME_Msk |
                NVMCTRL_STATUS_LOCKE_Msk |
                NVMCTRL_STATUS_PROGE_Msk)
    );

    /* Clear the error bits in both STATUS and INTFLAG register */
    NVMCTRL_REGS->NVMCTRL_STATUS |= nvm_error;
    NVMCTRL_REGS->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_ERROR_Msk;

    return nvm_error;
}

bool NVMCTRL_IsBusy(void) {
    return (NVMCTRL_REGS->NVMCTRL_INTFLAG & NVMCTRL_INTFLAG_READY_Msk) == 0U;
}

void NVMCTRL_RegionLock(uint32_t address) {
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_LR_Val | NVMCTRL_CTRLA_CMDEX_KEY;
}

void NVMCTRL_RegionUnlock(uint32_t address) {
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = NVMCTRL_CTRLA_CMD_UR_Val | NVMCTRL_CTRLA_CMDEX_KEY;
}

uint32_t NVMCTRL_InterruptFlagGet(void) {
    uint32_t intFlag =  NVMCTRL_REGS->NVMCTRL_INTFLAG & NVMCTRL_INTFLAG_Msk;
    /* Clear interrupt falg */
    NVMCTRL_REGS->NVMCTRL_INTFLAG = intFlag;
    return intFlag;
}
