#ifndef PLIB_DMAC_H    // Guards against multiple inclusion
#define PLIB_DMAC_H

#include <device.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

typedef enum
{
    DMAC_CHANNEL_0 = 0,
    DMAC_CHANNEL_1 = 1,
    DMAC_CHANNEL_2 = 2,
    DMAC_CHANNEL_3 = 3,
} DMAC_CHANNEL;

typedef enum
{
    /* No event */
    DMAC_TRANSFER_EVENT_NONE = 0,

    /* Data was transferred successfully. */
    DMAC_TRANSFER_EVENT_COMPLETE = 1,

    /* Error while processing the request */
    DMAC_TRANSFER_EVENT_ERROR = 2

} DMAC_TRANSFER_EVENT;

typedef enum
{
    /* CRC16 (CRC-CCITT): 0x1021 */
    DMAC_CRC_TYPE_16 = 0x0,

    /* CRC32 (IEEE 802.3): 0x04C11DB7*/
    DMAC_CRC_TYPE_32 = 0x1

} DMAC_CRC_POLYNOMIAL_TYPE;

typedef enum
{
    /* Byte bus access. */
    DMAC_CRC_BEAT_SIZE_BYTE     = 0x0,

    /* Half-word bus access. */
    DMAC_CRC_BEAT_SIZE_HWORD    = 0x1,

    /* Word bus access. */
    DMAC_CRC_BEAT_SIZE_WORD     = 0x2

} DMAC_CRC_BEAT_SIZE;

typedef struct
{
    /* CRCCTRL[CRCPOLY]: Polynomial Type (CRC16, CRC32) */
    DMAC_CRC_POLYNOMIAL_TYPE polynomial_type;

    /* CRCCHKSUM: Initial Seed for calculating the CRC */
    uint32_t seed;
} DMAC_CRC_SETUP;

typedef uint32_t DMAC_CHANNEL_CONFIG;

typedef void (*DMAC_CHANNEL_CALLBACK) (DMAC_TRANSFER_EVENT event, uintptr_t contextHandle);
void DMAC_ChannelCallbackRegister (DMAC_CHANNEL channel, const DMAC_CHANNEL_CALLBACK eventHandler, const uintptr_t contextHandle);
// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
/* The following functions make up the methods (set of possible operations) of
   this interface.
*/
void DMAC_Initialize( void );
bool DMAC_ChannelTransfer (DMAC_CHANNEL channel, const void *srcAddr, const void *destAddr, size_t blockSize);
bool DMAC_ChannelIsBusy ( DMAC_CHANNEL channel );
void DMAC_ChannelDisable ( DMAC_CHANNEL channel );

DMAC_CHANNEL_CONFIG  DMAC_ChannelSettingsGet ( DMAC_CHANNEL channel );
bool  DMAC_ChannelSettingsSet ( DMAC_CHANNEL channel, DMAC_CHANNEL_CONFIG settings );
uint16_t DMAC_ChannelGetTransferredCount( DMAC_CHANNEL channel );

void DMAC_ChannelCRCSetup(DMAC_CHANNEL channel, DMAC_CRC_SETUP CRCSetup);
uint32_t DMAC_CRCRead( void );

uint32_t DMAC_CRCCalculate(void *buffer, uint32_t length, DMAC_CRC_SETUP CRCSetup);

void DMAC_CRCDisable( void );
void DMAC_ChannelSuspend ( DMAC_CHANNEL channel );
void DMAC_ChannelResume ( DMAC_CHANNEL channel );
DMAC_TRANSFER_EVENT DMAC_ChannelTransferStatusGet(DMAC_CHANNEL channel);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //PLIB_DMAC_H
