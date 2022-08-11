#include "interrupts.h"
#include "plib_dmac.h"

#define DMAC_CHANNELS_NUMBER        4U


typedef struct {
    uint8_t                inUse;
    DMAC_CHANNEL_CALLBACK  callback;
    uintptr_t              context;
    bool                busyStatus;
} DMAC_CH_OBJECT ;

/* Initial write back memory section for DMAC */
static dmac_descriptor_registers_t write_back_section[DMAC_CHANNELS_NUMBER]    __ALIGNED(8);

/* Descriptor section for DMAC */
static dmac_descriptor_registers_t  descriptor_section[DMAC_CHANNELS_NUMBER]    __ALIGNED(8);

/* DMAC Channels object information structure */
static DMAC_CH_OBJECT dmacChannelObj[DMAC_CHANNELS_NUMBER];

void DMAC_Initialize(void) {
    /* Initialize DMAC Channel objects */
    for(uint16_t channel = 0U; channel < DMAC_CHANNELS_NUMBER; channel++) {
        dmacChannelObj[channel].inUse = 0U;
        dmacChannelObj[channel].callback = NULL;
        dmacChannelObj[channel].context = 0U;
        dmacChannelObj[channel].busyStatus = false;
    }

    /* Update the Base address and Write Back address register */
    DMAC_REGS->DMAC_BASEADDR = (uint32_t) descriptor_section;
    DMAC_REGS->DMAC_WRBADDR  = (uint32_t) write_back_section;

    /* Update the Priority Control register */
    DMAC_REGS->DMAC_PRICTRL = DMAC_PRICTRL_LVLPRI0(1UL) | DMAC_PRICTRL_RRLVLEN0_Msk | DMAC_PRICTRL_LVLPRI1(1UL) | DMAC_PRICTRL_RRLVLEN1_Msk | DMAC_PRICTRL_LVLPRI2(1UL) | DMAC_PRICTRL_RRLVLEN2_Msk | DMAC_PRICTRL_LVLPRI3(1UL) | DMAC_PRICTRL_RRLVLEN3_Msk;
    DMAC_REGS->DMAC_DBGCTRL |= DMAC_DBGCTRL_DBGRUN(1);
    /***************** Configure DMA channel 0 ********************/
    DMAC_REGS->DMAC_CHID = 0U;
    DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT(2UL) |
            DMAC_CHCTRLB_TRIGSRC(5UL) |
            DMAC_CHCTRLB_LVL(0UL);

    descriptor_section[0].DMAC_BTCTRL = (uint16_t)(DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_SRCINC_Msk );

    dmacChannelObj[0].inUse = 1U;
    DMAC_REGS->DMAC_CHINTENSET = (uint8_t)(DMAC_CHINTENSET_TERR_Msk | DMAC_CHINTENSET_TCMPL_Msk);
    /***************** Configure DMA channel 1 ********************/

    DMAC_REGS->DMAC_CHID = 1U;
    DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT(2UL) |
            DMAC_CHCTRLB_TRIGSRC(4UL) |
            DMAC_CHCTRLB_LVL(0UL) ;

    descriptor_section[1].DMAC_BTCTRL = (uint16_t)(DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_DSTINC_Msk );

    dmacChannelObj[1].inUse = 1U;
    DMAC_REGS->DMAC_CHINTENSET = (uint8_t)(DMAC_CHINTENSET_TERR_Msk | DMAC_CHINTENSET_TCMPL_Msk);

    /***************** Configure DMA channel 3 ********************/

    DMAC_REGS->DMAC_CHID = 3U;
    DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT(2UL) |
                              DMAC_CHCTRLB_TRIGSRC(2UL) |
                              DMAC_CHCTRLB_LVL(0UL) ;

    descriptor_section[3].DMAC_BTCTRL = (uint16_t)(DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_DSTINC_Msk );

    dmacChannelObj[3].inUse = 1U;
    DMAC_REGS->DMAC_CHINTENSET = (uint8_t)(DMAC_CHINTENSET_TERR_Msk | DMAC_CHINTENSET_TCMPL_Msk);

    /**end channel config***/

    /* Enable the DMAC module & Priority Level x Enable */
    DMAC_REGS->DMAC_CTRL = (uint16_t)(DMAC_CTRL_DMAENABLE_Msk | DMAC_CTRL_LVLEN0_Msk | DMAC_CTRL_LVLEN1_Msk | DMAC_CTRL_LVLEN2_Msk | DMAC_CTRL_LVLEN3_Msk);
}

/*******************************************************************************
    This function schedules a DMA transfer on the specified DMA channel.
********************************************************************************/

