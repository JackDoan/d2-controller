#include "l9958.h"
#include "device.h"
#include "sercom_spi_master.h"

union l9958_config {
    struct {
        bool res0: 1;
        bool diag_reset: 1;
        bool cl1: 1;
        bool cl2: 1;

        bool res4: 1;
        bool res5: 1;
        bool res6: 1;
        bool res7: 1;

        bool voltage_slew: 1;
        bool current_slew: 1;
        bool current_slew_enable: 1;
        bool open_load_enable: 1;

        bool res12: 1;
        bool res13: 1;
        bool write_0_14: 1;
        bool write_0_15: 1;
    };
    uint8_t bytes[2];
    uint16_t word;
};

union l9958_diag {
     struct {
        bool open_load_off: 1;
        bool open_load_on: 1;
        bool vbatt_uv: 1;
        bool vdd_ov: 1;

        bool current_limit_active: 1;
        bool temp_warn: 1;
        bool temp_shutdown: 1;
        bool bridge_active: 1;

        bool oc_ls1: 1;
        bool oc_ls2: 1;
        bool oc_hs1: 1;
        bool os_hs2: 1;

        bool res12: 1;
        bool res13: 1;
        bool short_to_gnd_off: 1;
        bool short_to_batt_off: 1;
    };
    uint8_t bytes[2];
    uint16_t word;
};

static union l9958_config config_data = {
        .diag_reset = 1,
        .cl1 = 1,
        .cl2 = 1,
};
static PORT_PIN cs_lines[] = {CS1, CS2, CS3, CS4};

struct l9958_context {
    enum motor_channel active;
    union l9958_diag rx_buf[MOTOR_COUNT];
};

static struct l9958_context g_context = {0};

void do_config_transaction(enum motor_channel channel) {
    SERCOM_SPI_WriteRead(
        cs_lines[channel],
        &config_data,
        2,
        &g_context.rx_buf[channel],
        2
    );
}

static void spi_callback(void) {
    g_context.active = (g_context.active + 1) % MOTOR_COUNT;
}

uint16_t L9958_Diag_Read(enum motor_channel channel) {
    return g_context.rx_buf[channel].word;
}

void L9958_Init(void) {
    SERCOM_SPI_Initialize(SPI);
    SERCOM_SPI_CallbackRegister(spi_callback);
}

void L9958_Tick(void) {
    //todo check register contents for errors and act on them
    //todo kick off reads on a regular interval (not as fast as possible though?)
    do_config_transaction(g_context.active);
}

static bool L9958_has_problem(enum motor_channel channel) {
    switch (g_context.rx_buf[channel].word) {
        case 0x0000: // no problems, bridge off
        case 0x0080: //no problems, bridge on
            return false;
        default:
            return true;
    }
}

uint32_t L9958_has_problems(void) {
    uint32_t problem[4] = {0};
    problem[0] = (1 & L9958_has_problem(MOTOR1));
    problem[1] = (20 * L9958_has_problem(MOTOR2));
    problem[2] = (300 * L9958_has_problem(MOTOR3));
    problem[3] = (4000 * L9958_has_problem(MOTOR4));
    return problem[0] + problem[1] + problem[2] + problem[3];
}