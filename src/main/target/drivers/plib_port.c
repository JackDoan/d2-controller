#include "plib_port.h"

void PORT_Initialize(void) {
    PORT_PinOutputEnable(EN1);
    PORT_PinOutputEnable(EN2);
    PORT_PinOutputEnable(EN3);
    PORT_PinOutputEnable(EN4);
    motors_set_enable(false);

    PORT_PinOutputEnable(DIR1);
    PORT_PinOutputEnable(DIR2);
    PORT_PinOutputEnable(DIR3);
    PORT_PinOutputEnable(DIR4);
    PORT_PinClear(DIR1);
    PORT_PinClear(DIR2);
    PORT_PinClear(DIR3);
    PORT_PinClear(DIR4);


    PORT_PinOutputEnable(CS1);
    PORT_PinOutputEnable(CS2);
    PORT_PinOutputEnable(CS3);
    PORT_PinOutputEnable(CS4);
    PORT_PinSet(CS1);
    PORT_PinSet(CS2);
    PORT_PinSet(CS3);
    PORT_PinSet(CS4);

    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA07, PERIPHERAL_FUNCTION_C); //AIN[7] for vbatt

//    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA08, PERIPHERAL_FUNCTION_C); //RX Sercom0 Pad 0 //todo switch these?
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA09, PERIPHERAL_FUNCTION_C); //RX Sercom0 Pad 1 --actually hooked up

    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA16, PERIPHERAL_FUNCTION_C); //FTDI Sercom1 Pad 0
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA17, PERIPHERAL_FUNCTION_C); //FTDI Sercom1 Pad 1

    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA10, PERIPHERAL_FUNCTION_F); //pwm1 TCC0/ WO[2] = CC1
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA11, PERIPHERAL_FUNCTION_F); //pwm2 TCC0/ WO[3] = CC0
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA15, PERIPHERAL_FUNCTION_F); //pwm3 TCC0/ WO[5] = CC2
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA14, PERIPHERAL_FUNCTION_F); //pwm4 TCC0/ WO[4] = CC3

    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA22, PERIPHERAL_FUNCTION_C); //mosi Sercom3 pad0
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA23, PERIPHERAL_FUNCTION_C); //sck  Sercom3 pad1
    PORT_PinPeripheralFunctionConfig(PORT_PIN_PA25, PERIPHERAL_FUNCTION_C); //miso Sercom3 pad3
}
// *****************************************************************************
/* Function:
    uint32_t PORT_GroupRead(PORT_GROUP group)

  Summary:
    Read all the I/O pins in the specified port group.

  Description:
    The function reads the hardware pin state of all pins in the specified group
    and returns this as a 32 bit value. Each bit in the 32 bit value represent a
    pin. For example, bit 0 in group 0 will represent pin PA0. Bit 1 will
    represent PA1 and so on. The application should only consider the value of
    the port group pins which are implemented on the device.

  Remarks:
    Refer plib_port.h file for more information.
*/

uint32_t PORT_GroupRead(PORT_GROUP group)
{
    return (((port_group_registers_t*)group)->PORT_IN);
}

// *****************************************************************************
/* Function:
    void PORT_GroupWrite(PORT_GROUP group, uint32_t mask, uint32_t value);

  Summary:
    Write value on the masked pins of the selected port group.

  Description:
    This function writes the value contained in the value parameter to the
    port group. Port group pins which are configured for output will be updated.
    The mask parameter provides additional control on the bits in the group to
    be affected. Setting a bit to 1 in the mask will cause the corresponding
    bit in the port group to be updated. Clearing a bit in the mask will cause
    that corresponding bit in the group to stay unaffected. For example,
    setting a mask value 0xFFFFFFFF will cause all bits in the port group
    to be updated. Setting a value 0x3 will only cause port group bit 0 and
    bit 1 to be updated.

    For port pins which are not configured for output and have the pull feature
    enabled, this function will affect pull value (pull up or pull down). A bit
    value of 1 will enable the pull up. A bit value of 0 will enable the pull
    down.

  Remarks:
    Refer plib_port.h file for more information.
*/

void PORT_GroupWrite(PORT_GROUP group, uint32_t mask, uint32_t value)
{
    /* Write the desired value */
    ((port_group_registers_t*)group)->PORT_OUT = (((port_group_registers_t*)group)->PORT_OUT & (~mask)) | (mask & value);
}

// *****************************************************************************
/* Function:
    uint32_t PORT_GroupLatchRead(PORT_GROUP group)

  Summary:
    Read the data driven on all the I/O pins of the selected port group.

  Description:
    The function will return a 32-bit value representing the logic levels being
    driven on the output pins within the group. The function will not sample the
    actual hardware state of the output pin. Each bit in the 32-bit return value
    will represent one of the 32 port pins within the group. The application
    should only consider the value of the pins which are available on the
    device.

  Remarks:
    Refer plib_port.h file for more information.
*/

uint32_t PORT_GroupLatchRead(PORT_GROUP group)
{
    return (((port_group_registers_t*)group)->PORT_OUT);
}

