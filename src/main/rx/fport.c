#include <string.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"
#include "plib_adc0.h"
#include "l9958.h"
#include "helpers.h"

#define T1_FIRST_ID               0x0400 // -> 0004
#define BATT_ID                   0xF104
enum fport_state {
    FPORT_SEEKING,
    FPORT_SOF,
    FPORT_FOUND,
};

static struct packet_stats g_packet_stats = {0};

static union fport_pkt fport_buf = {0};
static uint8_t fport_idx = 0;

static bool fport_print = false;

uint32_t fport_valid_frame_rate(void) {
    return g_packet_stats.valid_packets * 10000 / g_packet_stats.total_packets;
}

uint32_t fport_valid_byte_rate(void) {
    uint32_t total_scaled = g_packet_stats.total_bytes >> 4U;
    uint32_t disc_scaled = g_packet_stats.discarded_bytes >> 4U;
    return (total_scaled-disc_scaled) * 1000 / total_scaled;
}

static bool is_failsafe_pkt(uint8_t flags) {
    return (flags & SBUS_FLAG_FAILSAFE_ACTIVE) || (flags & SBUS_FLAG_SIGNAL_LOSS);
}

void fport_enable_printing(bool enable) {
    fport_print = enable;
}

static bool fport_check_telemetry_packet(union fport_pkt* pkt) {
    g_packet_stats.total_packets++;
    int length = pkt->ctrl.length + 1;
    uint8_t crc = frskyCheckSum(pkt->bytes, length);
    if(crc != pkt->bytes[length]) {
        g_packet_stats.crc_fail++;
        return false;
    }
    packet_timer_watchdog_feed();
    g_packet_stats.valid_packets++;
    return true;
}

static void fport_tx(const uint8_t* buf) {
    fport_enable_tx(true);
    for (unsigned int i = 0; i < sizeof (struct fport_telemetry); i++) {
        uint8_t c = buf[i];
        if (c == FPORT_STUFF_MARK || c == FPORT_START_OF_FRAME) {
            SERCOM_USART_WriteByte(RX, FPORT_STUFF_MARK);
            SERCOM_USART_WriteByte(RX, c ^ FPORT_XOR_VAL);
        } else {
            SERCOM_USART_WriteByte(RX, c);
        }
    }
    fport_enable_tx(false);
}

static void fport_proc_telemetry_req(union fport_pkt *pkt) {
    //note: very timing sensitive. Don't remove the delay
    static uint8_t idx_to_send = 0;

    if(!fport_check_telemetry_packet(pkt)) {
        return;
    }

    SYSTICK_DelayUs(800);  //this runs in the ISR context bc it's part of the DMA callback, I am very sorry

    union fport_pkt data = {
        .tele.len = 0x8,
        .tele.uplink = 0x81,
        .tele.type = 0x10,
    };

    idx_to_send = (idx_to_send + 1) % 22;
    if(idx_to_send & 1) {
        return;
    }
    switch(idx_to_send / 2) {
        case 0:
            data.tele.id = 0x6900;
            data.tele.data = ADC0_Convert_mV();
            break;
        case 1:
            data.tele.id = 0x6901;
            data.tele.data = TSENS_Get();
            break;
        case 2:
            data.tele.id = 0x6902;
            data.tele.data = RSTC_ResetCauseGet().byte;
            break;
        case 3:
            data.tele.id = 0x6903;
            data.tele.data = SYSTICK_GetTickCounter();
            break;
        case 4:
            data.tele.id = 0x6911;
            data.tele.data = L9958_Diag_Read(MOTOR1);
            break;
        case 5:
            data.tele.id = 0x6912;
            data.tele.data = L9958_Diag_Read(MOTOR2);
            break;
        case 6:
            data.tele.id = 0x6913;
            data.tele.data = L9958_Diag_Read(MOTOR3);
            break;
        case 7:
            data.tele.id = 0x6914;
            data.tele.data = L9958_Diag_Read(MOTOR4);
            break;
        case 8:
            data.tele.id = 0x6915;
            data.tele.data = L9958_has_problems();
            break;
        case 9:
            data.tele.id = 0x6920;
            data.tele.data = fport_valid_frame_rate();
            break;
        case 10:
            data.tele.id = 0x6921;
            data.tele.data = fport_valid_byte_rate();
            break;
        default:
            return;
    }

    data.tele.crc = frskyCheckSum(data.bytes, 8);
    fport_tx(data.bytes);
}

