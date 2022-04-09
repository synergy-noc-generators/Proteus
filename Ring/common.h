#ifndef __COMMON_H__
#define __COMMON_H__

#include "VN.h"
#include <ap_int.h>

typedef ap_int<16> INT16;

#define BUFFER_SIZE  4
#define NUM_NODES  8
#define DATA_WIDTH  22       //16+2log2(NUM_NODES)   = 16 +2*3 
#define ERROR -1
#define LOCAL 0
#define EAST 1
#define WEST 2
#define EVICT 3


struct Packet {
    bool valid;
    INT16 timestamp;
    INT16 source;
    INT16 dest;
    
    Packet()
    {
        valid = 0;
        timestamp = 0;
        source =0;
        dest =0;
    };
};

#endif
