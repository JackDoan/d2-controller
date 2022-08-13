//#include <stddef.h>
//#include "device.h"
//#include "sercom_usart.h"
//#include "plib_dmac.h"
//#include <stdbool.h>
//#include <stddef.h>
//
//int read(int handle, void *buffer, unsigned int len) {
//    bool success = false;
//    if(len <= 0) {
//        return 0;
//    }
//    switch (handle) {
//        case (int)(void*)FTDI:
//            //todo this doesn't block
//            success = DMAC_ChannelTransfer(DMAC_CHANNEL_1, (const void *)&FTDI->USART_INT.SERCOM_DATA, buffer, len);
//            break;
//        default:
//            success = false;
//            break;
//    }
//    if(success) {
//        return (int)len;
//    }
//    return 0;
//}
//
//int write(int handle, void * buffer, size_t count)
//{
//    bool success = false;
//    if (handle == 1)
//    {
//        do
//        {
//            success = false; //todo
//        }while( !success);
//    }
//    return count;
//}