/*
Description: This is the router microarchitecture and logic class

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (yangyuchen@gatech.edu)
*/

#include "Router.h"
#include "VN.h"
#include "common.h"
#include <stdio.h>
// #define DEBUG
Router::Router() {

}

Router::Router(int router_id, int routing_algorithm, int traffic_pattern) {
    this->router_id = router_id;
    this->buffer_threshold = 1;
    this->packet_wait_generate = 0;
    this->packets_recieved = 0;
    this->packets_sent = 0;
    this->latency_add_up = 0;
    this->max_latency = 0;
    this->routing_algorithm = routing_algorithm;
    this->traffic_pattern = traffic_pattern;
    this->backpressure = false;
    this->deadlock = false;
}

void Router::deadlock_check(int packet_idle_cycle, VN vn) {
    if (vn.deadlock_check(packet_idle_cycle)) {
#ifdef DEBUG
        std::cout << "Possible Deadlock Detected in node "<< this->router_id << ", current idle cycle for this packet = "<< packet_idle_cycle  << std::endl;
#endif
        this->deadlock = true;
    }

}

void Router::deadlock_check_all(VN vn) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        deadlock_check(packet_idle_cycle_east[i],vn);
        deadlock_check(packet_idle_cycle_west[i],vn);
        deadlock_check(packet_idle_cycle_local[i],vn);
    }
}

int Router::get_num_valid_buffer(Packet buffers[BUFFER_SIZE]) {
    int count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (!buffers[i].valid) {
            count++;
        }
    }
    return count;
}

int Router::find_empty_buffer(Packet buffers[BUFFER_SIZE]) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (!buffers[i].valid) {
            return i;
        }
    }
    return ERROR;
}

// the direction of the selected packets should not affect the arbitration of input
int Router::find_oldest_packet(Packet buffers[BUFFER_SIZE]) { 
    int location = ERROR;
    int timestamp_least = ERROR;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffers[i].valid) {
            if (timestamp_least == ERROR || buffers[i].timestamp < timestamp_least) {
                timestamp_least = buffers[i].timestamp;
                location = i;
            }
        }
    }
    
    return location;
}

INT16 Router::dest_compute() { 
    int num_destinations = NUM_NODES;
    int radix = NUM_COLS;
    int dest_x = -1;
    int dest_y = -1;
    int source = this->router_id;
    int src_x = this->router_id % NUM_COLS;
    int src_y = this->router_id / NUM_COLS;

    if (this->traffic_pattern == BIT_COMPLEMENT) {
        return NUM_NODES - this->router_id - 1;
    } 
    else if (this->traffic_pattern == TRANSPOSE) {
        return src_x * NUM_COLS + src_y; // reverse the x and y position
    } 
    else if (this->traffic_pattern == BIT_REVERSE) {
        unsigned int straight = source;
        unsigned int reverse = source & 1; // LSB
        int num_bits = (int) log2(num_destinations);
        for (int i = 1; i < num_bits; i++)
        {
            reverse <<= 1;
            straight >>= 1;
            reverse |= (straight & 1); // LSB
        }
        return reverse;
    } 
    else if (this->traffic_pattern == BIT_ROTATION) {
        if (source%2 == 0)
            return source/2;
        else // (source%2 == 1)
            return ((source/2) + (num_destinations/2));
    } 
    else if (this->traffic_pattern == NEIGHBOR) {
            dest_x = (src_x + 1) % NUM_COLS;
            dest_y = src_y;
            return dest_y*NUM_COLS + dest_x;
    } 
    else if (this->traffic_pattern == SHUFFLE) {
        if (source < num_destinations/2)
            return source*2;
        else
            return (source*2 - num_destinations + 1);
    }
    return ERROR;
}

void Router::packet_add_to_queue(VN vn) {
    if (vn.packet_if_send()) {
        this->packet_wait_generate += 1; // we do not record the cycle here, only calculate latency once the package enter the network
    }
}

// try to generate packets and put into local buffers
void Router::packet_produce(VN vn) {
    int location = find_empty_buffer(this->buffer_local);
    if (location != ERROR && this->packet_wait_generate > 0) {
        this->buffer_local[location].valid = true;
        this->buffer_local[location].source = this->router_id;
        this->buffer_local[location].timestamp = vn.get_current_cycle();
        this->buffer_local[location].dest = dest_compute();
        this->packets_sent++;
        this->packet_wait_generate--;
#ifdef DEBUG
        std::cout << "packet generated in node "<< this->router_id << " At time :"<<this->buffer_local[location].timestamp << " Going to: " << this->buffer_local[location].dest << std::endl;
#endif
    }
    

}

