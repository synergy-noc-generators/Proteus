#include "Router.h"
#include "VN.h"
#include "common.h"
#include <stdio.h>


Router::Router(int router_id, int buffer_size, int routing_algorithm, int traffic_pattern, int num_node) {
    this.router_id = router_id;
    this.num_node = num_node;
    this.buffer_threshold = 1;
    this.buffer_size = 4;
    this.buffer_west = {};
    this.buffer_east = {};
    this.buffer_local = {};
    this.packet_wait_generate = 0;
    this.packets_recieved = 0;
    this.packets_sent = 0;
    this.latency_add_up = 0;
    this.routing_algorithm = routing_algorithm;
    this.traffic_pattern = traffic_pattern;
    this.backpressure = false;
    this.deadlock = false;
    //static VN virtual_node(deadlock_threshold,packets_per_node,inject_rate,routing_algorithm,traffic_pattern, num_node);
}

void Router::deadlock_check(int packet_idle_cycle) {
    if (VN::deadlock_check(packet_idle_cycle)) {
        std::cout << "Possible Deadlock Detected" << std::endl;
        this.deadlock = true;
    }
}

int Router::get_num_valid_buffer(Packet buffers) {
    int count = 0;
    for (int i = 0; i < this.buffer_size; i++) {
        if (!buffers[i].valid) {
            count++;
        }
    }
    return count;
}

int Router::find_valid_buffer(Packet buffers) {
    for (int i = 0; i < this.buffer_size; i++) {
        if (!buffers[i].valid) {
            return i;
        }
    }
    return -1;
}

INT16 Router::dest_compute() {
    if (this.traffic_pattern == 0) { // bit_compliment
        return this.num_node - this.router_id - 1; // this is ring based
    }
}

void Router::packet_add_to_queue(VN vn) {
    if (vn.packet_if_send()) {
        this.packet_wait_generate += 1; // we do not record the cycle here, only calculate latency once the package enter the network
    }
}

// try to generate packets and put into local buffers
void Router::packet_produce(VN vn) {
    int location = find_valid_buffer(this.buffer_local);
    if (location != -1 && this.packet_wait_generate > 0) {
        this.buffer_local[i].valid = true;
        this.buffer_local[i].source = this.router_id;
        this.buffer_local[i].cycle_time = vn.get_current_cycle();
        this.buffer_local[i].dest = dest_compute();
        this.packets_sent++;
        this.packet_wait_generate--;
    }
}

// todo: need a switch allocator to arbite output port for parallel request from ports. Ex. west input port and local port both want to go east
Packet Router::Switch_Allocator() {
    // call packet_to_send for different ports and then arbite base on the cycle time of the packets?
}


// todo: packet send out, and how to access the "backpressure" signal from the next router, aka buffer read and link traversal
// Depending on the on-off, the top function decide call send_packet or not. the top function should return the packet to the top level: ring. 
// choose packets using an arbiter (based on cycle number for now?)
Packet Router::packet_to_send(int input_port) {

    
}


// todo: packet recieve aka buffer write. PS: when to write the data to the next router?


// todo: on-off switch update. The top function, ring, should call this for every router after processing in/out packets