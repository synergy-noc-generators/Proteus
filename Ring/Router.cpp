#include "Router.h"
#include "VN.h"
#include "common.h"
#include <stdio.h>


Router::Router(int router_id, int buffer_size, int routing_algorithm, int traffic_pattern, int num_node) {
    this.router_id = router_id;
    this.num_node = num_node;
    this.buffer_threshold = 1;
    this.buffer_size = 4;
    this.buffer_west = {}; // in_buffer, no out_buffer
    this.west_route_info = {};
    this.packet_idle_cycle_west = {};
    
    this.buffer_east = {}; // in_buffer, no out_buffer
    this.east_route_info = {};
    this.packet_idle_cycle_east = {};
    
    this.buffer_local = {};
    this.local_route_info = {};
    this.packet_idle_cycle_local = {};
    
    this.packet_wait_generate = 0;
    this.packets_recieved = 0;
    this.packets_sent = 0;
    this.latency_add_up = 0;
    this.max_latency = 0;
    this.routing_algorithm = routing_algorithm;
    this.traffic_pattern = traffic_pattern;
    this.backpressure = false;
    this.deadlock = false;
}

void Router::deadlock_check(int packet_idle_cycle) {
    if (VN::deadlock_check(packet_idle_cycle)) {
        std::cout << "Possible Deadlock Detected" << std::endl;
        this.deadlock = true;
    }
}

void Router::deadlock_check_all() {
    for (int i = 0; i < this.buffer_size; i++) {
        deadlock_check(packet_idle_cycle_east[i]);
        deadlock_check(packet_idle_cycle_west[i]);
        deadlock_check(packet_idle_cycle_local[i]);
    }
}

int Router::get_num_valid_buffer(Packet[] buffers) {
    int count = 0;
    for (int i = 0; i < this.buffer_size; i++) {
        if (!buffers[i].valid) {
            count++;
        }
    }
    return count;
}

int Router::find_empty_buffer(Packet[] buffers) {
    for (int i = 0; i < this.buffer_size; i++) {
        if (!buffers[i].valid) {
            return i;
        }
    }
    return ERROR;
}

// the direction of the selected packets should not affect the arbitration of input 
int Router::find_oldest_packet(Packet[] buffers) { 
    int location = ERROR;
    int timestamp_least = ERROR;
    for (int i = 0; i < this.buffer_size; i++) {
        if (buffers[i].valid) {
            if (timestamp_least == ERROR || buffers[i].timestamp < timestamp_least) {
                timestamp_least = buffers[i].timestamp;
                location = i;
            }
        }
    }
    return location;
}

INT16 Router::dest_compute() {      //For now assuming only 1 type of trafic pattern.
//     if (this.traffic_pattern == 0) { // bit_compliment
        return this.num_node - this.router_id - 1; // this is ring based
//     }
}

void Router::packet_add_to_queue(VN vn) {
    if (vn.packet_if_send()) {
        this.packet_wait_generate += 1; // we do not record the cycle here, only calculate latency once the package enter the network
    }
}

// try to generate packets and put into local buffers
void Router::packet_produce(VN vn) {
    int location = find_empty_buffer(this.buffer_local);
    if (location != ERROR && this.packet_wait_generate > 0) {
        this.buffer_local[location].valid = true;
        this.buffer_local[location].source = this.router_id;
        this.buffer_local[location].timestamp = vn.get_current_cycle();
        this.buffer_local[location].dest = dest_compute();
        this.packets_sent++;
        this.packet_wait_generate--;
    }
}

void Router::packet_consume(Packet packet, VN vn) {
    this.packets_recieved++;
    int latency = vn.get_current_cycle() - packet.timestamp;
    this.max_latency = latency > max_latency ? latency : max_latency;
    this.latency_add_up += latency;
}

void Router::packet_idle_cycle_update() {
    for (int i = 0; i < this.buffer_size; i++) {
        if (buffer_east[i].valid) {
            packet_idle_cycle_east[i] += 1;
        }

        if (buffer_west[i].valid) {
            packet_idle_cycle_west[i] += 1;
        }

        if (buffer_local[i].valid) {
            packet_idle_cycle_local[i] += 1;
        }

    }
}


// todo: packet recieve aka buffer write. Before do link traversal, we should make sure on-off switch is true. Otherwise the link should not be occupied
// return true if correct behavior detected, false otherwise
bool Router::Buffer_Write(Packet packet, int buffer_location) {
    if (!packet.valid) {
        return true;
    }

    if (buffer_location == LOCAL) {
        return false;
    } else if (buffer_location == EAST) {
        int location = find_empty_buffer(this.buffer_east);
        if (location == ERROR) {
            return false;
        }

        buffer_east[location] = packet;
        return true;
    } else if (buffer_location == WEST) {
        int location = find_empty_buffer(this.buffer_west);
        if (location == ERROR) {
            return false;
        }

        buffer_west[location] = packet;
        return true;
    }

    return false;
}