void Router::packet_consume(Packet packet, VN vn) {
    this->packets_recieved++;
    int latency = vn.get_current_cycle() - packet.timestamp;
    this->max_latency = latency > max_latency ? latency : max_latency;
    this->latency_add_up += latency;
}

void Router::packet_idle_cycle_update() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer_north[i].valid) {
            packet_idle_cycle_north[i] += 1;
        }

        if (buffer_east[i].valid) {
            packet_idle_cycle_east[i] += 1;
        }

        if (buffer_west[i].valid) {
            packet_idle_cycle_west[i] += 1;
        }

        if (buffer_south[i].valid) {
            packet_idle_cycle_south[i] += 1;
        }

        if (buffer_local[i].valid) {
            packet_idle_cycle_local[i] += 1;
        }

    }
}

bool Router::Buffer_Write(Packet packet, int buffer_location) {
    if (!packet.valid) {
        return true;
    }

    if (buffer_location == LOCAL) {
        return false;
    } 
    else if (buffer_location == EAST) {
        int location = find_empty_buffer(this->buffer_east);
        if (location == ERROR) {
            return false;
        }

#ifdef DEBUG 
         std::cout << " Writing to buffer east at location " << location << " , the packet V: "<< packet.valid << " timestamp " << packet.timestamp << " source " << packet.source << " dest " << packet.dest << std::endl ;
#endif
        buffer_east[location] = packet;
        return true;
    } 
    else if (buffer_location == WEST) {
        int location = find_empty_buffer(this->buffer_west);
        if (location == ERROR) {
            return false;
        }
#ifdef DEBUG 
         std::cout << " Writing to buffer west at location " << location << " , the packet V: "<< packet.valid << " timestamp " << packet.timestamp << " source " << packet.source << " dest " << packet.dest << std::endl ;
#endif
        buffer_west[location] = packet;
        return true;
    }
    else if (buffer_location == NORTH) {
        int location = find_empty_buffer(this->buffer_north);
        if (location == ERROR) {
            return false;
        }

#ifdef DEBUG 
         std::cout << " Writing to buffer north at location " << location << " , the packet V: "<< packet.valid << " timestamp " << packet.timestamp << " source " << packet.source << " dest " << packet.dest << std::endl ;
#endif
        buffer_north[location] = packet;
        return true;
    }

    else if (buffer_location == SOUTH) {
        int location = find_empty_buffer(this->buffer_south);
        if (location == ERROR) {
            return false;
        }

#ifdef DEBUG 
         std::cout << " Writing to buffer south at location " << location << " , the packet V: "<< packet.valid << " timestamp " << packet.timestamp << " source " << packet.source << " dest " << packet.dest << std::endl ;
#endif
        buffer_south[location] = packet;
        return true;
    }
    return false;
}

// Refer to the random oblivious routing implementation in Garnet
int Router::Route_Compute_random_oblivious(INT16 dst_id, int input_port, bool random_counter) {
    if (dst_id == this->router_id) {
        return EVICT;
    }

    int my_x = this->router_id % NUM_COLS;
    int my_y = this->router_id / NUM_COLS;

    int dst_x = dst_id % NUM_COLS;
    int dst_y = dst_id / NUM_COLS;

//     int x_hops = std::abs(dst_x - my_x);
//     int y_hops = std::abs(dst_y - my_y);
    int x_hops = (dst_x - my_x) >= 0 ? dst_x - my_x : my_x - dst_x;
    int y_hops = (dst_y - my_y) >= 0 ? dst_y - my_y : my_y - dst_y;

    bool x_dirn = (dst_x >= my_x);
    bool y_dirn = (dst_y >= my_y);

    int direction = ERROR;
    if (x_hops == 0) {
        if (y_dirn > 0) {
            direction = SOUTH;
        } else {
            direction = NORTH;
        }
    } else if (y_hops == 0) {
        if (x_dirn > 0) {
            direction = EAST;
        } else {
            direction = WEST;
        }
    } else {
        if (x_dirn && y_dirn) { // Quadrant I
            direction = random_counter ? EAST : NORTH;
        } else if (!x_dirn && y_dirn) { // Quadrant II
            direction = random_counter ? WEST : NORTH;
        } else if (!x_dirn && !y_dirn) { // Quadrant III
            direction = random_counter ? WEST : SOUTH;
        } else { // Quadrant IV
            direction = random_counter ? EAST : SOUTH;
        }
    }

    if (direction == input_port) {
        return ERROR;
    }

    return direction;
}

