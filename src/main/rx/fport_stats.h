#ifndef D2_FPORT_STATS_H
#define D2_FPORT_STATS_H

#include <stdint.h>

struct packet_stats {
    uint32_t valid_packets;
    uint32_t total_packets;
    uint32_t discarded_bytes;
    uint32_t total_bytes;
    uint32_t num_failsafes;

    uint32_t crc_fail; //todo make these measureable?
    uint32_t eof_fail;
    uint32_t rssi_invalid;
};

#endif //D2_FPORT_STATS_H
