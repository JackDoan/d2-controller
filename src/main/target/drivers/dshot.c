#include "dshot.h"
#include "device.h"
#include "sercom_spi_master.h"
#include "plib_dmac.h"

typedef enum {
    DSHOT_CMD_MOTOR_STOP = 0,
    DSHOT_CMD_BEACON1,
    DSHOT_CMD_BEACON2,
    DSHOT_CMD_BEACON3,
    DSHOT_CMD_BEACON4,
    DSHOT_CMD_BEACON5,
    DSHOT_CMD_ESC_INFO, // V2 includes settings
    DSHOT_CMD_SPIN_DIRECTION_1,
    DSHOT_CMD_SPIN_DIRECTION_2,
    DSHOT_CMD_3D_MODE_OFF,
    DSHOT_CMD_3D_MODE_ON,
    DSHOT_CMD_SETTINGS_REQUEST, // Currently not implemented
    DSHOT_CMD_SAVE_SETTINGS,
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,
    DSHOT_CMD_LED0_ON, // BLHeli32 only
    DSHOT_CMD_LED1_ON, // BLHeli32 only
    DSHOT_CMD_LED2_ON, // BLHeli32 only
    DSHOT_CMD_LED3_ON, // BLHeli32 only
    DSHOT_CMD_LED0_OFF, // BLHeli32 only
    DSHOT_CMD_LED1_OFF, // BLHeli32 only
    DSHOT_CMD_LED2_OFF, // BLHeli32 only
    DSHOT_CMD_LED3_OFF, // BLHeli32 only
    DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF = 30, // KISS audio Stream mode on/Off
    DSHOT_CMD_SILENT_MODE_ON_OFF = 31, // KISS silent Mode on/Off
    DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE = 32,
    DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY = 33,
    DSHOT_CMD_MAX = 47
} dshotCommands_e;

struct dshot_context {
    uint16_t speed;
    uint16_t frame;
};

static struct dshot_context g_context = { 0 };

static uint16_t calc_dshot_command(uint16_t command) {
    //command is a magic number from 0 to 47, or a speed from 48 to 2047
    //https://www.swallenhardware.io/battlebots/2019/4/20/a-developers-guide-to-dshot-escs
    //compute the checksum. xor the three nibbles of the speed + the telemetry bit (not used here)
    uint16_t checksum = 0;
    uint16_t checksum_data = command << 1;
    for (uint8_t i = 0; i < 3; i++) {
        checksum ^= checksum_data;
        checksum_data >>= 4;
    }
    checksum &= 0x000F;//we only use the least-significant four bits as checksum
    uint16_t dshot_frame = (command << 5) | checksum; //add in the checksum bits to the least-four-significant bits
    return dshot_frame;
}

static void do_tx(void) {
    if(DMAC_ChannelIsBusy(DSHOT_DMA_CHANNEL))
        return;
    g_context.frame = calc_dshot_command(g_context.speed);
    //todo convert to dshot format?
    DMAC_ChannelTransfer(
            DSHOT_DMA_CHANNEL,
            &g_context.frame,
            (const void *)&DSHOT->SPIM.SERCOM_DATA,
            sizeof(g_context.frame)
    );
}



void dshot_set_speed(uint16_t speed) {
    if (speed > DSHOT_MAX)
        speed = 0; //reject invalid speeds
    g_context.speed = speed+DSHOT_CMD_MAX+1;
}

static void dmaCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
//    struct dshot_context *ctx = (struct dshot_context*)contextHandle;
    switch(event) {
        case DMAC_TRANSFER_EVENT_COMPLETE:
            do_tx();
            break;
        case DMAC_TRANSFER_EVENT_NONE:
        case DMAC_TRANSFER_EVENT_ERROR:
        default:
            break;
    }
}

void dshot_init(void) {
    SERCOM_SPI_Initialize(DSHOT);
    DMAC_ChannelCallbackRegister(DSHOT_DMA_CHANNEL, dmaCallback, (uintptr_t) &g_context);
    dshot_set_speed(0);
    do_tx();
}

void dshot_tick(void) {
    //todo remove
}