#ifndef __ROUTER_H__
#define __ROUTER_H__

#include "common.h"

class Router {
    int router_id;
    int num_node;
    int buffer_threshold;
    int buffer_size; // should be >= threshold since we are using on-off switch for now, assume 1 cycle of passing the information now
    Packet buffer_east[buffer_size];
    Packet buffer_west[buffer_size];
    Packet buffer_local[buffer_size];
    int packet_wait_generate; // packets waiting for generating
    int packets_recieved;
    int packets_sent;
    int latency_add_up; // do the average at the end for stats display
    int routing_algorithm;
    int traffic_pattern;

    bool backpressure;
    bool deadlock;
}


public:
    Router(int router_id, int buffer_size, int routing_algorithm, int traffic_pattern, int num_node, );
    void deadlock_check(int packet_idle_cycle);
    int get_num_valid_buffer(Packet buffers);
    int find_valid_buffer(Packet buffers);
    INT16 dest_compute();
    void packet_add_to_queue(VN vn);
    void packet_produce(VN vn);
}


};








#endif
