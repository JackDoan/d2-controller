#include <stdio.h>
#include <string.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"
#include "plib_adc0.h"
#include "l9958.h"

#define FPORT_START_OF_FRAME 0x7e
#define FPORT_END_OF_FRAME 0x7e

static uint8_t fport_dma_rx[1] = {0};
static uint8_t fport_buf[30] = {0};
static uint8_t fport_idx = 0;

void fport_trigger(size_t len) {
    fport_gets(fport_dma_rx, len);
}

uint8_t fport_dma_get_byte(void) {
    return fport_dma_rx[0];
}

static bool fport_print = false;
void fport_enable_printing(bool enable) {
    fport_print = enable;
}

enum fport_state {
    FPORT_SEEKING,
    FPORT_FOUND_SOF,
    FPORT_FOUND_1,
    FPORT_FOUND,
    FPORT_STUFF_BYTE
};

static int crc_fail = 0; //todo make these measureable?
static int eof_fail = 0;

bool fport_check(uint8_t* pkt) {
    int length = pkt[0]+1;
    uint8_t crc = frskyCheckSum(pkt, length);
    if(crc != pkt[length]) {
        crc_fail++;
        return false;
    }
    packet_timer_watchdog_feed();
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
#define T1_FIRST_ID               0x0400 // -> 0004
#define BATT_ID                   0xF104


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

void fport_proc_telemetry_req(uint8_t* pkt) {
    //note: very timing sensitive. Don't remove the delay
    static bool send = false;
    static uint8_t i = 0;

    if(!fport_check(pkt)) {
        return;
    }
    send = !send;
/*
 * why he jump
 * (236157) 1900e0031ff8c0c70af0810f7c08400002108000042000010062f7
(236165) 1900c000000000c0800f000060f97e7e080110000000000000e67e
1900e00280800700010e7c00000000000046147e7e080110000000
 (400350) 1900e60b5ff8ccc70af0810f7c08400002108000042000010059a6
(400358) 1900c0008004e0000e7c00005a607e7e080110000000000000e67e

 */
//    static char fport_print_buf[64] = {0};
//    if(fport_print) {
//        print_hex(fport_print_buf, pkt, fport_idx);
//    } else {
        SYSTICK_DelayMs(1);
//    }


    if(!send)
        return;
//    data[9] = frskyCheckSum(data, 8);
    union fport_response data = {
            .len = 0x8,
            .uplink = 0x81,
            .type = 0x10,
    };

    i = (i+1) % 16;
    if(i & 1) {
        return;
    }
    switch(i/2) {
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
        default:
            return;
    }

    data.crc = frskyCheckSum(data.bytes, 8);
    fport_enable_tx(true);
    SERCOM_USART_Write(RX, data.bytes, sizeof(data));
    fport_enable_tx(false);
}
static uint32_t rssi_invalid = 0;
void fport_proc_packet(uint8_t* pkt) {
    static char fport_print_buf[64] = {0};
    struct fport_frame *frame = (struct fport_frame *)pkt;
    //todo log dropped packets

    uint8_t crc = frskyCheckSum(pkt, 28-2);
    if(crc != frame->crc) {
        crc_fail++;
        return;
    }

    if(frame->rssi > 100 || frame->rssi < 10) {
        rssi_invalid++;
        return;
    }

    packet_timer_watchdog_feed();

    if(frame->flags & SBUS_FLAG_FAILSAFE_ACTIVE) {
        failsafe_activate();
        if(fport_print) {
            sprintf(fport_print_buf, "FAILSAFE %02x\r\n", frame->flags);
            serial_puts(fport_print_buf);
        }
        return;
    }

    //todo packet timeouts
    //todo sanity checks, filters?

    if (frame->flags & SBUS_FLAG_CHANNEL_17) {

    }

    if (frame->flags & SBUS_FLAG_CHANNEL_18) {

    }

    if (frame->flags & SBUS_FLAG_SIGNAL_LOSS) {

    }

    do_brakes(frame->chan4);
    motor_set_speed(MOTOR1, frame->chan0);
    motor_set_speed(MOTOR2, frame->chan1);
    motor_set_speed(MOTOR3, frame->chan2);
    motor_set_speed(MOTOR4, frame->chan3);

    if(fport_print) {
        print_hex(fport_print_buf, pkt, fport_idx);
    }
}

static enum fport_state state = FPORT_SEEKING;
void proc_fport_rx(void) {
    //todo log dropped bytes?
    uint8_t x = fport_dma_get_byte();
    switch(state) {
        case FPORT_SEEKING:
            fport_idx = 0;
            if(x == FPORT_START_OF_FRAME) {
                state = FPORT_FOUND_SOF;
            }
            else {
                //we don't want it
            }
            break;
        case FPORT_FOUND_SOF:
            fport_idx = 0;
            if(x == FPORT_START_OF_FRAME) {
                state = FPORT_FOUND_1;
            }
//            else if(x == 0x19) { //hmm why does this work
//                fport_buf[fport_idx++] = x;
//                state = FPORT_FOUND;
//            }
            else {
                state = FPORT_SEEKING;
            }
            break;
        case FPORT_FOUND_1:
            if(x == 0x19) {
                fport_buf[fport_idx++] = x;
                state = FPORT_FOUND;
            }
            else if(x == 0x08) {
                fport_buf[fport_idx++] = x;
                state = FPORT_FOUND;
            }
            else {
                fport_idx = 0; //we don't want it
            }
            break;
        case FPORT_STUFF_BYTE:
            x ^= 0x20;
            goto proc_byte;
        case FPORT_FOUND: //todo is this all I need for byte-stuffing?
            if(x == 0x7d) { //byte stuffing
                state = FPORT_STUFF_BYTE;
                break;
            }
        proc_byte:
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
                        eof_fail++;
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
