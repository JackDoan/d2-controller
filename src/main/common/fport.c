#include <stdio.h>
#include <string.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"
#include "plib_adc0.h"
#include "l9958.h"
#include "plib_dmac.h"

#define FPORT_START_OF_FRAME 0x7e
#define FPORT_END_OF_FRAME 0x7e
#define FPORT_STUFF_MARK 0x7d
#define FPORT_XOR_VAL 0x20

#define T1_FIRST_ID               0x0400 // -> 0004
#define BATT_ID                   0xF104

enum fport_state {
    FPORT_SEEKING,
    FPORT_FOUND_SOF,
    FPORT_FOUND_1,
    FPORT_FOUND,
};

struct fport_dma_context {
    bool got_bytes;
    bool byte_stuffed;
    uint8_t dma_rx[30];
};

struct packet_stats {
    uint32_t valid_packets;
    uint32_t total_packets;
    uint32_t discarded_bytes;
    uint32_t total_bytes;

    uint32_t crc_fail; //todo make these measureable?
    uint32_t eof_fail;
    uint32_t rssi_invalid;
};

union __attribute__((packed)) fport_response {
    struct __attribute__((packed)) {
        uint8_t len;
        uint8_t uplink;
        uint8_t type;
        uint16_t id;
        uint32_t data;
        uint8_t crc;
    };
    uint8_t bytes[10];
};

static struct fport_dma_context g_context = {0};
static struct packet_stats g_packet_stats = {0};
static uint8_t fport_buf[30] = {0};
static uint8_t fport_idx = 0;

static bool fport_print = false;

static uint8_t discard_buf[64] = {0};
static int discard_idx = 0;

static void discard_byte(uint8_t x) {
    g_packet_stats.discarded_bytes++;
    discard_buf[discard_idx++] = x;
    if((discard_idx % sizeof discard_buf) == 0) {
        discard_idx = 0;
    }
}

uint32_t fport_valid_frame_rate(void) {
    return g_packet_stats.valid_packets * 10000 / g_packet_stats.total_packets;
}

uint32_t fport_valid_byte_rate(void) {
    return (g_packet_stats.total_bytes-g_packet_stats.discarded_bytes) * 10000 / g_packet_stats.total_bytes;
}

static void fport_trigger(size_t len) {
    fport_gets(g_context.dma_rx, len);
}

static bool is_failsafe_pkt(uint8_t flags) {
    return (flags & SBUS_FLAG_FAILSAFE_ACTIVE) || (flags & SBUS_FLAG_SIGNAL_LOSS);
}

static void fport_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    struct fport_dma_context *context = (struct fport_dma_context*)contextHandle;
    switch(event) {
        case DMAC_TRANSFER_EVENT_COMPLETE: {
            uint8_t x = context->dma_rx[0];
            if(x == FPORT_STUFF_MARK) {
                context->byte_stuffed = true;
                fport_trigger(1);
                return;
            }
            else if(context->byte_stuffed) {
                context->dma_rx[0] ^= FPORT_XOR_VAL;
                context->byte_stuffed = false;
            }
            context->got_bytes = true;
            break;
        }
        case DMAC_TRANSFER_EVENT_NONE:
        case DMAC_TRANSFER_EVENT_ERROR:
        default:
            break;
    }
}

void fport_dma_register(void) {
    DMAC_ChannelCallbackRegister(FPORT_DMA_CHANNEL, fport_dma_callback, (uintptr_t) &g_context);
    fport_trigger(1);
}

static uint8_t fport_dma_get_byte(void) {
    g_context.got_bytes = false;
    g_packet_stats.total_bytes++;
    return g_context.dma_rx[0];
}

void fport_enable_printing(bool enable) {
    fport_print = enable;
}

static bool fport_check_telemetry_packet(uint8_t* pkt) {
    g_packet_stats.total_packets++;
    int length = pkt[0]+1;
    uint8_t crc = frskyCheckSum(pkt, length);
    if(crc != pkt[length]) {
        g_packet_stats.crc_fail++;
        return false;
    }
    packet_timer_watchdog_feed();
    g_packet_stats.valid_packets++;
    return true;
}

void print_hex(void* buf, uint8_t* pkt, int len) {
    char* str = (char*)buf;
    for (int i = 0; i < len; i++) {
        sprintf(&(str[(2 * i)]), "%02x", pkt[i]);
    }
    sprintf(&(str[(2 * len)]), "\r\n");
    serial_puts(str);
}

static void fport_tx(const uint8_t* buf) {
    fport_enable_tx(true);
    for (unsigned int j = 0; j < sizeof (union fport_response); j++) {
        uint8_t c = buf[j];
        if (c == FPORT_STUFF_MARK || c == FPORT_START_OF_FRAME) {
            SERCOM_USART_WriteByte(RX, FPORT_STUFF_MARK);
            SERCOM_USART_WriteByte(RX, c ^ FPORT_XOR_VAL);
        } else {
            SERCOM_USART_WriteByte(RX, c);
        }
    }
    fport_enable_tx(false);
}

