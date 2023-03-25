#include <stdio.h>
#include <string.h>
#include "sercom_usart.h"
#include "rx/frsky_crc.h"
#include "helpers.h"

void print_hex(void* buf, uint8_t* pkt, int len) {
    char* str = (char*)buf;
    for (int i = 0; i < len; i++) {
        sprintf(&(str[(2 * i)]), "%02x", pkt[i]);
    }
    sprintf(&(str[(2 * len)]), "\r\n");
    serial_puts(str);
}