//
// Created by jack on 11/6/22.
//

#include "plib_pdec.h"
#include "pic32cm6408mc00032.h"

#define PDEC_IO2_ENABLE (1 << 18)
#define PDEC_IO1_ENABLE (1 << 17)
#define PDEC_IO0_ENABLE (1 << 16)

#define PDEC_ENABLE (1 << 1)
#define PDEC_SWRESET (1 << 0)

static void sync(void) {
    while((PDEC_REGS->PDEC_SYNCBUSY) != 0U) { }
}

void PDEC_Init(void) {
    PDEC_REGS->PDEC_CTRLA = PDEC_SWRESET;
    sync();
    PDEC_REGS->PDEC_CTRLA |= PDEC_ENABLE | PDEC_IO0_ENABLE | PDEC_IO1_ENABLE;
}

uint32_t PDEC_Counts(void) {
    return PDEC_REGS->PDEC_COUNT;
}
