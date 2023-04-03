#include <string.h>
#include <stdio.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"
#include "plib_adc0.h"
#include "l9958.h"
#include "helpers.h"
#include "fport_helpers.h"

#define T1_FIRST_ID               0x0400 // -> 0004
#define BATT_ID                   0xF104
enum fport_state {
    FPORT_SEEKING,
    FPORT_SOF,
    FPORT_FOUND,
};

static struct packet_stats g_packet_stats = {0};

struct packet_stats* fport_get_stats(void) {
    return &g_packet_stats;
}

void fport_packet_timeout_hit(void) {
    g_packet_stats.packet_timeouts++;
}

static union fport_pkt fport_buf = {0};
static uint8_t fport_idx = 0;
static bool fport_cal_mode = false;
static bool fport_print = false;
static enum fport_state state = FPORT_SOF;
struct fport_dma_context {
    bool byte_stuffed;
    uint8_t dma_rx[30];
    bool print_bytes;
};

static struct fport_dma_context g_context = {0};


uint32_t fport_valid_frame_rate(void) {
    return g_packet_stats.valid_packets * 10000 / g_packet_stats.total_packets;
}

uint32_t fport_valid_byte_rate(void) {
    uint32_t total_scaled = g_packet_stats.total_bytes >> 4U;
    uint32_t disc_scaled = g_packet_stats.discarded_bytes >> 4U;
    return (total_scaled-disc_scaled) * 1000 / total_scaled;
}

static bool is_failsafe_pkt(uint8_t flags) {
    if (flags & SBUS_FLAG_FAILSAFE_ACTIVE) {
        g_packet_stats.failsafe_active++;
        return true;
    }
    if (flags & SBUS_FLAG_SIGNAL_LOSS) {
        g_packet_stats.signal_loss++;
//        return true;
    }
    return false;
}

void fport_enable_printing(bool enable) {
    fport_print = enable;
}

void fport_enable_printing_bytes(bool enable) {
    g_context.print_bytes = enable;
}

