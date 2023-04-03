#ifndef FPORT_H
#define FPORT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "plib_dmac.h"

#define FPORT_START_OF_FRAME 0x7e
#define FPORT_END_OF_FRAME 0x7e

#define FPORT_STUFF_MARK 0x7d
#define FPORT_XOR_VAL 0x20

struct __attribute__((packed, aligned(1))) fport_frame {
    uint8_t length;
    uint8_t kind;
    // 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int chan9 : 11;
    unsigned int chan10 : 11;
    unsigned int chan11 : 11;
    unsigned int chan12 : 11;
    unsigned int chan13 : 11;
    unsigned int chan14 : 11;
    unsigned int chan15 : 11;
    unsigned int chan16 : 11;
    uint8_t flags;
    uint8_t rssi;
    uint8_t crc;
//    uint8_t eof;
};

struct __attribute__((packed, aligned(1))) fport_telemetry {
    uint8_t len;
    uint8_t uplink;
    uint8_t type;
    uint16_t id;
    uint32_t data;
    uint8_t crc;
};

union fport_pkt {
    struct fport_frame ctrl;
    struct fport_telemetry tele;
    uint8_t bytes[30];
};

struct packet_stats {
    uint32_t valid_packets;
    uint32_t total_packets;
    uint32_t discarded_bytes;
    uint32_t total_bytes;

    uint32_t crc_fail;
    uint32_t eof_fail;
    uint32_t rssi_invalid;
    uint32_t failsafe_active;
    uint32_t signal_loss;
    uint32_t packet_timeouts;
    uint32_t dma_error;
};

#define SBUS_FLAG_CHANNEL_17        (1 << 0)
#define SBUS_FLAG_CHANNEL_18        (1 << 1)
#define SBUS_FLAG_SIGNAL_LOSS       (1 << 2) /* 4 */
#define SBUS_FLAG_FAILSAFE_ACTIVE   (1 << 3) /* 8 */

void fport_tick(uint8_t x);
void fport_enable_printing(bool enable);
void fport_enable_printing_bytes(bool enable);
uint32_t fport_valid_frame_rate(void);

void fport_packet_timeout_hit(void);

void fport_dma_register(void);
void fport_trigger(size_t len);
void fport_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle);
struct packet_stats* fport_get_stats(void);

#endif
