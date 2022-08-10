#include "interrupts.h"
#include "plib_dmac.h"
#define DMAC_CRC_CHANNEL_OFFSET     0x20U
/*******************************************************************************
    This function Disables the CRC engine and clears the CRC Control register
********************************************************************************/
void DMAC_CRCDisable( void )
{
    DMAC_REGS->DMAC_CTRL &= (uint16_t)(~DMAC_CTRL_CRCENABLE_Msk);

    DMAC_REGS->DMAC_CRCCTRL = (uint16_t)DMAC_CRCCTRL_RESETVALUE;
}

/*******************************************************************************
    This function sets the CRC Engine to use DMAC channel for calculating CRC.

    This Function has to be called before submitting DMA transfer request for
    the channel to calculate CRC
********************************************************************************/

void DMAC_ChannelCRCSetup(DMAC_CHANNEL channel, DMAC_CRC_SETUP CRCSetup)
{
    /* Disable CRC Engine and clear the CRC Control register before configuring */
    DMAC_CRCDisable();

    DMAC_REGS->DMAC_CRCCHKSUM = CRCSetup.seed;

    /* Setup the CRC engine to use DMA Channel */
    DMAC_REGS->DMAC_CRCCTRL = (uint16_t)(DMAC_CRCCTRL_CRCPOLY((uint32_t)CRCSetup.polynomial_type) | DMAC_CRCCTRL_CRCSRC((DMAC_CRC_CHANNEL_OFFSET + (uint32_t)channel)));

    DMAC_REGS->DMAC_CTRL |= (uint16_t)DMAC_CTRL_CRCENABLE_Msk;
}

uint32_t DMAC_CRCRead(void) {
    return (DMAC_REGS->DMAC_CRCCHKSUM);
}


/*******************************************************************************
    This function sets the CRC Engine in IO mode to get the data using the CPU
    which will be written in CRCDATAIN register. It internally calculates the
    Beat Size to be used based on the buffer length.

    This function returns the final CRC value once the computation is done
********************************************************************************/
uint32_t DMAC_CRCCalculate(void *buffer, uint32_t length, DMAC_CRC_SETUP CRCSetup)
{
    uint8_t beatSize    = (uint8_t)DMAC_CRC_BEAT_SIZE_BYTE;
    uint32_t counter    = 0U;
    uint8_t *buffer_8   = buffer;
    uint16_t *buffer_16 = buffer;
    uint32_t *buffer_32 = buffer;

    /* Calculate the beatsize to be used basd on buffer length */
    if ((length & 0x3U) == 0U)
    {
        beatSize = (uint8_t)DMAC_CRC_BEAT_SIZE_WORD;
        length = length >> 0x2U;
    }
    else if ((length & 0x1U) == 0U)
    {
        beatSize = (uint8_t)DMAC_CRC_BEAT_SIZE_HWORD;
        length = length >> 0x1U;
    }
    else
    {
        /* Do nothing */
    }

    /* Disable CRC Engine and clear the CRC Control register before configuring */
    DMAC_CRCDisable();

    DMAC_REGS->DMAC_CRCCHKSUM = CRCSetup.seed;

    /* Setup the CRC engine to use IO Mode */
    DMAC_REGS->DMAC_CRCCTRL = (uint16_t)(DMAC_CRCCTRL_CRCPOLY((uint32_t)CRCSetup.polynomial_type) | DMAC_CRCCTRL_CRCBEATSIZE((uint32_t)beatSize) | DMAC_CRCCTRL_CRCSRC_IO );

    DMAC_REGS->DMAC_CTRL |= (uint16_t)DMAC_CTRL_CRCENABLE_Msk;

    /* Start the CRC calculation by writing the buffer into CRCDATAIN register based
     * on the beat size configured
     */
    for (counter = 0U; counter < length; counter++)
    {
        if (beatSize == (uint8_t)DMAC_CRC_BEAT_SIZE_BYTE)
        {
            DMAC_REGS->DMAC_CRCDATAIN = buffer_8[counter];
        }
        else if (beatSize == (uint8_t)DMAC_CRC_BEAT_SIZE_HWORD)
        {
            DMAC_REGS->DMAC_CRCDATAIN = buffer_16[counter];
        }
        else if (beatSize == (uint8_t)DMAC_CRC_BEAT_SIZE_WORD)
        {
            DMAC_REGS->DMAC_CRCDATAIN = buffer_32[counter];
        }
        else
        {
            /* Do nothing */
        }

        /* Wait until CRC Calculation is completed for the current data in CRCDATAIN */
        while ((DMAC_REGS->DMAC_CRCSTATUS & DMAC_CRCSTATUS_CRCBUSY_Msk) == 0U)
        {
            /* Do nothing */
        }

        /* Clear the busy bit */
        DMAC_REGS->DMAC_CRCSTATUS = (uint8_t)DMAC_CRCSTATUS_CRCBUSY_Msk;
    }

    /* Return the final CRC calculated for the entire buffer */
    return (DMAC_REGS->DMAC_CRCCHKSUM);
}

