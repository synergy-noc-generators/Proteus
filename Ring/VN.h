#ifndef __VN_H__
#define __VN_H__


class VN {
    int deadlock_threshold;
    int packets_per_node;
    int inject_rate;
    int routing_algorithm;
    int traffic_pattern;
    int num_node;
    
    int current_cycle;
}

public:
    VN(int num_node);
    VN(int deadlock_threshold, int packets_per_node, int inject_rate, int routing_algorithm, int traffic_pattern, int num_node);
    bool deadlock_check(int idle_cycle);
    bool packet_if_send();
    int get_current_cycle();
    int get_num_node();
    int get_routing_algorithm();
    int get_traffic_pattern();
};


#endif
