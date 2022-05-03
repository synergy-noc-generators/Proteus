/*
Description: This is the header file providing common used Macro and struct

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (ychen940@gatech.edu)
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include "VN.h"
#include <ap_int.h>

typedef ap_int<16> INT16;

#define BUFFER_SIZE    4

#define NODES_PER_ROW  8
#define NUM_NODES      NODES_PER_ROW*NODES_PER_ROW
#define NUM_NODES_LOG2 log2(NUM_NODES) // log2 can only be used in C level simulation, need to change it to hard code number when sythesis
#define NUM_COLS       NODES_PER_ROW
#define NUM_ROWS       NODES_PER_ROW
#define DATA_WIDTH     22       //16+2log2(NUM_NODES)   = 16 +2*3 
#define ERROR          -1

#define NORTH          4
#define EAST           1
#define WEST           2
#define SOUTH          3
#define LOCAL          0
#define EVICT          5

#define RANDOM_OBLIVIOUS  0
#define XY                1


// Traffic patterns
#define BIT_COMPLEMENT 0
#define TRANSPOSE      1
#define BIT_REVERSE    2
#define BIT_ROTATION   3
#define SHUFFLE        4
#define NEIGHBOR       5

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