static void fport_proc_telemetry_req(uint8_t* pkt) {
    //note: very timing sensitive. Don't remove the delay
    static uint8_t idx_to_send = 0;

    if(!fport_check_telemetry_packet(pkt)) {
        return;
    }

//    static char fport_print_buf[64] = {0};
//    if(fport_print) {
//        print_hex(fport_print_buf, pkt, fport_idx);
//    } else {
        SYSTICK_DelayMs(1);
//    }

    union fport_response data = {
        .len = 0x8,
        .uplink = 0x81,
        .type = 0x10,
    };

    idx_to_send = (idx_to_send + 1) % 20;
    if(idx_to_send & 1) {
        return;
    }
    switch(idx_to_send / 2) {
        case 0:
            data.id = 0x6900;
            data.data = ADC0_Convert_mV();
            break;
        case 1:
            data.id = 0x6901;
            data.data = TSENS_Get();
            break;
        case 2:
            data.id = 0x6902;
            data.data = RSTC_ResetCauseGet().byte;
            break;
        case 3:
            data.id = 0x6903;
            data.data = SYSTICK_GetTickCounter();
            break;
        case 4:
            data.id = 0x6911;
            data.data = L9958_Diag_Read(MOTOR1);
            break;
        case 5:
            data.id = 0x6912;
            data.data = L9958_Diag_Read(MOTOR2);
            break;
        case 6:
            data.id = 0x6913;
            data.data = L9958_Diag_Read(MOTOR3);
            break;
        case 7:
            data.id = 0x6914;
            data.data = L9958_Diag_Read(MOTOR4);
            break;
        case 8:
            data.id = 0x6920;
            data.data = fport_valid_frame_rate();
            break;
        case 9:
            data.id = 0x6921;
            data.data = fport_valid_byte_rate();
            break;
        default:
            return;
    }

    data.crc = frskyCheckSum(data.bytes, 8);
    fport_tx(data.bytes);
}

static bool fport_check_control_packet(struct fport_frame *frame) {
    g_packet_stats.total_packets++;

    uint8_t crc = frskyCheckSum((uint8_t*)frame, 28-2);
    if(crc != frame->crc) {
        g_packet_stats.crc_fail++;
        return false;
    }

    if( (frame->rssi == 0) && is_failsafe_pkt(frame->flags)) {
        goto good; //this is a valid packet, even with RSSI at 0
    }

    //todo better sanity checks, filters?
    if(frame->rssi > 100 || frame->rssi < 10) {
        g_packet_stats.rssi_invalid++;
        return false;
    }

    good:
    packet_timer_watchdog_feed();
    g_packet_stats.valid_packets++;
    return true;
}

void fport_proc_packet(uint8_t* pkt) {
    static char fport_print_buf[64] = {0};
    struct fport_frame *frame = (struct fport_frame *)pkt;
    if(!fport_check_control_packet(frame)) {
        return;
    }

    if(is_failsafe_pkt(frame->flags)) {
        failsafe_activate();
        serial_puts("FAILSAFE\r\n");
        return;
    }

    if (frame->flags & SBUS_FLAG_CHANNEL_17) {

    }

    if (frame->flags & SBUS_FLAG_CHANNEL_18) {

    }

    do_brakes(frame->chan4);
    motor_set_speed(MOTOR1, frame->chan0);
    motor_set_speed(MOTOR2, frame->chan1);
    motor_set_speed(MOTOR3, frame->chan2);
    motor_set_speed(MOTOR4, frame->chan3);

    if(fport_print) {
        sprintf(fport_print_buf, "%04lu %04d %04d %04d %04d %04d %04d %02x %03d %02x %lu %lu %lu %lu\r\n",
                sbus_to_duty_cycle(frame->chan0, TCC0_REGS->TCC_PER, &channel_defaults).magnitude,
                frame->chan0,
                frame->chan1,
                frame->chan2,
                frame->chan3,

                frame->chan4,
                frame->chan5,

                frame->flags,
                frame->rssi, frame->crc,
                g_packet_stats.total_packets-g_packet_stats.valid_packets,
                g_packet_stats.total_packets, g_packet_stats.discarded_bytes, g_packet_stats.total_bytes);
        serial_puts(fport_print_buf);
//        print_hex(fport_print_buf, pkt, fport_idx);
    }
}

static enum fport_state state = FPORT_SEEKING;
void fport_tick(void) {
    if(!g_context.got_bytes) {
        return;
    }
    //todo log dropped bytes?
    uint8_t x = fport_dma_get_byte();
    switch(state) {
        case FPORT_SEEKING:
            fport_idx = 0;
            if(x == FPORT_START_OF_FRAME) {
                state = FPORT_FOUND_SOF;
            }
            else {
                discard_byte(x);
            }
            break;
        case FPORT_FOUND_SOF:
            fport_idx = 0;
            if(x == FPORT_START_OF_FRAME) {
                state = FPORT_FOUND_1;
            }
            else if(x == 0x19) { //hmm why does this work
                fport_buf[fport_idx++] = x;
                state = FPORT_FOUND;
            }
            else {
                state = FPORT_SEEKING;
                discard_byte(x);
            }
            break;
        case FPORT_FOUND_1:
            if(x == 0x19 || x == 0x08) {
                fport_buf[fport_idx++] = x;
                state = FPORT_FOUND;
            }
            else if(x == FPORT_START_OF_FRAME) {
                //do nothing
            }
            else {
                fport_idx = 0; //we don't want it
                state = FPORT_SEEKING;
                discard_byte(x);
            }
            break;
        case FPORT_FOUND:
            fport_buf[fport_idx++] = x;
            if(fport_idx >= (fport_buf[0]+2)) {
                switch(fport_buf[1]) { //todo if we're in pkt timeout, stop doing telemetry frames
                    case 0:
                        fport_proc_packet(fport_buf);
                        break;
                    case 1:
                        fport_proc_telemetry_req(fport_buf);
                        break;
                    default:
                        g_packet_stats.eof_fail++;
                        break;
                }
                fport_idx = 0;
                memset(fport_buf, 0, 30);
                state = FPORT_SEEKING;
            }
            break;
        default:
            state = FPORT_SEEKING;
            break;
    }
    fport_trigger(1);
}