bool DMAC_ChannelTransfer(DMAC_CHANNEL channel, const void *srcAddr, const void *destAddr, size_t blockSize ) {
    const uint32_t* pu32srcAddr = (const uint32_t*)srcAddr;
    const uint32_t* pu32dstAddr = (const uint32_t*)destAddr;

    /* Save channel ID */
    uint8_t channelId = DMAC_REGS->DMAC_CHID;

    /* Set the DMA channel */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    if (
        (dmacChannelObj[channel].busyStatus != false) ||
        ((DMAC_REGS->DMAC_CHINTFLAG & (DMAC_CHINTENCLR_TCMPL_Msk | DMAC_CHINTENCLR_TERR_Msk)) != 0U)
    ) {
        /* Restore channel ID */
        DMAC_REGS->DMAC_CHID = channelId;
        return false;
    }

    /* Clear the transfer complete flag */
    DMAC_REGS->DMAC_CHINTFLAG = DMAC_CHINTENCLR_TCMPL_Msk | DMAC_CHINTENCLR_TERR_Msk;

    dmacChannelObj[channel].busyStatus = true;

    /* Get a pointer to the module hardware instance */
    dmac_descriptor_registers_t *const dmacDescReg = &descriptor_section[channel];

    /* Set source address */
    if ((dmacDescReg->DMAC_BTCTRL & DMAC_BTCTRL_SRCINC_Msk) == DMAC_BTCTRL_SRCINC_Msk)
        dmacDescReg->DMAC_SRCADDR = ((uintptr_t)pu32srcAddr + blockSize);
    else
        dmacDescReg->DMAC_SRCADDR = (uintptr_t)(pu32srcAddr);

    /* Set destination address */
    if ((dmacDescReg->DMAC_BTCTRL & DMAC_BTCTRL_DSTINC_Msk) == DMAC_BTCTRL_DSTINC_Msk)
        dmacDescReg->DMAC_DSTADDR = ((uintptr_t)pu32dstAddr + blockSize);
    else
        dmacDescReg->DMAC_DSTADDR = (uintptr_t)(pu32dstAddr);

    /* Calculate the beat size and then set the BTCNT value */
    uint8_t beat_size = (uint8_t)((dmacDescReg->DMAC_BTCTRL & DMAC_BTCTRL_BEATSIZE_Msk) >> DMAC_BTCTRL_BEATSIZE_Pos);

    /* Set Block Transfer Count */
    dmacDescReg->DMAC_BTCNT = (uint16_t)(blockSize / (1UL << beat_size));

    /* Enable the channel */
    DMAC_REGS->DMAC_CHCTRLA |= (uint8_t)DMAC_CHCTRLA_ENABLE_Msk;

    /* Verify if Trigger source is Software Trigger */
    bool triggerCondition = ((DMAC_REGS->DMAC_CHCTRLB & DMAC_CHCTRLB_EVIE_Msk) != DMAC_CHCTRLB_EVIE_Msk);
    triggerCondition &= (((DMAC_REGS->DMAC_CHCTRLB & DMAC_CHCTRLB_TRIGSRC_Msk) >> DMAC_CHCTRLB_TRIGSRC_Pos) == 0x00U);
    if (triggerCondition) {
        /* Trigger the DMA transfer */
        DMAC_REGS->DMAC_SWTRIGCTRL |= (1UL << (uint32_t)channel);
    }

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;

    return true;
}


bool DMAC_ChannelIsBusy ( DMAC_CHANNEL channel ) {
    bool isBusy = false;

    /* Save channel ID */
    uint8_t channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA channel */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    if ((dmacChannelObj[channel].busyStatus == true) && ((DMAC_REGS->DMAC_CHINTFLAG & (DMAC_CHINTENCLR_TCMPL_Msk | DMAC_CHINTENCLR_TERR_Msk)) == 0U)) {
        isBusy = true;
    }

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;

    return isBusy;
}

DMAC_TRANSFER_EVENT DMAC_ChannelTransferStatusGet(DMAC_CHANNEL channel)
{
    uint32_t chanIntFlagStatus = 0;
    uint8_t channelId = 0U;

    DMAC_TRANSFER_EVENT event = DMAC_TRANSFER_EVENT_NONE;

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA channel */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    /* Get the DMAC channel interrupt status */
    chanIntFlagStatus = DMAC_REGS->DMAC_CHINTFLAG;

    if ((chanIntFlagStatus & DMAC_CHINTENCLR_TCMPL_Msk) != 0U)
    {
        event = DMAC_TRANSFER_EVENT_COMPLETE;
    }

    /* Verify if DMAC Channel Error flag is set */
    if ((chanIntFlagStatus & DMAC_CHINTENCLR_TERR_Msk) != 0U)
    {
        event = DMAC_TRANSFER_EVENT_ERROR;
    }

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;

    return event;
}

void DMAC_ChannelDisable ( DMAC_CHANNEL channel )
{
    uint8_t channelId = 0U;

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA Channel ID */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    /* Disable the DMA channel */
    DMAC_REGS->DMAC_CHCTRLA &= (uint8_t)(~DMAC_CHCTRLA_ENABLE_Msk);

    while((DMAC_REGS->DMAC_CHCTRLA & DMAC_CHCTRLA_ENABLE_Msk) != 0U)
    {
        /* Wait for Channel enable */
    }

    dmacChannelObj[channel].busyStatus = false;

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;
}

uint16_t DMAC_ChannelGetTransferredCount( DMAC_CHANNEL channel )
{
    uint16_t transferredCount = descriptor_section[channel].DMAC_BTCNT;
    transferredCount -= write_back_section[channel].DMAC_BTCNT;
    return(transferredCount);
}