static bool fport_check_telemetry_packet(union fport_pkt* pkt) {
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


static void fport_proc_telemetry_req(union fport_pkt *pkt) {
    //note: very timing sensitive. Don't remove the delay
    static uint8_t idx_to_send = 0;
    if(!fport_check_telemetry_packet(pkt)) {
        return;
    }
    SYSTICK_DelayUs(1000);  //this runs in the ISR context bc it's part of the DMA callback, I am very sorry

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
            data.tele.data = L9958_has_problems() + (fport_cal_mode*10000);
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
    fport_puts(data.bytes);
}

static bool fport_check_control_packet(union fport_pkt* pkt) {
    struct fport_frame *frame = &pkt->ctrl;

    uint8_t crc = frskyCheckSum((uint8_t*)frame, 28-2);
    if(crc != frame->crc) {
        g_packet_stats.crc_fail++;
        return false;
    }

    if((frame->rssi == 0) && is_failsafe_pkt(frame->flags)) { //todo consider removing rssi==0 check
        goto good; //this is a valid packet, even with RSSI at 0
    }

    //todo better sanity checks, filters?
    if(frame->rssi > 100) {
        g_packet_stats.rssi_invalid++;
        return false;
    }

    good:
    packet_timer_watchdog_feed();
    g_packet_stats.valid_packets++;
    return true;
}

void fport_proc_packet(union fport_pkt* pkt) {
    if(!fport_check_control_packet(pkt)) {
        return;
    }

    if(is_failsafe_pkt(pkt->ctrl.flags)) {
        failsafe_activate();
        return;
    }
//    bool chan17 = pkt->ctrl.flags & SBUS_FLAG_CHANNEL_17;
//    bool chan18 = pkt->ctrl.flags & SBUS_FLAG_CHANNEL_18;
    bool cal_switch = (pkt->ctrl.chan16 < 200) && (pkt->ctrl.chan16 > 50);
    if(cal_switch && (SYSTICK_GetTickCounter() < 5000)) {
        if(!fport_cal_mode) {
            for (enum motor_channel chan = 0; chan < MOTOR_COUNT; chan++) {
                struct motor_t *motor = get_motor(chan);
                motor->sbus_config.max = 1000;
                motor->sbus_config.mid = 1000;
                motor->sbus_config.min = 1000;
                motor->sbus_config.deadband = 3;
            }
        }
        fport_cal_mode = true;
        motors_set_enable(false);
    } else if (fport_cal_mode && !cal_switch) {
        motor_cal_save();
        motors_set_enable(true);
        fport_cal_mode = false;
    }

    if (fport_cal_mode) {
        motor_cal(MOTOR1, pkt->ctrl.chan1);
        motor_cal(MOTOR2, pkt->ctrl.chan2);
        motor_cal(MOTOR3, pkt->ctrl.chan3);
        motor_cal(MOTOR4, pkt->ctrl.chan4);
    }
    else {
        do_brakes(pkt->ctrl.chan5);
        motor_set_speed(MOTOR1, pkt->ctrl.chan1);
        motor_set_speed(MOTOR2, pkt->ctrl.chan2);
        motor_set_speed(MOTOR3, pkt->ctrl.chan3);
        motor_set_speed(MOTOR4, pkt->ctrl.chan4);
    }
}

void fport_trigger(size_t len) {
    fport_gets(g_context.dma_rx, len);
}

void fport_dma_register(void) {
    DMAC_ChannelCallbackRegister(FPORT_DMA_CHANNEL, fport_dma_callback, (uintptr_t) &g_context);
    DMAC_ChannelCallbackRegister(FPORT_TX_DMA_CHANNEL, fport_tx_dma_callback, (uintptr_t) &g_context);
    fport_trigger(1);
}

void fport_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    struct fport_dma_context *context = (struct fport_dma_context*)contextHandle;
    g_packet_stats.total_bytes++;
//    if (context->print_bytes) {
//        SERCOM_USART_WriteByte(FTDI, context->dma_rx[0]);
//    }
    fport_trigger(1);
    switch(event) {
        case DMAC_TRANSFER_EVENT_COMPLETE: {
            if(context->dma_rx[0] == FPORT_START_OF_FRAME) {
                state = FPORT_SOF;
                g_packet_stats.discarded_bytes += fport_idx;
                fport_idx = 0;
                return;
            }
            if(context->byte_stuffed) {
                context->dma_rx[0] ^= FPORT_XOR_VAL;
                context->byte_stuffed = false;
            } else if(context->dma_rx[0] == FPORT_STUFF_MARK) {
                context->byte_stuffed = true;
                return;
            }
            fport_tick(context->dma_rx[0]);
            break;
        }
        case DMAC_TRANSFER_EVENT_NONE:
        case DMAC_TRANSFER_EVENT_ERROR:
        default:
            g_packet_stats.dma_error++;
            break;
    }
}

void fport_tick(uint8_t x) {
    switch(state) {
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
                g_packet_stats.discarded_bytes++;
            }
            break;
        case FPORT_FOUND:
            fport_buf.bytes[fport_idx++] = x;
            /*
             * SOF L K  d1 d2 d3 d4 d5 d6 d7 d8 d9 d10d11d12d13d14d15d16d17d18d19d20D21d22FlgRSICRCEOF
             * 7E 19 00 43 03 DE D0 D0 97 3E 56 4C 9C 15 AC 48 DF C4 93 07 3E F0 41 7B E2 00 0F E2 7E
             */
            if(fport_idx >= (fport_buf.ctrl.length + 2)) {
                g_packet_stats.total_packets++;
                if (fport_print) {
                    fport_debug_print(&fport_buf, fport_cal_mode);
                }
                switch(fport_buf.ctrl.kind) {
                    //todo if we're in pkt timeout, stop doing telemetry frames
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
                state = FPORT_SOF;
            }
            break;
        default:
            state = FPORT_SEEKING;
            break;
    }
}
