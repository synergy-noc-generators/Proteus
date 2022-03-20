#ifndef __COMMON_H__
#define __COMMON_H__

#include "VN.h"
#include <ap_int.h>

typedef ap_int<16> INT16;

struct Packet {
    bool valid;
    INT16 cycle_time;
    INT16 source;
    INT16 dest;
};

#endif