int Router::Route_Compute_XY(INT16 dst_id, int input_port) {
    if (dst_id == this->router_id) {
        return EVICT;
    }

    int my_x = this->router_id % NUM_COLS;
    int my_y = this->router_id / NUM_COLS;

    int dst_x = dst_id % NUM_COLS;
    int dst_y = dst_id / NUM_COLS;

//     int x_hops = abs(dst_x - my_x);
//     int y_hops = abs(dst_y - my_y);
int x_hops = (dst_x - my_x) >= 0 ? dst_x - my_x : my_x - dst_x;
int y_hops = (dst_y - my_y) >= 0 ? dst_y - my_y : my_y - dst_y;

    bool x_dirn = (dst_x >= my_x);
    bool y_dirn = (dst_y >= my_y);

    int direction = ERROR;
    if (x_hops > 0) {
        if (x_dirn) {
            direction = EAST;
        } else {
            direction = WEST;
        }
    } else if (y_hops > 0) {
        if (y_dirn) {
            direction = SOUTH;
        } else {
            direction = NORTH;
        }
    } else {
        direction = ERROR;
    }

    if (direction == input_port) {
        return ERROR;
    }

    return direction;
}


INT16 Router::Output_Compute(INT16 dst_id, int input_port) {
    static bool random_counter = false; // fake random number decider: false -> east, true -> west
    random_counter = !random_counter;
    if (this->routing_algorithm == RANDOM_OBLIVIOUS) { // random_oblivious
        return Route_Compute_random_oblivious(dst_id, input_port, random_counter);
    } else if (this->routing_algorithm == XY) {
        return Route_Compute_XY(dst_id, input_port);
    }

    return ERROR;
}

void Router::Router_Compute() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer_north[i].valid) {
    		north_route_info[i] = Output_Compute(buffer_north[i].dest, NORTH);
    	}

    	if (buffer_east[i].valid) {
    		east_route_info[i] = Output_Compute(buffer_east[i].dest, EAST);
//     		printf("east route info %d: %d \n", i, (int)east_route_info[i]);
    	}

    	if (buffer_west[i].valid) {
    		west_route_info[i] = Output_Compute(buffer_west[i].dest, WEST);
    	}
        
        if (buffer_south[i].valid) {
    		south_route_info[i] = Output_Compute(buffer_south[i].dest, SOUTH);
    	}

    	if (buffer_local[i].valid) {
    		local_route_info[i] = Output_Compute(buffer_local[i].dest, LOCAL);
    		//printf("local route info %d: %d \n", i, (int)local_route_info[i]);
    	}

    }
}

// this->switch allocator will also done buffer_read and return the packet for Link Traversal
Packet Router::Switch_Allocator(INT16 backpressure, int output_port) {
    Packet ret;
    ret.valid = false;
    if (backpressure) {
        return ret;
    }

    int candidate_pool[5] = {0,0,0,0,0}; // each location represent one direction: N, E, W, S
    int timestamp_pool[5] = {0,0,0,0,0}; 

    candidate_pool[NORTH] = find_oldest_packet(this->buffer_north);
    timestamp_pool[NORTH] = buffer_north[candidate_pool[NORTH]].timestamp;

    candidate_pool[EAST] = find_oldest_packet(this->buffer_east);
    timestamp_pool[EAST] = buffer_east[candidate_pool[EAST]].timestamp;

    candidate_pool[WEST] = find_oldest_packet(this->buffer_west);
    timestamp_pool[WEST] = buffer_east[candidate_pool[WEST]].timestamp;

    candidate_pool[SOUTH] = find_oldest_packet(this->buffer_south);
    timestamp_pool[SOUTH] = buffer_south[candidate_pool[SOUTH]].timestamp;

    candidate_pool[NORTH] = north_route_info[candidate_pool[NORTH]] == output_port ? candidate_pool[NORTH] : ERROR;
    candidate_pool[EAST] = east_route_info[candidate_pool[EAST]] == output_port ? candidate_pool[EAST] : ERROR;
    candidate_pool[WEST] = west_route_info[candidate_pool[WEST]] == output_port ? candidate_pool[WEST] : ERROR;
    candidate_pool[SOUTH] = south_route_info[candidate_pool[SOUTH]] == output_port ? candidate_pool[SOUTH] : ERROR;

    int final_candidate = ERROR;
    int least_candidate_timestamp = ERROR;
    for (int i = 1; i < 5; i++) {
        if (candidate_pool[i] != ERROR) {
            if (final_candidate == ERROR) {
                final_candidate = i;
                least_candidate_timestamp = timestamp_pool[i];
            } else {
                if (timestamp_pool[i] < least_candidate_timestamp) {
                    final_candidate = i;
                    least_candidate_timestamp = timestamp_pool[i]; 
                }
            }
        }
    }

    if (final_candidate == ERROR) {
        int location = find_oldest_packet(this->buffer_local);
        if (local_route_info[location] == output_port) {
            packet_idle_cycle_local[location] = 0;
            ret = buffer_local[location];
            buffer_local[location].valid = false;
        }
    } else {
        int location = candidate_pool[final_candidate];
        if (final_candidate == NORTH) {
            packet_idle_cycle_north[location] = 0;
            ret = buffer_north[location];
            buffer_north[location].valid = false;
        } else if (final_candidate == EAST) {
            packet_idle_cycle_east[location] = 0;
            ret = buffer_east[location];
            buffer_east[location].valid = false;
        } else if (final_candidate == WEST) {
            packet_idle_cycle_west[location] = 0;
            ret = buffer_west[location];
            buffer_west[location].valid = false;
        } else {
            packet_idle_cycle_south[location] = 0;
            ret = buffer_south[location];
            buffer_south[location].valid = false;
        }
    }

    return ret;
}

