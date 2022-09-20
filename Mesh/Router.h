/*
Description: This is the header file for router class

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (ychen940@gatech.edu)
*/

#ifndef __ROUTER_H__
#define __ROUTER_H__

#include "common.h"

class Router {
    int router_id;      //Self ID
    int buffer_threshold;

    Packet buffer_north[BUFFER_SIZE];
    INT16 north_route_info[BUFFER_SIZE] = {EVICT, EVICT, EVICT, EVICT};
    INT16 packet_idle_cycle_north[BUFFER_SIZE]= {0, 0, 0, 0};
// TODO do a for loop in the constructor to make this parameterizable
    Packet buffer_east[BUFFER_SIZE];
    INT16 east_route_info[BUFFER_SIZE] = {EVICT, EVICT, EVICT, EVICT};
    INT16 packet_idle_cycle_east[BUFFER_SIZE]=  {0, 0, 0, 0};

    Packet buffer_west[BUFFER_SIZE];
    INT16 west_route_info[BUFFER_SIZE] = {EVICT, EVICT, EVICT, EVICT};
    INT16 packet_idle_cycle_west[BUFFER_SIZE]=  {0, 0, 0, 0};

    Packet buffer_south[BUFFER_SIZE];
    INT16 south_route_info[BUFFER_SIZE] = {EVICT, EVICT, EVICT, EVICT};
    INT16 packet_idle_cycle_south[BUFFER_SIZE]=  {0, 0, 0, 0};
    
    Packet buffer_local[BUFFER_SIZE];
    INT16 local_route_info[BUFFER_SIZE] = {EVICT, EVICT, EVICT, EVICT};
    INT16 packet_idle_cycle_local[BUFFER_SIZE]=  {0, 0, 0, 0};

    int packet_wait_generate[NUM_NODES]; // packets waiting for generating
    int packet_wait_cycle;
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
    int Route_Compute_north_last(INT16 dst_id, int input_port, bool random_counter) ;
    int Route_Compute_west_first(INT16 dst_id, int input_port, bool random_counter) ;
    int Route_Compute_XY(INT16 dst_id, int input_port) ;

public:
    Router(int router_id,  int routing_algorithm, int traffic_pattern);
    Router();
    void deadlock_check(int packet_idle_cycle , VN vn);
    ap_uint<16> random_counter =49;
    ap_uint<16> random_counter_reset =49;
    int get_num_valid_buffer(Packet buffers[BUFFER_SIZE]);
    INT16 dest_compute();                   //Calculates the packet destination based on network traffic pattern.
    void packet_add_to_queue(VN vn,int random_lfsr);
    void packet_produce(VN vn,int random_lfsr);
    void router_phase_one(Packet north_input, Packet east_input, Packet west_input, Packet south_input, VN vn,int random_lfsr);
    Packet router_phase_two(INT16 output_port_on_off, int output_dirn);
    INT16 on_off_switch_update(int input_port);
    int get_packets_sent();
    int get_packets_recieved();
    int get_max_latency();
    int get_added_latency();
    int get_deadlock_info();
};











#endif
