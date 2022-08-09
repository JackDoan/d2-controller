#include "device.h"
#include "plib_nvic.h"

void NVIC_Initialize( void ) {
    /* Enable NVIC Controller */
    __DMB();
    __enable_irq();

    /* Enable the interrupt sources and configure the priorities as configured
     * from within the "Interrupt Manager" of MHC. */
    NVIC_SetPriority(SERCOM3_IRQn, 3); //SPI
    NVIC_EnableIRQ(SERCOM3_IRQn);
    NVIC_SetPriority(SERCOM1_IRQn, 3); //FTDI
    NVIC_EnableIRQ(SERCOM1_IRQn);
    NVIC_SetPriority(SERCOM0_IRQn, 3); //RX
    NVIC_EnableIRQ(SERCOM0_IRQn);
}

void NVIC_INT_Enable( void )
{
    __DMB();
    __enable_irq();
}

bool NVIC_INT_Disable( void )
{
    bool processorStatus = (__get_PRIMASK() == 0U);

    __disable_irq();
    __DMB();

    return processorStatus;
}

void NVIC_INT_Restore( bool state )
{
    if( state == true )
    {
        __DMB();
        __enable_irq();
    }
    else
    {
        __disable_irq();
        __DMB();
    }
}