static bool fport_check_control_packet(struct fport_frame *frame) {
    g_packet_stats.total_packets++;

    uint8_t crc = frskyCheckSum((uint8_t*)frame, 28-2);
    if(crc != frame->crc) {g_packet_stats.crc_fail++;
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

void fport_proc_packet(union fport_pkt* pkt) {
//    static char fport_print_buf[64] = {0};
    struct fport_frame frame = pkt->ctrl;
    if(!fport_check_control_packet(&frame)) {
        return;
    }

    if(is_failsafe_pkt(frame.flags)) {
        failsafe_activate();
        g_packet_stats.num_failsafes++;
        return;
    }

    if (frame.flags & SBUS_FLAG_CHANNEL_17) {

    }

    if (frame.flags & SBUS_FLAG_CHANNEL_18) {

    }

    do_brakes(frame.chan4);
    motor_set_speed(MOTOR1, frame.chan0);
    motor_set_speed(MOTOR2, frame.chan1);
    motor_set_speed(MOTOR3, frame.chan2);
    motor_set_speed(MOTOR4, frame.chan3);

    if(fport_print) {
//        sprintf(fport_print_buf, "%04lu %04d %04d %04d %04d %04d %04d %02x %03d %02x %lu %lu %lu %lu\r\n",
//                sbus_to_duty_cycle(frame.chan0, TCC0_REGS->TCC_PER, &drive_sbus_params).magnitude,
//                frame.chan0,
//                frame.chan1,
//                frame.chan2,
//                frame.chan3,
//
//                frame.chan4,
//                frame.chan5,
//
//                frame.flags,
//                frame.rssi, frame.crc,
//                g_packet_stats.total_packets-g_packet_stats.valid_packets,
//                g_packet_stats.total_packets, g_packet_stats.discarded_bytes, g_packet_stats.total_bytes);
//        serial_puts(fport_print_buf);
//        print_hex(fport_print_buf, pkt, fport_idx);
    }
}

static enum fport_state state = FPORT_SEEKING;
void fport_tick(void) {
    uint8_t x = fport_dma_get_byte();
    g_packet_stats.total_bytes++;
    switch(state) {
        case FPORT_SEEKING:
            fport_idx = 0;
            if(x == FPORT_START_OF_FRAME) {
                state = FPORT_SOF;
            }
            else {
                g_packet_stats.discarded_bytes++;
            }
            break;
        case FPORT_SOF:
            if(x == 0x19 || x == 0x08) {
                fport_buf.bytes[fport_idx++] = x;
                state = FPORT_FOUND;
            }
            else if(x == FPORT_START_OF_FRAME) {
                //do nothing
            }
            else {
                fport_idx = 0; //we don't want it
                state = FPORT_SEEKING;
                g_packet_stats.discarded_bytes++;
            }
            break;
        case FPORT_FOUND:
            fport_buf.bytes[fport_idx++] = x;
            if(fport_idx >= (fport_buf.ctrl.length + 2)) {
                switch(fport_buf.ctrl.kind) { //todo if we're in pkt timeout, stop doing telemetry frames
                    case 0:
                        fport_proc_packet(&fport_buf);
                        break;
                    case 1:
                        fport_proc_telemetry_req(&fport_buf);
                        break;
                    default:
                        g_packet_stats.eof_fail++;
                        break;
                }
                fport_idx = 0;
                memset(fport_buf.bytes, 0, 30);
                state = FPORT_SEEKING;
            }
            break;
        default:
            state = FPORT_SEEKING;
            break;
    }
    fport_trigger(1);
}