INT16 Router::Output_Compute(INT16 dst_id, int input_port) {
    static bool random_counter = false; // fake random number decider: false -> east, true -> west
    random_counter = !random_counter;
    if (this.routing_algorithm == 0) { // random_oblivious
        if (input_port == LOCAL) {
            int go_east_hop = 0;
            int go_west_hop = 0;
            if (dst_id == this.router_id) {
                return EVICT;
            } else if (dst_id < this.router_id) {       
                go_west_hop = this.router_id - dst_id;
                go_east_hop = (this.num_node - this.router_id) + dst_id;
            } else {
                go_west_hop = this.router_id + (this.num_node - dst_id);
                go_east_hop = this.router_id - dst_id;
            }

            if (go_east_hop == go_west_hop) {
                return random_counter ? WEST : EAST;
            } else {
                return go_east_hop > go_west_hop ? WEST : EAST;
            }
        } else if (input_port == EAST) {
            if (dst_id == this.router_id) {
                return EVICT;
            }
            return WEST;
        } else if (input_port == WEST) {
            if (dst_id == this.router_id) {
                return EVICT;
            }
            return EAST;
        } else {
            return ERROR;
        }
    }
}

void Router::Router_Compute() {
    for (int i = 0; i < this.buffer_size; i++) {
        east_route_info[i] = Output_Compute(buffer_east[i].dest, EAST);
        west_route_info[i] = Output_Compute(buffer_west[i].dest, WEST);
        local_route_info[i] = Output_Compute(buffer_local[i].dest, LOCAL);
    }
}

// This switch allocator will also done buffer_read and return the packet for Link Traversal
Packet Router::Switch_Allocator(INT16 output_port_on_off, int output_port) {
    // call packet_to_send for different ports and then arbite base on the cycle time of the packets?
    Packet ret;
    ret.valid = false;
    if (!output_port_on_off) {
        return ret;
    }

    if (output_port == WEST) {
        int location = find_oldest_packet(this.buffer_east);
        if (location == ERROR && east_route_info[location] == EVICT) {
            location = find_oldest_packet(this.buffer_local);
            if (local_route_info[location] == WEST) {
                packet_idle_cycle_local[location] = 0;
                ret = buffer_local[location];
                buffer_local[location].valid = false;
            }
            return ret;
        } else {
            packet_idle_cycle_east[location] = 0;
            ret = buffer_east[location];
            buffer_east[location].valid = false;
        }
    } else if (output_port == EAST) {
        int location = find_oldest_packet(this.buffer_west);
        if (location == ERROR && west_route_info[location] == EVICT) {
            location = find_oldest_packet(this.buffer_local);
            if (local_route_info[location] == EAST) {
                packet_idle_cycle_local[location] = 0;
                ret = buffer_local[location];
                buffer_local[location].valid = false;
            }
            return ret;
        } else {
            packet_idle_cycle_west[location] = 0;
            ret = buffer_west[location];
            buffer_west[location].valid = false;
        }
    }

    return ret;
}

// have a function has parameter of the buffer write data, then another function for different output port packet return for link traversal
void Router::router_phase_one(Packet east_input, Packet west_input, VN vn) {
    // deadlock check
    deadlock_check_all();

    // consume the packets with EVICT mark at the start?
    // TODO: What happens to packet with route_info as EAST or WEST.
    for (int i = 0; i < this.buffer_size; i++) {
        if (east_route_info[i] == EVICT) {
            packet_consume(buffer_east[i], vn);
            buffer_east[i].valid = false;
            packet_idle_cycle_east[i] = 0;
        }

        if (west_route_info[i] == EVICT) {
            packet_consume(buffer_west[i], vn);
            buffer_west[i].valid = false;
            packet_idle_cycle_west[i] = 0;
        }

        if (local_route_info[i] == EVICT) {
            packet_consume(buffer_local[i], vn);
            buffer_local[i].valid = false;
            packet_idle_cycle_local[i] = 0;
        }
    }

    // local new packet produce
    packet_add_to_queue(vn);
    packet_produce(vn);


    // Writes from the links to the buffer in the node.
    bool east_op = Buffer_Write(east_input, EAST);
    bool west_op = Buffer_Write(west_input, WEST);

    if (!east_op || !west_op) {
        std::cout << "Something goes wrong with Buffer Write" << std::endl;
        return;
    }

    Router_Compute();
}
// send packets from buffer to links
Packet Router::router_phase_two(INT16 output_port_on_off, int output_dirn) {
    // switch allocation, buffer read, return the packet
    Packet ret = Switch_Allocator(output_port_on_off, output_port);
    return ret;
}

// todo: on-off switch update after link traversal. Q: When to updaten backpressure
INT16 Router::on_off_switch_update(int input_port) {
    if (input_port == EAST) {
        this.backpressure = get_num_valid_buffer(buffer_east) <= this.buffer_threshold;
        return this.backpressure;
    } else if (input_port == WEST) {
        this.backpressure = get_num_valid_buffer(buffer_west) <= this.buffer_threshold;
        return this.backpressure;
    }

    return false;
}
