#include <stdio.h>
#include <string.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"

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
    for (int i = 0; i < len; i++) {
        sprintf(&(buf[(2 * i)]), "%02x", pkt[i]);
    }
    sprintf(&(buf[(2 * len)]), "\r\n");
    serial_puts(buf);
}
#define T1_FIRST_ID               0x0400 // -> 0004
#define BATT_ID                   0xF104 // -> 04f1
void fport_proc_telemetry_req(uint8_t* pkt) {
    static bool send = false;
    static uint8_t i = 0;

    uint8_t data[] = {0x08, 0x81, 0x10, 0x04, 0xf1, i++, 0x0, 0x00, 0x00, 0};
    if(!fport_check(pkt)) {
        return;
    }
    send = !send;

    static char fport_print_buf[64] = {0};
    if(fport_print) {
        print_hex(fport_print_buf, pkt, fport_idx);
    }

    if(!send)
        return;
    data[9] = frskyCheckSum(data, 8);
    fport_enable_tx(true);
    SERCOM_USART_Write(RX, data, sizeof(data));
//    SERCOM_USART_Write_Nonblock(RX, data, sizeof (data));
//    SERCOM_USART_TX_Wait(RX);
    fport_enable_tx(false);


}

void fport_proc_packet(uint8_t* pkt) {
    static char fport_print_buf[64] = {0};
    struct fport_frame *frame = (struct fport_frame *)pkt;
    //todo log dropped packets

    uint8_t crc = frskyCheckSum(pkt, 28-2);
    if(crc != frame->crc) {
        crc_fail++;
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

    motors_set_enable(true); //todo disable per-motor?
    motor_set_speed(MOTOR3, frame->chan0);
    motor_set_speed(MOTOR1, frame->chan0);
    motor_set_speed(MOTOR2, frame->chan1);
    //todo motor_set_speed(MOTOR3, frame->chan2);
    motor_set_speed(MOTOR4, frame->chan3);

    if(fport_print) {
//        print_hex(fport_print_buf, pkt, fport_idx/2);
    }
}

static enum fport_state state = FPORT_SEEKING;
void proc_fport_rx(void) {
    //todo log dropped bytes?
    uint8_t x = fport_dma_get_byte();
//    fport_trigger(1);
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
//            fport_trigger(1);
            if(fport_idx >= (fport_buf[0]+2)) {
//            if(fport_idx >= 28) {
                switch(fport_buf[1]) {
                    case 0:
                        fport_proc_packet(fport_buf);
//                        fport_trigger(1);
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