// have a function has parameter of the buffer write data, then another function for different output port packet return for link traversal
void Router::router_phase_one(Packet north_input, Packet east_input, Packet west_input, Packet south_input, VN vn) {
    // deadlock check
    deadlock_check_all(vn);

    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer_north[i].valid && north_route_info[i] == EVICT) {
            packet_consume(buffer_north[i], vn);
            buffer_north[i].valid = false;
            packet_idle_cycle_north[i] = 0;
        }


        if (buffer_east[i].valid && east_route_info[i] == EVICT) {
            packet_consume(buffer_east[i], vn);
            buffer_east[i].valid = false;
            packet_idle_cycle_east[i] = 0;
        }

        if (buffer_west[i].valid && west_route_info[i] == EVICT) {
            packet_consume(buffer_west[i], vn);
            buffer_west[i].valid = false;
            packet_idle_cycle_west[i] = 0;
        }

        if (buffer_south[i].valid && south_route_info[i] == EVICT) {
            packet_consume(buffer_south[i], vn);
            buffer_south[i].valid = false;
            packet_idle_cycle_south[i] = 0;
        }

        if (buffer_local[i].valid && local_route_info[i] == EVICT) {
            packet_consume(buffer_local[i], vn);
            buffer_local[i].valid = false;
            packet_idle_cycle_local[i] = 0;
        }
    }

    // local new packet produce
    packet_add_to_queue(vn);
    packet_produce(vn);


    // Writes from the links to the buffer in the node.
    bool north_op = Buffer_Write(north_input, NORTH);
    bool east_op = Buffer_Write(east_input, EAST);
    bool west_op = Buffer_Write(west_input, WEST);
    bool south_op = Buffer_Write(south_input, SOUTH);

    if (!north_op || !east_op || !west_op || !south_op) {
#ifdef DEBUG
        std::cout << "Something goes wrong with Buffer Write" << std::endl;
#endif
        return;
    }
    packet_idle_cycle_update();

    Router_Compute();
}

// send packets from buffer to links
Packet Router::router_phase_two(INT16 backpressure, int output_dirn) {
    // switch allocation, buffer read, return the packet
    Packet ret = Switch_Allocator(backpressure, output_dirn);
#ifdef DEBUG
    printf("Packet going %d: %d %d %d %d \n", output_dirn, ret.valid, (int)ret.timestamp, (int)ret.source, (int)ret.dest);
#endif
    return ret;
}

INT16 Router::on_off_switch_update(int input_port) {
    if (input_port == EAST) {
        this->backpressure = get_num_valid_buffer(buffer_east) <= this->buffer_threshold;
        return this->backpressure;
    } else if (input_port == WEST) {
        this->backpressure = get_num_valid_buffer(buffer_west) <= this->buffer_threshold;
        return this->backpressure;
    } else if (input_port == NORTH) {
        this->backpressure = get_num_valid_buffer(buffer_north) <= this->buffer_threshold;
        return this->backpressure;
    } else if (input_port == SOUTH) {
        this->backpressure = get_num_valid_buffer(buffer_south) <= this->buffer_threshold;
        return this->backpressure;
    }

    return false;
}

int Router::get_packets_sent(){
    return this->packets_sent;
}

int Router::get_packets_recieved() {
	return this->packets_recieved;
}

int Router::get_max_latency() {
    return this->max_latency;
}

int Router::get_added_latency() {
    return this->latency_add_up;
}

int Router::get_deadlock_info() {
    if (this->deadlock) {
        return 1;
    } else {
        return 0;
    }
}
