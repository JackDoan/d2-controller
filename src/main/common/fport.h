#ifndef INAV_FPORT_H
#define INAV_FPORT_H

#include <stdint.h>
#include <stddef.h>

void proc_fport_rx();
void fport_trigger(size_t len);

#endif //INAV_FPORT_H
