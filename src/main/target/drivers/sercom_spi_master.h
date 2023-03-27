#ifndef PLIB_SERCOM_SPI_MASTER_H
#define PLIB_SERCOM_SPI_MASTER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <device.h>
#include "sercom_spi_master.h"
#include "plib_port.h"

#ifdef __cplusplus // Provide C++ Compatibility
extern "C" {
#endif

typedef void (*SERCOM_SPI_CALLBACK)(void);

typedef struct {
    void *                   txBuffer;
    void *                   rxBuffer;
    size_t                   txSize;
    size_t                   rxSize;
    size_t                   dummySize;
    size_t                   rxCount;
    size_t                   txCount;
    bool                     transferIsBusy;
    SERCOM_SPI_CALLBACK      callback;
    uint32_t                 status;
    PORT_PIN                 cs_line;
    uint32_t                 baud;
} SPI_OBJECT;

// *****************************************************************************
/* Function:
    void SERCOM_SPI_Initialize (void);

  Summary:
    Initializes instance SERCOM1 operating in SPI mode.

  Description:
    This function initializes instance SERCOM1 operating in SPI mode.
    This function should be called before any other library function. The SERCOM
    module will be configured as per the MHC settings.

  Precondition:
    MCC GUI should be configured with the right values. The Generic Clock
    configuration and the SERCOM Peripheral Clock channel should have been
    configured in the clock manager GUI.The function will itself enable the
    required peripheral clock channel and main clock.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
        SERCOM_SPI_Initialize();
    </code>

  Remarks:
    This function must be called once before any other SPI function is called.
*/

void SERCOM_SPI_Initialize (sercom_registers_t* sercom);



// *****************************************************************************
/* Function:
    bool SERCOM_SPI_WriteRead (void* pTransmitData, size_t txSize
                                        void* pReceiveData, size_t rxSize);

  Summary:
    Write and Read data on SERCOM SERCOM1 SPI peripheral.

  Description:
    This function transmits "txSize" number of bytes and receives "rxSize"
    number of bytes on SERCOM SERCOM1 SPI module. Data pointed by pTransmitData is
    transmitted and received data is saved in the location pointed by
    pReceiveData. The function will transfer the maximum of "txSize" or "rxSize"
    data units towards completion.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    transferring all the data.  This indicates that the operation has been
    completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    transmit and receive buffer  are ownerd by the library until the data
    transfer is complete and should not be modified by the application till the
    transfer is complete.  Only one transfer is allowed at any time. The
    Application can use a callback function or a polling function to check for
    completion of the transfer. If a callback is required, this should be
    registered prior to calling the SERCOM_SPI_WriteRead() function. The
    application can use the SERCOM_SPI_IsBusy() to poll for completion.

  Precondition:
    The SERCOM_SPI_Initialize function must have been called.  If the
    peripheral instance has been configured for Interrupt mode and transfer
    completion status needs to be communicated back to application via callback,
    a callback should have been registered using SERCOM_SPI_CallbackRegister()
    function.

  Parameters:
    pTransmitData - Pointer to the data which has to be transmitted. In a case
    where only data reception is required, this pointer can be set to NULL. If
    the module is configured for 9 bit data length, the data should be right
    aligned in a 16 bit memory location. The size of this buffer should be
    txSize.

    txSize - Number of bytes to be transmitted. For 9 but data length, a count
    of 1 counts 2 bytes. This value can be different from rxSize.

    pReceiveData - Pointer to the location where the received data has to be
    stored.  It is user's responsibility to ensure that this location has
    sufficient memory to store rxSize amount of data. In a case where only data
    transmission is required, this pointer can be set to NULL.  If the module is
    configured for 9 bit data length, received data will be right aligned and
    will be stored in a 16 bit memory location.

    rxSize - Number of bytes to be received. This value can be different from
    txSize. For 9 bit data length, a size count of 1 indicates 2 bytes required
    to store 9 bits of data.

  Returns:
    true - If configured for Non-interrupt mode, the function has recevied and
    transmitted the requested number of bytes. If configured for Interrupt mode,
    the request was accepted successfully and will be processed in the
    interrupt.

    false - If both pTransmitData and pReceiveData are NULL, or if both txSize
    and rxSize are 0 or if txSize is non-zero but the pTransmitData is set to
    NULL or rxSize is non-zero but pReceiveData is NULL. In Interrupt mode, the
    function returns false if there is an on-going data transfer at the time of
    calling the function.

  Example:
    <code>

     The following code snippet shows an example using the
     SERCOM_SPI_WriteRead() function in interrupt mode operation using the
     callback function.

    uint8_t     txBuffer[4];
    uint8_t     rxBuffer[10];
    size_t      txSize = 4;
    size_t      rxSize = 10;

    void APP_SPITransferHandler(uintptr_t context)
    {
       Transfer was completed without error, do something else now.
    }

    SERCOM_SPI_Initialize();
    SERCOM_SPI_CallbackRegister(&APP_SPITransferHandler, (uintptr_t)NULL);
    if(SERCOM_SPI_WriteRead(&txBuffer, txSize, &rxBuffer, rxSize))
    {
        request got accepted
    }
    else
    {
        request didn't get accepted, try again later with correct arguments
    }
    The following code snippet shows non-interrupt or blocking mode
    operation.

    uint8_t txBuffer[4];
    uint8_t rxBuffer[10];
    size_t txSize = 4;
    size_t rxSize = 10;

    SERCOM_SPI_Initialize();

    This function call will block.
    SERCOM_SPI_WriteRead(&txBuffer, txSize, &rxBuffer, rxSize);

    </code>

  Remarks:
    None.
*/

bool SERCOM_SPI_WriteRead(sercom_registers_t* sercom, PORT_PIN cs_line, void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize);

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_Write(void* pTransmitData, size_t txSize);

  Summary:
    Writes data to SERCOM SERCOM1 SPI peripheral.

  Description:
    This function writes "txSize" number of bytes on SERCOM SERCOM1 SPI module. Data
    pointed by pTransmitData is transmitted.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    transferring all the data.  This indicates that the operation has been
    completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    transmit buffer  is ownerd by the library until the data transfer is
    complete and should not be modified by the application till the transfer is
    complete.  Only one transfer is allowed at any time. The application can use
    a callback function or a polling function to check for completion of the
    transfer. If a callback is required, this should be registered prior to
    calling the SERCOM_SPI_WriteRead() function. The application can use the
    SERCOM_SPI_IsBusy() to poll for completion.

  Precondition:
    The SERCOM_SPI_Initialize function must have been called.

    Callback has to be registered using SERCOM_SPI_CallbackRegister API if the
    peripheral instance has been configured in Interrupt mode and
    transfer completion status needs to be communicated back to application via
    callback.

  Parameters:
    pTransmitData - Pointer to the buffer containing the data which has to be
    transmitted.  For 9 bit mode, data should be right aligned in the 16 bit
    memory location. In "Interrupt Mode", this buffer should not be modified
    after calling the function and before the callback function has been called
    or the SERCOM_SPI_IsBusy() function returns false.

    txSize - Number of bytes to be transmitted. For 9 bit mode, 2 bytes make up
    a count of 1.

  Returns:
    true - If configured for Non-interrupt mode, the function has transmitted
    the requested number of bytes. If configured for Interrupt mode, the request
    was accepted successfully and will be processed in the interrupt.

    false - If pTransmitData is NULL. In Interrupt mode, the function will
    additionally return false if there is an on-going data transfer at the time
    of calling the function.

  Example:
    <code>
    uint8_t txBuffer[4];
    size_t txSize = 4;

    void APP_SPITransferHandler(uintptr_t context)
    {
        Transfer was completed without error, do something else now.
    }

    SERCOM_SPI_Initialize();
    SERCOM_SPI_CallbackRegister(&APP_SPITransferHandler, (uintptr_t)NULL);
    if(SERCOM_SPI_Write(&txBuffer, txSize))
    {
        request got accepted
    }
    else
    {
        request didn't get accepted, try again later with correct arguments
    }

    </code>

  Remarks:
    None.

*/

bool SERCOM_SPI_Write(sercom_registers_t* sercom, void* pTransmitData, size_t txSize);

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_Read(void* pReceiveData, size_t rxSize);

  Summary:
    Reads data on the SERCOM SERCOM1 SPI peripheral.

  Description:
    This function reads "rxSize" number of bytes on SERCOM SERCOM1 SPI module. The
    received data is stored in the buffer pointed by pReceiveData.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    receiving "rxSize" number of bytes.  This indicates that the operation has
    been completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    receive buffer  is ownerd by the library until the data transfer is
    complete and should not be modified by the application till the transfer is
    complete.  Only one transfer is allowed at any time. The application can use
    a callback function or a polling function to check for completion of the
    transfer. If a callback is required, this should be registered prior to
    calling the SERCOM_SPI_WriteRead() function. The application can use the
    SERCOM_SPI_IsBusy() to poll for completion.

  Precondition:
    The SERCOM_SPI_Initialize function must have been called.

    Callback has to be registered using SERCOM_SPI_CallbackRegister API if the
    peripheral instance has been configured in Interrupt mode and
    transfer completion status needs to be communicated back to application via
    callback.

  Parameters:
    pReceiveData - Pointer to the buffer where the received data will be stored.
    For 9 bit mode, data should be right aligned in the 16 bit memory location.
    In "Interrupt Mode", this buffer should not be modified after calling the
    function and before the callback function has been called or the
    SERCOM_SPI_IsBusy() function returns false.

    rxSize - Number of bytes to be received. For 9 bit mode, 2 bytes make up a
    count of 1.

  Returns:
    true - If configured for Non-interrupt mode, the function has received the
    requested number of bytes. If configured for Interrupt mode, the request was
    accepted successfully and will be processed in the interrupt.

    false - If pReceiveData is NULL. In Interrupt mode, the function will
    additionally return false if there is an on-going data transfer at the time
    of calling the function.

  Example:
    <code>
    uint8_t     rxBuffer[10];
    size_t      rxSize = 10;

    void APP_SPITransferHandler(uintptr_t context)
    {
        Transfer was completed without error, do something else now.
    }

    SERCOM_SPI_Initialize();
    SERCOM_SPI_CallbackRegister(&APP_SPITransferHandler, (uintptr_t)NULL);
    if(SERCOM_SPI_Read(&rxBuffer, rxSize))
    {
        request got accepted
    }
    else
    {
        request didn't get accepted, try again later with correct arguments
    }
    </code>

  Remarks:
    None.
*/

bool SERCOM_SPI_Read(sercom_registers_t* sercom, void* pReceiveData, size_t rxSize);


void SERCOM_SPI_CallbackRegister(sercom_registers_t* sercom, SERCOM_SPI_CALLBACK callBack);

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_IsBusy (void);

  Summary:
    Returns transfer status of SERCOM SERCOM1SPI.

  Description:
    This function ture if the SERCOM SERCOM1SPI module is busy with a transfer. The
    application can use the function to check if SERCOM SERCOM1SPI module is busy
    before calling any of the data transfer functions. The library does not
    allow a data transfer operation if another transfer operation is already in
    progress. This function returns true when the SPI PLIB software state machine is idle and
	all the bytes are transmitted out on the bus (shift register is empty).

    This function can be used as an alternative to the callback function when
    the library is operating interrupt mode. The allow the application to
    implement a synchronous interface to the library.

  Precondition:
    The SERCOM_SPI_Initialize() should have been called once. The module should
    have been configured for interrupt mode operation in MHC.

  Parameters:
    None.

  Returns:
    true -  Transfer is still in progress
    false - Transfer is completed or no transfer is currently in progress.

  Example:
    <code>
        The following code example demonstrates the use of the
        SERCOM_SPI_IsBusy() function. This example shows a blocking while
        loop. The function can also be called periodically.

        uint8_t dataBuffer[20];

        SERCOM_SPI_Initialize();
        SERCOM_SPI_Write(dataBuffer, 20);

        while (SERCOM_SPI_IsBusy() == true)
        {
            Wait here till the transfer is done.
        }
    </code>

  Remarks:
    None.
*/

bool SERCOM_SPI_IsBusy (sercom_registers_t* sercom);

// *****************************************************************************
/* Function:
    bool SERCOM_SPI_IsTransmitterBusy (void);

  Summary:
    Returns hardware transfer status of the SPI transmit shift register

  Description:
    This function returns the hardware status of the transmit shift register. 
	The status is  returned true after all the bytes have been shifted out on the 
	SPI bus. This function should be used when using DMA with SPI PLIB to make 
	sure that all the bytes have been transmitted out on the bus. For SPI 
	transfers without DMA, the SERCOM_SPI_IsBusy() API must be used.

  Precondition:
    The SERCOM_SPI_Initialize() should have been called once.

  Parameters:
    None.

  Returns:
    true -  Data is being shifted out on the SPI bus
    false - All the data bytes have been shifted out on the SPI bus

  Example:
    <code>
        
    </code>

  Remarks:
    None.
*/
bool SERCOM_SPI_IsTransmitterBusy(sercom_registers_t* sercom);


#ifdef __cplusplus // Provide C++ Compatibility
}
#endif

#endif /* PLIB_SERCOM_SPI_MASTER_H */