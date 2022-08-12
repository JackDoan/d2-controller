#include <stdio.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"

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

enum fport_state {
    FPORT_SEEKING,
    FPORT_FOUND_1,
    FPORT_FOUND,
    FPORT_STUFF_BYTE
};

static int crc_fail = 0; //todo make these measureable?
static int eof_fail = 0;

void fport_proc_packet(uint8_t* pkt) {
    static char fport_print_buf[64] = {0};
    struct fport_frame *frame = (struct fport_frame *)pkt;
    //todo log dropped packets
    if(frame->eof != FPORT_END_OF_FRAME) {
        eof_fail++;
        return;
    }

    uint8_t crc = frskyCheckSum(pkt, 28-2);
    if(crc != frame->crc) {
        crc_fail++;
        return;
    } else {
        packet_timer_watchdog_feed();
    }

    if(frame->flags & SBUS_FLAG_FAILSAFE_ACTIVE) {
        failsafe_activate();
        sprintf(fport_print_buf, "FAILSAFE %02x\r\n", frame->flags);
        serial_puts(fport_print_buf);
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

    for(int i = 0; i < fport_idx; i++) {
        sprintf(&(fport_print_buf[2*i]), "%02x", pkt[i]);
    }
    sprintf(&(fport_print_buf[2*fport_idx]), "\r\n");
    serial_puts(fport_print_buf);
}

static enum fport_state state = FPORT_SEEKING;
void proc_fport_rx(void) {
    //todo log dropped bytes?
    uint8_t x = fport_dma_get_byte();
    switch(state) {
        case FPORT_SEEKING:
            if(x == FPORT_START_OF_FRAME) {
                state = FPORT_FOUND_1;
            }
            else {
                //we don't want it
            }
            fport_idx = 0;
            fport_trigger(1);
            break;
        case FPORT_FOUND_1:
            if(x == 0x19) {
                fport_buf[fport_idx++] = x;
                state = FPORT_FOUND;
            }
            else {
                fport_idx = 0; //we don't want it
            }
            fport_trigger(1);
            break;
        case FPORT_STUFF_BYTE:
            x ^= 0x20;
            goto proc_byte;
        case FPORT_FOUND: //todo is this all I need for byte-stuffing?
            if(x == 0x7d) { //byte stuffing
                state = FPORT_STUFF_BYTE;
                fport_trigger(1);
                break;
            }
        proc_byte:
            fport_buf[fport_idx++] = x;
            fport_trigger(1);
            if(fport_idx >= 28) {
                fport_proc_packet(fport_buf);
                state = FPORT_SEEKING;
            }
            break;
        default:
            state = FPORT_SEEKING;
            fport_trigger(1);
            break;
    }
}
