#include <stdio.h>
#include "fport.h"
#include "sercom_usart.h"
#include "common/rx/sbus_channels.h"

static uint8_t fport_dma_rx[1] = {0};
static uint8_t fport_buf[30] = {0};
static uint8_t fport_idx = 0;
static char fport_print_buf[64] = {0};

void fport_trigger(size_t len) {
    fport_gets(fport_dma_rx, len);
}

enum fport_state {
    FPORT_SEEKING,
    FPORT_FOUND_1,
    FPORT_FOUND,
};

void fport_proc_packet(uint8_t* pkt) {
    if(pkt[27] != 0x7e) {
        return; //todo log somehow
    }
    struct sbusChannels_s *frame = (struct sbusChannels_s *) &(pkt[2]);
    //todo byte stuffing
    //todo check CRC!!!
    //todo packet timeouts
    //todo failsafe! and sanity checks

    if(frame->flags & SBUS_FLAG_FAILSAFE_ACTIVE) {
        sprintf(fport_print_buf, "FAILSAFE %02x\r\n", frame->flags);
        motor_enable(MOTOR3, false);
        //todo nix PWM too
        serial_puts(fport_print_buf);
        return;
    }
    motor_enable(MOTOR3, true);
    motor_set_speed(MOTOR3, frame->chan0);

    for(int i = 0; i < fport_idx; i++) {
        sprintf(&(fport_print_buf[2*i]), "%02x", pkt[i]);
    }
    sprintf(&(fport_print_buf[2*fport_idx]), "\r\n");
    serial_puts(fport_print_buf);
}

static enum fport_state state = FPORT_SEEKING;
void proc_fport_rx(void) {
    uint8_t x = fport_dma_rx[0];
    switch(state) {
        case FPORT_SEEKING:
            if(x == 0x7e) {
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
        case FPORT_FOUND:
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