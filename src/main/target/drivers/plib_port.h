#ifndef PLIB_PORT_H
#define PLIB_PORT_H

#include "pic32cm6408mc00032.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif
    
#define PORT_GROUP_0 (PORT_BASE_ADDRESS + (0U * 0x80U))
#define PORT_GROUP_1 (PORT_BASE_ADDRESS + (1U * 0x80U))

/* Helper macros to get port information from the pin */
#define GET_PORT_GROUP(pin)  ((PORT_GROUP)(PORT_BASE_ADDRESS + (0x80U * (((uint32_t)(pin)) >> 5U))))
#define GET_PIN_MASK(pin)   (((uint32_t)(0x1U)) << (((uint32_t)(pin)) & 0x1FU))

/* Named type for port group */
typedef uint32_t PORT_GROUP;

typedef enum {
    PERIPHERAL_FUNCTION_A = 0x0,
    PERIPHERAL_FUNCTION_B = 0x1,
    PERIPHERAL_FUNCTION_C = 0x2,
    PERIPHERAL_FUNCTION_D = 0x3,
    PERIPHERAL_FUNCTION_E = 0x4,
    PERIPHERAL_FUNCTION_F = 0x5,
    PERIPHERAL_FUNCTION_G = 0x6,
    PERIPHERAL_FUNCTION_H = 0x7,
    PERIPHERAL_FUNCTION_I = 0x8,
    PERIPHERAL_FUNCTION_J = 0x9,
} PERIPHERAL_FUNCTION;

typedef enum {
    PORT_PIN_PA00 = 0U,
    PORT_PIN_PA01 = 1U,
    PORT_PIN_PA02 = 2U,
    PORT_PIN_PA03 = 3U,
    PORT_PIN_PA04 = 4U,
    PORT_PIN_PA05 = 5U,
    PORT_PIN_PA06 = 6U,
    PORT_PIN_PA07 = 7U,
    PORT_PIN_PA08 = 8U,
    PORT_PIN_PA09 = 9U,
    PORT_PIN_PA10 = 10U,
    PORT_PIN_PA11 = 11U,
    PORT_PIN_PA12 = 12U,
    PORT_PIN_PA13 = 13U,
    PORT_PIN_PA14 = 14U,
    PORT_PIN_PA15 = 15U,
    PORT_PIN_PA16 = 16U,
    PORT_PIN_PA17 = 17U,
    PORT_PIN_PA18 = 18U,
    PORT_PIN_PA19 = 19U,
    PORT_PIN_PA20 = 20U,
    PORT_PIN_PA21 = 21U,
    PORT_PIN_PA22 = 22U,
    PORT_PIN_PA23 = 23U,
    PORT_PIN_PA24 = 24U,
    PORT_PIN_PA25 = 25U,
    PORT_PIN_PA27 = 27U,
    PORT_PIN_PA28 = 28U,
    PORT_PIN_PA30 = 30U,
    PORT_PIN_PA31 = 31U,
    PORT_PIN_PB02 = 34U,
    PORT_PIN_PB03 = 35U,
    PORT_PIN_PB08 = 40U,
    PORT_PIN_PB09 = 41U,
    PORT_PIN_PB10 = 42U,
    PORT_PIN_PB11 = 43U,
    PORT_PIN_PB22 = 54U,
    PORT_PIN_PB23 = 55U,

    /* This element should not be used in any of the PORT APIs.
     * It will be used by other modules or application to denote that none of
     * the PORT Pin is used */
    PORT_PIN_NONE = 65535U,

} PORT_PIN;

void PORT_Initialize(void);
uint32_t PORT_GroupRead(PORT_GROUP group);
uint32_t PORT_GroupLatchRead(PORT_GROUP group);
void PORT_GroupWrite(PORT_GROUP group, uint32_t mask, uint32_t value);
void PORT_GroupSet(PORT_GROUP group, uint32_t mask);
void PORT_GroupClear(PORT_GROUP group, uint32_t mask);
void PORT_GroupToggle(PORT_GROUP group, uint32_t mask);
void PORT_GroupInputEnable(PORT_GROUP group, uint32_t mask);
void PORT_GroupOutputEnable(PORT_GROUP group, uint32_t mask);
void PORT_PinPeripheralFunctionConfig(PORT_PIN pin, PERIPHERAL_FUNCTION function);
void PORT_PinGPIOConfig(PORT_PIN pin);
void PORT_Pullup(PORT_PIN pin);

static inline void PORT_PinWrite(PORT_PIN pin, bool value) {
    if(pin != PORT_PIN_NONE)
        PORT_GroupWrite(GET_PORT_GROUP(pin),
                        GET_PIN_MASK(pin),
                        (value ? GET_PIN_MASK(pin) : 0U));
}

static inline bool PORT_PinRead(PORT_PIN pin) {
    return ((PORT_GroupRead(GET_PORT_GROUP(pin)) & GET_PIN_MASK(pin)) != 0U);
}

static inline bool PORT_PinLatchRead(PORT_PIN pin) {
    return ((PORT_GroupLatchRead(GET_PORT_GROUP(pin)) & GET_PIN_MASK(pin)) != 0U);
}

static inline void PORT_PinToggle(PORT_PIN pin) {
    if(pin != PORT_PIN_NONE)
        PORT_GroupToggle(GET_PORT_GROUP(pin), GET_PIN_MASK(pin));
}

static inline void PORT_PinSet(PORT_PIN pin) {
    if(pin != PORT_PIN_NONE)
        PORT_GroupSet(GET_PORT_GROUP(pin), GET_PIN_MASK(pin));
}

static inline void PORT_PinClear(PORT_PIN pin) {
    if(pin != PORT_PIN_NONE)
        PORT_GroupClear(GET_PORT_GROUP(pin), GET_PIN_MASK(pin));
}

static inline void PORT_PinInputEnable(PORT_PIN pin) {
    if(pin != PORT_PIN_NONE)
        PORT_GroupInputEnable(GET_PORT_GROUP(pin), GET_PIN_MASK(pin));
}

/*
    Enables selected IO pin as output.
    Calling this function will override the MHC input output pin configuration.
*/
static inline void PORT_PinOutputEnable(PORT_PIN pin) {
    if(pin != PORT_PIN_NONE)
        PORT_GroupOutputEnable(GET_PORT_GROUP(pin), GET_PIN_MASK(pin));
}

#ifdef __cplusplus
}
#endif
#endif // PLIB_PORT_H
