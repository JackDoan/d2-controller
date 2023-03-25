#include <string.h>
#include <stdio.h>
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
static bool fport_cal_mode = false;
static bool fport_print = false;
static enum fport_state state = FPORT_SOF;
struct fport_dma_context {
    bool byte_stuffed;
    uint8_t dma_rx[30];
};


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
    if(fport_print) {
        char fport_print_buf[64] = {0};
        print_hex(fport_print_buf, fport_buf.bytes, fport_idx);
    }
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
    if(!fport_check_control_packet(&pkt->ctrl)) {
        return;
    }

    if(is_failsafe_pkt(pkt->ctrl.flags)) {
        failsafe_activate();
        g_packet_stats.num_failsafes++;
        return;
    }
//    bool chan17 = pkt->ctrl.flags & SBUS_FLAG_CHANNEL_17;
//    bool chan18 = pkt->ctrl.flags & SBUS_FLAG_CHANNEL_18;

    if((pkt->ctrl.chan15 < 200) && (SYSTICK_GetTickCounter() < 5000)) {
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
    } else if (pkt->ctrl.chan15 > 200) {
        if(fport_cal_mode) {
            motor_cal_save();
        }
        fport_cal_mode = false;
        motors_set_enable(true);
    }

    if(fport_print) {
        char fport_print_buf[128] = {0};
//        if(!fport_cal_mode) {
//            sprintf(fport_print_buf, "%04d %04d %04d %04d %04d %04d %02x 16:%d %03d %02x %lu %lu %lu %lu\r\n",
////                sbus_to_duty_cycle(pkt->ctrl.chan0, TCC0_REGS->TCC_PER, &drive_sbus_params).magnitude,
//                    pkt->ctrl.chan0,
//                    pkt->ctrl.chan1,
//                    pkt->ctrl.chan2,
//                    pkt->ctrl.chan3,
//
//                    pkt->ctrl.chan4,
//                    pkt->ctrl.chan5,
//
//                    pkt->ctrl.flags,
//                    pkt->ctrl.chan15,
//                    pkt->ctrl.rssi, pkt->ctrl.crc,
//                    g_packet_stats.total_packets-g_packet_stats.valid_packets,
//                    g_packet_stats.total_packets, g_packet_stats.discarded_bytes, g_packet_stats.total_bytes);
//        }
//        else {
            sprintf(fport_print_buf, 
                    "1: %04d %04d %04d / "
                    "2: %04d %04d %04d / "
                    "3: %04d %04d %04d / "
                    "4: %04d %04d %04d\r\n",
                    get_motor(MOTOR1)->sbus_config.max, get_motor(MOTOR1)->sbus_config.mid, get_motor(MOTOR1)->sbus_config.min,
                    get_motor(MOTOR2)->sbus_config.max, get_motor(MOTOR2)->sbus_config.mid, get_motor(MOTOR2)->sbus_config.min,
                    get_motor(MOTOR3)->sbus_config.max, get_motor(MOTOR3)->sbus_config.mid, get_motor(MOTOR3)->sbus_config.min,
                    get_motor(MOTOR4)->sbus_config.max, get_motor(MOTOR4)->sbus_config.mid, get_motor(MOTOR4)->sbus_config.min);
//        }
        
        serial_puts(fport_print_buf);
//        print_hex(fport_print_buf, pkt, fport_idx);
    }
    if (fport_cal_mode) {
        motor_cal(MOTOR1, pkt->ctrl.chan0);
        motor_cal(MOTOR2, pkt->ctrl.chan1);
        motor_cal(MOTOR3, pkt->ctrl.chan2);
        motor_cal(MOTOR4, pkt->ctrl.chan3);
    }
    else {
        do_brakes(pkt->ctrl.chan4);
        motor_set_speed(MOTOR1, pkt->ctrl.chan0);
        motor_set_speed(MOTOR2, pkt->ctrl.chan1);
        motor_set_speed(MOTOR3, pkt->ctrl.chan2);
        motor_set_speed(MOTOR4, pkt->ctrl.chan3);
    }
}



static struct fport_dma_context g_context = {0};

void fport_trigger(size_t len) {
    fport_gets(g_context.dma_rx, len);
}

void fport_dma_register(void) {
    DMAC_ChannelCallbackRegister(FPORT_DMA_CHANNEL, fport_dma_callback, (uintptr_t) &g_context);
    fport_trigger(1);
}

uint8_t fport_dma_get_byte(void) {
    return g_context.dma_rx[0];
}

void fport_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    struct fport_dma_context *context = (struct fport_dma_context*)contextHandle;
    g_packet_stats.total_bytes++;
    switch(event) {
        case DMAC_TRANSFER_EVENT_COMPLETE: {
            if(context->dma_rx[0] == FPORT_START_OF_FRAME) {
                state = FPORT_SOF;
                if (fport_idx)
                    g_packet_stats.discarded_bytes+= fport_idx;
                fport_idx = 0;
                fport_trigger(1);
                return;
            }
            if(context->byte_stuffed) {
                context->dma_rx[0] ^= FPORT_XOR_VAL;
                context->byte_stuffed = false;
            } else if(context->dma_rx[0] == FPORT_STUFF_MARK) {
                context->byte_stuffed = true;
                fport_trigger(1);
                return;
            }
            fport_tick();
            break;
        }
        case DMAC_TRANSFER_EVENT_NONE:
        case DMAC_TRANSFER_EVENT_ERROR:
        default:
            break;
    }
}


void fport_tick(void) {
    uint8_t x = fport_dma_get_byte();
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
            if(fport_idx >= (fport_buf.ctrl.length + 2)) {
//                if(fport_print) {
//                    char fport_print_buf[64] = {0};
//                    print_hex(fport_print_buf, fport_buf.bytes, fport_idx);
//                }
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
                state = FPORT_SOF;
            }
            break;
        default:
            state = FPORT_SEEKING;
            break;
    }
    fport_trigger(1);
}
