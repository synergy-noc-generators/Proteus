#ifndef __ROUTER_H__
#define __ROUTER_H__

#include "common.h"

class Router {
    int router_id;      //Self ID
    int num_node;       //Total Number of nodes in the ring
    int buffer_threshold;
    int buffer_size; // should be >= threshold since we are using on-off switch for now, assume 1 cycle of passing the information now

    Packet buffer_east[buffer_size];
    INT16 east_route_info[buffer_size];
    INT16 packet_idle_cycle_east[buffer_size];

    Packet buffer_west[buffer_size];
    INT16 west_route_info[buffer_size];
    INT16 packet_idle_cycle_west[buffer_size];
    
    Packet buffer_local[buffer_size];
    INT16 local_route_info[buffer_size];
    INT16 packet_idle_cycle_local[buffer_size];

    int packet_wait_generate; // packets waiting for generating
    int packets_recieved;
    int packets_sent;
    int latency_add_up; // do the average at the end for stats display
    int max_latency;
    int routing_algorithm;
    int traffic_pattern;

    bool backpressure;
    bool deadlock;
}


public:
    Router(int router_id, int buffer_size, int routing_algorithm, int traffic_pattern, int num_node);
    void deadlock_check(int packet_idle_cycle);
    int get_num_valid_buffer(Packet buffers);
    int find_valid_buffer(Packet buffers);
    INT16 dest_compute();                   //Calculates the packet destination based on network traffic pattern.
    void packet_add_to_queue(VN vn);
    void packet_produce(VN vn);
    void Router::router_phase_one(Packet east_input, Packet west_input, VN vn);
    Packet Router::router_phase_two(INT16 output_port_on_off, int output_dirn);
    INT16 Router::on_off_switch_update(int input_port);
}


};








#endif
