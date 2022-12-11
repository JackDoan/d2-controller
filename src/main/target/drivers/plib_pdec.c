#include "plib_pdec.h"
#include "pic32cm6408mc00032.h"
#include "plib_port.h"
#include "boards/motortest-rev1.h"

#define PDEC_IO2_ENABLE (1 << 18)
#define PDEC_IO1_ENABLE (1 << 17)
#define PDEC_IO0_ENABLE (1 << 16)

#define PDEC_ENABLE (1 << 1)
#define PDEC_SWRESET (1 << 0)

enum pdec_cmd {
    PDEC_CMD_NONE = 0,
    PDEC_CMD_RETRIGGER = (1 << 5),
    PDEC_CMD_UPDATE = (2 << 5),
    PDEC_CMD_READSYNC = (3 << 5),
    PDEC_CMD_START = (4 << 5),
    PDEC_CMD_STOP = (5 << 5),
};

static inline void sync(void) {
    while((PDEC_REGS->PDEC_SYNCBUSY) != 0U) { }
}

static inline void cmd(enum pdec_cmd x) {
    PDEC_REGS->PDEC_CTRLBSET |= x;
    sync();
}

void PDEC_Init(void) {
    PORT_PinPeripheralFunctionConfig(PDEC_A, PERIPHERAL_FUNCTION_G);
    PORT_PinPeripheralFunctionConfig(PDEC_B, PERIPHERAL_FUNCTION_G);
    PDEC_REGS->PDEC_CTRLA = PDEC_SWRESET;
    sync();
    PDEC_REGS->PDEC_CTRLA |= PDEC_ENABLE | PDEC_IO0_ENABLE | PDEC_IO1_ENABLE;
    sync();
    cmd(PDEC_CMD_START);
}

uint32_t PDEC_Counts(void) {
    cmd(PDEC_CMD_READSYNC);
    return PDEC_REGS->PDEC_COUNT;
}
