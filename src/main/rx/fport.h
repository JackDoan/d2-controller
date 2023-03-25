#ifndef INAV_FPORT_H
#define INAV_FPORT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "plib_dmac.h"
#include "fport_dma.h"
#include "fport_stats.h"

#define FPORT_START_OF_FRAME 0x7e
#define FPORT_END_OF_FRAME 0x7e

#define FPORT_STUFF_MARK 0x7d
#define FPORT_XOR_VAL 0x20

struct fport_frame {
    uint8_t length;
    uint8_t kind;
    // 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
    unsigned int chan0 : 11;
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
    uint8_t flags;
    uint8_t rssi;
    uint8_t crc;
    uint8_t eof;
} __attribute__((__packed__));

struct __attribute__((packed)) fport_telemetry {
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

#define SBUS_FLAG_CHANNEL_17        (1 << 0)
#define SBUS_FLAG_CHANNEL_18        (1 << 1)
#define SBUS_FLAG_SIGNAL_LOSS       (1 << 2)
#define SBUS_FLAG_FAILSAFE_ACTIVE   (1 << 3)

void fport_tick(void);
void fport_enable_printing(bool enable);
uint32_t fport_valid_frame_rate(void);

#endif //INAV_FPORT_H
