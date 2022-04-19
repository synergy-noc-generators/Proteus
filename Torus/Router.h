/*
Description: This is the header file for router class

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (yangyuchen@gatech.edu)
*/

#ifndef __ROUTER_H__
#define __ROUTER_H__

#include "common.h"

class Router {
    int router_id;      //Self ID
    int buffer_threshold;

    Packet buffer_north[BUFFER_SIZE];
    INT16 north_route_info[BUFFER_SIZE] = {}; //{EVICT, EVICT, EVICT, EVICT};
    INT16 packet_idle_cycle_north[BUFFER_SIZE]= {}; //{0, 0, 0, 0};

    Packet buffer_east[BUFFER_SIZE];
    INT16 east_route_info[BUFFER_SIZE] = {};
    INT16 packet_idle_cycle_east[BUFFER_SIZE]= {};

    Packet buffer_west[BUFFER_SIZE];
    INT16 west_route_info[BUFFER_SIZE] = {};
    INT16 packet_idle_cycle_west[BUFFER_SIZE]= {};

    Packet buffer_south[BUFFER_SIZE];
    INT16 south_route_info[BUFFER_SIZE] = {};
    INT16 packet_idle_cycle_south[BUFFER_SIZE]= {};
    
    Packet buffer_local[BUFFER_SIZE];
    INT16 local_route_info[BUFFER_SIZE] = {};
    INT16 packet_idle_cycle_local[BUFFER_SIZE]= {};

    int packet_wait_generate; // packets waiting for generating
    int packets_recieved;
    int packets_sent;
    int latency_add_up; // do the average at the end for stats display
    int max_latency;
    int routing_algorithm;
    int traffic_pattern;

    bool backpressure;
    bool deadlock;

    Packet Switch_Allocator(INT16 output_port_on_off, int output_port);
    bool Buffer_Write(Packet packet, int buffer_location);
    void packet_consume(Packet packet, VN vn);
    void deadlock_check_all(VN vn);
    int find_empty_buffer(Packet buffers[BUFFER_SIZE]);
    int find_oldest_packet(Packet buffers[BUFFER_SIZE]); 
    void packet_idle_cycle_update();
    INT16 Output_Compute(INT16 dst_id, int input_port);

    void Router_Compute();
    int Route_Compute_random_oblivious(INT16 dst_id, int input_port, bool random_counter) ;
    int Route_Compute_XY(INT16 dst_id, int input_port, bool random_counter) ;

public:
    Router(int router_id,  int routing_algorithm, int traffic_pattern);
    Router();
    void deadlock_check(int packet_idle_cycle , VN vn);
    int get_num_valid_buffer(Packet buffers[BUFFER_SIZE]);
    INT16 dest_compute();                   //Calculates the packet destination based on network traffic pattern.
    void packet_add_to_queue(VN vn);
    void packet_produce(VN vn);
    void router_phase_one(Packet north_input, Packet east_input, Packet west_input, Packet south_input, VN vn);
    Packet router_phase_two(INT16 output_port_on_off, int output_dirn);
    INT16 on_off_switch_update(int input_port);
    int get_packets_sent();
    int get_packets_recieved();
    int get_max_latency();
    int get_added_latency();
    int get_deadlock_info();
};











#endif