// *****************************************************************************
/* Function:
    void PORT_GroupSet(PORT_GROUP group, uint32_t mask)

  Summary:
    Set the selected IO pins of a group.

  Description:
    This function sets (drives a logic high) on the selected output pins of a
    group. The mask parameter control the pins to be updated. A mask bit
    position with a value 1 will cause that corresponding port pin to be set. A
    mask bit position with a value 0 will cause the corresponding port pin to
    stay un-affected.

  Remarks:
    Refer plib_port.h file for more information.
*/

void PORT_GroupSet(PORT_GROUP group, uint32_t mask)
{
    ((port_group_registers_t*)group)->PORT_OUTSET = mask;
}

// *****************************************************************************
/* Function:
    void PORT_GroupClear(PORT_GROUP group, uint32_t mask)

  Summary:
    Clears the selected IO pins of a group.

  Description:
    This function clears (drives a logic 0) on the selected output pins of a
    group. The mask parameter control the pins to be updated. A mask bit
    position with a value 1 will cause that corresponding port pin to be clear.
    A mask bit position with a value 0 will cause the corresponding port pin to
    stay un-affected.

  Remarks:
    Refer plib_port.h file for more information.
*/

void PORT_GroupClear(PORT_GROUP group, uint32_t mask)
{
    ((port_group_registers_t*)group)->PORT_OUTCLR = mask;
}

// *****************************************************************************
/* Function:
    void PORT_GroupToggle(PORT_GROUP group, uint32_t mask)

  Summary:
    Toggles the selected IO pins of a group.

  Description:
    This function toggles the selected output pins of a group. The mask
    parameter control the pins to be updated. A mask bit position with a value 1
    will cause that corresponding port pin to be toggled.  A mask bit position
    with a value 0 will cause the corresponding port pin to stay un-affected.

  Remarks:
    Refer plib_port.h file for more information.
*/

void PORT_GroupToggle(PORT_GROUP group, uint32_t mask)
{
    ((port_group_registers_t*)group)->PORT_OUTTGL = mask;
}

// *****************************************************************************
/* Function:
    void PORT_GroupInputEnable(PORT_GROUP group, uint32_t mask)

  Summary:
    Configures the selected IO pins of a group as input.

  Description:
    This function configures the selected IO pins of a group as input. The pins
    to be configured as input are selected by setting the corresponding bits in
    the mask parameter to 1.

  Remarks:
    Refer plib_port.h file for more information.
*/

void PORT_GroupInputEnable(PORT_GROUP group, uint32_t mask)
{
    ((port_group_registers_t*)group)->PORT_DIRCLR = mask;
}

// *****************************************************************************
/* Function:
    void PORT_GroupOutputEnable(PORT_GROUP group, uint32_t mask)

  Summary:
    Configures the selected IO pins of a group as output.

  Description:
    This function configures the selected IO pins of a group as output. The pins
    to be configured as output are selected by setting the corresponding bits in
    the mask parameter to 1.

  Remarks:
    Refer plib_port.h file for more information.
*/

void PORT_GroupOutputEnable(PORT_GROUP group, uint32_t mask)
{
   ((port_group_registers_t*)group)->PORT_DIRSET = mask;
}

// *****************************************************************************
/* Function:
    void PORT_PinPeripheralFunctionConfig(PORT_PIN pin, PERIPHERAL_FUNCTION function)

  Summary:
    Configures the peripheral function on the selected port pin

  Description:
    This function configures the selected peripheral function on the given port pin.

  Remarks:
    Refer plib_port.h file for more information.
*/
void PORT_PinPeripheralFunctionConfig(PORT_PIN pin, PERIPHERAL_FUNCTION function)
{
    uint32_t periph_func = (uint32_t) function;
    PORT_GROUP group = GET_PORT_GROUP(pin);
    uint32_t pin_num = ((uint32_t)pin) & 0x1FU;
    uint32_t pinmux_val = (uint32_t)((port_group_registers_t*)group)->PORT_PMUX[(pin_num >> 1)];

    /* For odd pins */
    if (0U != (pin_num & 0x01U))
    {
        pinmux_val = (pinmux_val & ~0xF0U) | (periph_func << 4);
    }
    else
    {
        pinmux_val = (pinmux_val & ~0x0FU) | periph_func;
    }
    ((port_group_registers_t*)group)->PORT_PMUX[(pin_num >> 1)] = (uint8_t)pinmux_val;

    /* Enable peripheral control of the pin */
    ((port_group_registers_t*)group)->PORT_PINCFG[pin_num] |= (uint8_t)PORT_PINCFG_PMUXEN_Msk;
}

// *****************************************************************************
/* Function:
    void PORT_PinGPIOConfig(PORT_PIN pin)

  Summary:
    Configures the selected pin as GPIO

  Description:
    This function configures the given pin as GPIO.

  Remarks:
    Refer plib_port.h file for more information.
*/
void PORT_PinGPIOConfig(PORT_PIN pin)
{
    PORT_GROUP group = GET_PORT_GROUP(pin);
    uint32_t pin_num = ((uint32_t)pin) & 0x1FU;

    /* Disable peripheral control of the pin */
    ((port_group_registers_t*)group)->PORT_PINCFG[pin_num] &= ((uint8_t)(~PORT_PINCFG_PMUXEN_Msk));
}