void DMAC_ChannelSuspend ( DMAC_CHANNEL channel )
{
    uint8_t channelId = 0;

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA Channel ID */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    /* Suspend the DMA channel */
    DMAC_REGS->DMAC_CHCTRLB = (DMAC_REGS->DMAC_CHCTRLB & ~DMAC_CHCTRLB_CMD_Msk) | DMAC_CHCTRLB_CMD_SUSPEND;

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;
}

void DMAC_ChannelResume ( DMAC_CHANNEL channel )
{
    uint8_t channelId = 0;

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA Channel ID */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    /* Suspend the DMA channel */
    DMAC_REGS->DMAC_CHCTRLB = (DMAC_REGS->DMAC_CHCTRLB & ~DMAC_CHCTRLB_CMD_Msk) | DMAC_CHCTRLB_CMD_RESUME;

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;
}

/*******************************************************************************
    This function function allows a DMAC PLIB client to set an event handler.
********************************************************************************/
void DMAC_ChannelCallbackRegister( DMAC_CHANNEL channel, const DMAC_CHANNEL_CALLBACK eventHandler, const uintptr_t contextHandle )
{
    dmacChannelObj[channel].callback = eventHandler;
    dmacChannelObj[channel].context  = contextHandle;
}

/*******************************************************************************
    This function returns the current channel settings for the specified DMAC Channel
********************************************************************************/

DMAC_CHANNEL_CONFIG DMAC_ChannelSettingsGet (DMAC_CHANNEL channel)
{
    /* Get a pointer to the module hardware instance */
    dmac_descriptor_registers_t *const dmacDescReg = &descriptor_section[0];

    return (dmacDescReg[channel].DMAC_BTCTRL);
}

/*******************************************************************************
    This function changes the current settings of the specified DMAC channel.
********************************************************************************/
bool DMAC_ChannelSettingsSet (DMAC_CHANNEL channel, DMAC_CHANNEL_CONFIG settings)
{
    uint8_t channelId = 0U;

    /* Get a pointer to the module hardware instance */
    dmac_descriptor_registers_t *const dmacDescReg = &descriptor_section[0];

    /* Save channel ID */
    channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Set the DMA Channel ID */
    DMAC_REGS->DMAC_CHID = (uint8_t)channel;

    /* Disable the DMA channel */
    DMAC_REGS->DMAC_CHCTRLA &= (uint8_t)(~DMAC_CHCTRLA_ENABLE_Msk);

    /* Wait for channel to be disabled */
    while((DMAC_REGS->DMAC_CHCTRLA & DMAC_CHCTRLA_ENABLE_Msk) != 0U)
    {
        /* Wait for channel to be disabled */
    }

    /* Set the new settings */
    dmacDescReg[channel].DMAC_BTCTRL = (uint16_t)settings;

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;

    return true;
}


void DMAC_InterruptHandler(void) {
    DMAC_TRANSFER_EVENT event = DMAC_TRANSFER_EVENT_ERROR;

    /* Get active channel number */
    uint8_t channel = (uint8_t)((uint32_t)DMAC_REGS->DMAC_INTPEND & DMAC_INTPEND_ID_Msk);

    DMAC_CH_OBJECT  *dmacChObj = (DMAC_CH_OBJECT *)&dmacChannelObj[channel];

    /* Save channel ID */
    uint8_t channelId = (uint8_t)DMAC_REGS->DMAC_CHID;

    /* Update the DMAC channel ID */
    DMAC_REGS->DMAC_CHID = channel;

    /* Get the DMAC channel interrupt status */
    volatile uint32_t chanIntFlagStatus = (uint8_t)DMAC_REGS->DMAC_CHINTFLAG;

    /* Verify if DMAC Channel Transfer complete flag is set */
    if ((chanIntFlagStatus & DMAC_CHINTENCLR_TCMPL_Msk) == DMAC_CHINTENCLR_TCMPL_Msk) {
        /* Clear the transfer complete flag */
        DMAC_REGS->DMAC_CHINTFLAG = (uint8_t)DMAC_CHINTENCLR_TCMPL_Msk;
        event = DMAC_TRANSFER_EVENT_COMPLETE;
        dmacChObj->busyStatus = false;
    }

    /* Verify if DMAC Channel Error flag is set */
    if ((chanIntFlagStatus & DMAC_CHINTENCLR_TERR_Msk) == DMAC_CHINTENCLR_TERR_Msk) {
        /* Clear transfer error flag */
        DMAC_REGS->DMAC_CHINTFLAG = (uint8_t)DMAC_CHINTENCLR_TERR_Msk;
        event = DMAC_TRANSFER_EVENT_ERROR;
        dmacChObj->busyStatus = false;
    }

    /* Execute the callback function */
    if (dmacChObj->callback != NULL) {
        dmacChObj->callback (event, dmacChObj->context);
    }

    /* Restore channel ID */
    DMAC_REGS->DMAC_CHID = channelId;
}
