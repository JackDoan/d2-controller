#include "fport_helpers.h"
#include <string.h>
#include <stdio.h>
#include "fport.h"
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "plib_systick.h"
#include "plib_adc0.h"
#include "l9958.h"
#include "helpers.h"

void fport_debug_print(union fport_pkt* pkt, bool cal_mode) {
    if(serial_busy())
        return;
    char fport_print_buf[128] = {0};
    if(!cal_mode) {
//        sprintf(fport_print_buf, "%04lu/%04d %04d %04d %04d 16:%d %02x RSSI:%03d CRC:%02x\r\n",
//                sbus_to_duty_cycle(pkt->ctrl.chan0, get_motor(MOTOR1)).magnitude,
//                pkt->ctrl.chan0,
//                pkt->ctrl.chan1,
//                pkt->ctrl.chan2,
//                pkt->ctrl.chan3,
//
//                pkt->ctrl.chan15,
//                pkt->ctrl.flags,
//
//                pkt->ctrl.rssi, pkt->ctrl.crc
//        );
//        serial_puts(fport_print_buf);
        print_hex(fport_print_buf, pkt->bytes, pkt->ctrl.length);
    }
    else {
        sprintf(fport_print_buf,
                "1: %04d %04d %04d / "
                "2: %04d %04d %04d / "
                "3: %04d %04d %04d / "
                "4: %04d %04d %04d\r\n",
                get_motor(MOTOR1)->sbus_config.max, get_motor(MOTOR1)->sbus_config.mid, get_motor(MOTOR1)->sbus_config.min,
                get_motor(MOTOR2)->sbus_config.max, get_motor(MOTOR2)->sbus_config.mid, get_motor(MOTOR2)->sbus_config.min,
                get_motor(MOTOR3)->sbus_config.max, get_motor(MOTOR3)->sbus_config.mid, get_motor(MOTOR3)->sbus_config.min,
                get_motor(MOTOR4)->sbus_config.max, get_motor(MOTOR4)->sbus_config.mid, get_motor(MOTOR4)->sbus_config.min);
        serial_puts(fport_print_buf);
    }
}

void fport_tx_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    fport_enable_tx(false);
}

void fport_puts(const uint8_t* buffer) {
    static char print_buf[64] = {0};
    memset(print_buf, 0, sizeof(print_buf));
    int print_buf_idx = 0;
    for (unsigned int i = 0; i < sizeof (struct fport_telemetry); i++) {
        uint8_t c = buffer[i];
        if (c == FPORT_STUFF_MARK || c == FPORT_START_OF_FRAME) {
            print_buf[print_buf_idx++] = FPORT_STUFF_MARK;
            print_buf[print_buf_idx++] = c ^ FPORT_XOR_VAL;
        } else {
            print_buf[print_buf_idx++] = c;
        }
    }
    if(!DMAC_ChannelIsBusy(FPORT_TX_DMA_CHANNEL)) {
        fport_enable_tx(true);
        DMAC_ChannelTransfer(FPORT_TX_DMA_CHANNEL, print_buf, (const void *)&RX->USART_INT.SERCOM_DATA, print_buf_idx);
    }
}

