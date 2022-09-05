#ifndef PLIB_SERCOM_USART_COMMON_H
#define PLIB_SERCOM_USART_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
    extern "C" {
#endif
// *****************************************************************************
/* USART Error convenience macros */
// *****************************************************************************
// *****************************************************************************
    /* Error status when no error has occurred */
#define USART_ERROR_NONE 0U

    /* Error status when parity error has occurred */
#define USART_ERROR_PARITY SERCOM_USART_INT_STATUS_PERR_Msk

    /* Error status when framing error has occurred */
#define USART_ERROR_FRAMING SERCOM_USART_INT_STATUS_FERR_Msk

    /* Error status when overrun error has occurred */
#define USART_ERROR_OVERRUN SERCOM_USART_INT_STATUS_BUFOVF_Msk

typedef uint16_t USART_ERROR;

typedef void (*SERCOM_USART_CALLBACK)( uintptr_t context );

typedef struct {
    void * txBuffer;
    size_t txSize;
    volatile size_t txProcessedSize;
    SERCOM_USART_CALLBACK txCallback;
    volatile uintptr_t txContext;
    volatile bool  txBusyStatus;
    void * rxBuffer;
    size_t rxSize;
    volatile size_t rxProcessedSize;
    SERCOM_USART_CALLBACK rxCallback;
    volatile uintptr_t rxContext;
    volatile bool rxBusyStatus;
    volatile USART_ERROR errorStatus;
    volatile uint32_t ctrla_defaults;
    volatile uint32_t ctrlb_defaults;
} SERCOM_USART_OBJECT;


typedef enum
{
    /* Threshold number of bytes are available in the receive ring buffer */
    SERCOM_USART_EVENT_READ_THRESHOLD_REACHED = 0,

    /* Receive ring buffer is full. Application must read the data out to avoid missing data on the next RX interrupt. */
    SERCOM_USART_EVENT_READ_BUFFER_FULL,

    /* USART error. Application must call the SERCOMx_USART_ErrorGet API to get the type of error and clear the error. */
    SERCOM_USART_EVENT_READ_ERROR,

    /* Threshold number of free space is available in the transmit ring buffer */
    SERCOM_USART_EVENT_WRITE_THRESHOLD_REACHED,

    /* Recevie break signal is detected */
    SERCOM_USART_EVENT_BREAK_SIGNAL_DETECTED,
}SERCOM_USART_EVENT;

typedef void (*SERCOM_USART_RING_BUFFER_CALLBACK)(SERCOM_USART_EVENT event, uintptr_t context );

typedef struct
{
    SERCOM_USART_RING_BUFFER_CALLBACK                   wrCallback;
    uintptr_t                                           wrContext;
    volatile uint32_t                                   wrInIndex;
    volatile uint32_t                                   wrOutIndex;
    uint32_t                                            wrBufferSize;
    bool                                                isWrNotificationEnabled;
    uint32_t                                            wrThreshold;
    bool                                                isWrNotifyPersistently;
    SERCOM_USART_RING_BUFFER_CALLBACK                   rdCallback;
    uintptr_t                                           rdContext;
    volatile uint32_t                                   rdInIndex;
    volatile uint32_t                                   rdOutIndex;
    uint32_t                                            rdBufferSize;
    bool                                                isRdNotificationEnabled;
    uint32_t                                            rdThreshold;
    bool                                                isRdNotifyPersistently;
    volatile USART_ERROR                                errorStatus;
} SERCOM_USART_RING_BUFFER_OBJECT;

#ifdef __cplusplus
    }
#endif

#endif //PLIB_SERCOM_USART_COMMON_H
