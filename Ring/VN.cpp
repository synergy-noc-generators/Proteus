#include "VN.h"


VN::VN(int num_node) {
    deadlock_threshold = 1000;
    packets_per_node = 10;
    inject_rate = 10;    // the unit here is cycle
    routing_algorithm = 0; // every number will represent a algorithm, 0 is for random oblivious
    traffic_pattern = 0; // every number will represent a traffic pattern, 0 is for bit_compliement
    current_cycle = 0;
    this->num_node = num_node;
}

VN::VN(int deadlock_threshold, int packets_per_node, int inject_rate, int routing_algorithm, int traffic_pattern, int num_node) {
    this.deadlock_threshold = deadlock_threshold;
    this.packets_per_node = packets_per_node;
    this.inject_rate = inject_rate;    // the unit here is cycle
    this.routing_algorithm = routing_algorithm; // every number will represent a algorithm, 0 is for random oblivious
    this.traffic_pattern = traffic_pattern; // every number will represent a traffic pattern, 0 is for bit_compliement
    this->num_node = num_node;
    current_cycle = 0;
}

bool VN::deadlock_check(int idle_cycle) {
    return idle_cycle >= this.deadlock_threshold;
}

bool VN::packet_if_send() {
    if (this.current_cycle >= this.packets_per_node * this.inject_rate) {
        return false;
    }

    return this.current_cycle % this.inject_rate == 0;
}

int VN::get_current_cycle() {
    return this.current_cycle;
}

int VN::get_num_node() {
    return this.num_node;
}

int VN::get_routing_algorithm() {
    return this.routing_algorithm;
}

int VN::get_traffic_pattern() {
    return this.traffic_pattern;
}
