#ifndef PLIB_NVMCTRL_H
#define PLIB_NVMCTRL_H

#include "device.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define NVMCTRL_FLASH_START_ADDRESS        (0x00000000U)
#define NVMCTRL_FLASH_PAGESIZE             (64U)
#define NVMCTRL_FLASH_ROWSIZE              (256U)

/* NVMCTRL invalid commands and/or bad keywords error */
#define    NVMCTRL_ERROR_PROG    ( 0x4U )

/* NVMCTRL lock error */
#define   NVMCTRL_ERROR_LOCK     ( 0x8U )

/* NVMCTRL programming or erase error */
#define    NVMCTRL_ERROR_NVM     ( 0x10U )

typedef uint16_t NVMCTRL_ERROR;

#define NVMCTRL_DATAFLASH_START_ADDRESS    (0x00400000U)
#define NVMCTRL_DATAFLASH_PAGESIZE         (64U)
#define NVMCTRL_DATAFLASH_ROWSIZE          (256U)

#define NVMCTRL_USERROW_START_ADDRESS     (0x00804000U)
#define NVMCTRL_USERROW_SIZE              (0x100U)
#define NVMCTRL_USERROW_PAGESIZE          (64U)

void NVMCTRL_Initialize(void);

bool NVMCTRL_Read(uint32_t *data, uint32_t length, uint32_t address);

bool NVMCTRL_PageWrite(const uint32_t* data, uint32_t address);

bool NVMCTRL_RowErase(uint32_t address);

bool NVMCTRL_DATA_FLASH_Read(uint32_t *data, uint32_t address, uint32_t length);

bool NVMCTRL_DATA_FLASH_PageWrite(const uint32_t* data, uint32_t address);

bool NVMCTRL_DATA_FLASH_RowErase(uint32_t address);

NVMCTRL_ERROR NVMCTRL_ErrorGet(void);

bool NVMCTRL_IsBusy(void);

void NVMCTRL_RegionLock (uint32_t address);

void NVMCTRL_RegionUnlock (uint32_t address);

bool NVMCTRL_PageBufferWrite(const uint32_t *data, uint32_t address);

bool NVMCTRL_PageBufferCommit(uint32_t address);

bool NVMCTRL_USER_ROW_PageWrite(const uint32_t *data, uint32_t address);

bool NVMCTRL_USER_ROW_RowErase( uint32_t address );

void NVMCTRL_CacheInvalidate (void);

uint32_t NVMCTRL_InterruptFlagGet(void);

#ifdef __cplusplus
}
#endif

#endif // PLIB_NVMCTRL_H
