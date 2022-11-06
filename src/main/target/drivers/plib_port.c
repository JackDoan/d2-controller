#include "plib_port.h"
#include "device.h"
void PORT_Initialize(void) {
    motors_init();

    PORT_PinOutputEnable(M1_OUT);
    PORT_PinOutputEnable(M2_OUT);

    PORT_PinOutputEnable(CS1);
    PORT_PinOutputEnable(CS2);
    PORT_PinOutputEnable(CS3);
    PORT_PinOutputEnable(CS4);
    PORT_PinSet(CS1);
    PORT_PinSet(CS2);
    PORT_PinSet(CS3);
    PORT_PinSet(CS4);

    PORT_PinInputEnable(FAULT);
    PORT_Pullup(FAULT);

    PORT_PinPeripheralFunctionConfig(VBATT_SENSE, PERIPHERAL_FUNCTION_C); //AIN[7] for vbatt

//    PORT_PinPeripheralFunctionConfig(RX_SERCOM_TX, PERIPHERAL_FUNCTION_C); //RX Sercom0 Pad 0 -- configured dynamically
//    PORT_PinPeripheralFunctionConfig(RX_SERCOM_RX, PERIPHERAL_FUNCTION_C); //RX Sercom0 Pad 1

    PORT_PinPeripheralFunctionConfig(FTDI_SERCOM_RXI, PERIPHERAL_FUNCTION_C);
    PORT_PinPeripheralFunctionConfig(FTDI_SERCOM_TXO, PERIPHERAL_FUNCTION_C);

//    PORT_PinPeripheralFunctionConfig(PIN_MOSI, PERIPHERAL_FUNCTION_C); //mosi Sercom3 pad0
//    PORT_PinPeripheralFunctionConfig(PIN_SCK, PERIPHERAL_FUNCTION_C); //sck  Sercom3 pad1
//    PORT_PinPeripheralFunctionConfig(PIN_MISO, PERIPHERAL_FUNCTION_C); //miso Sercom3 pad3
}

uint32_t PORT_GroupRead(PORT_GROUP group) {
    return (((port_group_registers_t*)group)->PORT_IN);
}

void PORT_GroupWrite(PORT_GROUP group, uint32_t mask, uint32_t value) {
    ((port_group_registers_t*)group)->PORT_OUT = (((port_group_registers_t*)group)->PORT_OUT & (~mask)) | (mask & value);
}

uint32_t PORT_GroupLatchRead(PORT_GROUP group) {
    return (((port_group_registers_t*)group)->PORT_OUT);
}

void PORT_GroupSet(PORT_GROUP group, uint32_t mask) {
    ((port_group_registers_t*)group)->PORT_OUTSET = mask;
}

void PORT_GroupClear(PORT_GROUP group, uint32_t mask) {
    ((port_group_registers_t*)group)->PORT_OUTCLR = mask;
}

void PORT_GroupToggle(PORT_GROUP group, uint32_t mask) {
    ((port_group_registers_t*)group)->PORT_OUTTGL = mask;
}

void PORT_GroupInputEnable(PORT_GROUP group, uint32_t mask) {
    ((port_group_registers_t*)group)->PORT_DIRCLR = mask;
}

void PORT_GroupOutputEnable(PORT_GROUP group, uint32_t mask) {
   ((port_group_registers_t*)group)->PORT_DIRSET = mask;
}

void PORT_PinPeripheralFunctionConfig(PORT_PIN pin, PERIPHERAL_FUNCTION function) {
    if(pin == PORT_PIN_NONE)
        return;
    PORT_GROUP group = GET_PORT_GROUP(pin);
    uint32_t pin_num = ((uint32_t)pin) & 0x1FU;
    uint32_t pinmux_val = (uint32_t)((port_group_registers_t*)group)->PORT_PMUX[(pin_num >> 1)];

    /* For odd pins */
    if (0U != (pin_num & 0x01U)) {
        pinmux_val = (pinmux_val & ~0xF0U) | (function << 4);
    }
    else {
        pinmux_val = (pinmux_val & ~0x0FU) | function;
    }
    ((port_group_registers_t*)group)->PORT_PMUX[(pin_num >> 1)] = (uint8_t)pinmux_val;

    /* Enable peripheral control of the pin */
    ((port_group_registers_t*)group)->PORT_PINCFG[pin_num] |= (uint8_t)PORT_PINCFG_PMUXEN_Msk;
}

void PORT_PinGPIOConfig(PORT_PIN pin) {
    PORT_GROUP group = GET_PORT_GROUP(pin);
    uint32_t pin_num = ((uint32_t)pin) & 0x1FU;

    /* Disable peripheral control of the pin */
    ((port_group_registers_t*)group)->PORT_PINCFG[pin_num] &= ((uint8_t)(~PORT_PINCFG_PMUXEN_Msk));
}

void PORT_Pullup(PORT_PIN pin) {
    PORT_GROUP group = GET_PORT_GROUP(pin);
    uint32_t pin_num = ((uint32_t)pin) & 0x1FU;

    /* Disable peripheral control of the pin */
    ((port_group_registers_t*)group)->PORT_PINCFG[pin_num] |= ((uint8_t)(PORT_PINCFG_PULLEN(true)));
}