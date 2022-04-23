/*
Description: This is the header file providing common used Macro and struct

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (yangyuchen@gatech.edu)
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include "VN.h"
#include <ap_int.h>

typedef ap_int<16> INT16;

#define BUFFER_SIZE  4
#define NUM_NODES  8
#define DATA_WIDTH  22       //16+2log2(NUM_NODES)   = 16 +2*3 
#define ERROR -1
#define EAST 1
#define WEST 2
#define LOCAL 3
#define EVICT 4

#define BIT_COMPLEMENT 0
#define SHUFFLE 1

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
