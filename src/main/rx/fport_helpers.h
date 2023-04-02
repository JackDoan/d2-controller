#ifndef D2_FPORT_HELPERS_H
#define D2_FPORT_HELPERS_H

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

void fport_debug_print(union fport_pkt* pkt, bool cal_mode);
void fport_puts(const uint8_t* buffer);
void fport_tx_dma_callback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle);

#endif //D2_FPORT_HELPERS_H
