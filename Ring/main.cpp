/*
Description: This is the testbench for the top function Ring.cpp

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (ychen940@gatech.edu)
*/

#include <iostream>
#include <fstream>
#include <cmath>

#include "common.h"
#include "Router.h"
#include "VN.h"

using namespace std;
void ring( 
        int       deadlock_cycles,
        int       num_packets_per_node,
        int       packet_inject_period, // EX. =10, every 10 cycles, a packet will be generated at each node
        int       routing_algorithm, 
        int       traffic_pattern,
        int       &total_packets_sent,
        int       &total_packets_recieved,
        long      &total_latency, 
        int       &overall_max_latency, 
        int       &num_node_deadlock_detected
        );

int main ()
{
    
    int deadlock_cycles = 1000;
    int num_packets_per_node = 2000;
    int packet_inject_period = 1;
    int routing_algorithm = RANDOM_OBLIVIOUS;
    int traffic_pattern = SHUFFLE;
    int total_packets_sent = 0;
    int total_packets_recieved = 0;
    long total_latency = 0;
    int overall_max_latency = 0;
    int num_node_deadlock_detected = 0;
    
    ring (
        deadlock_cycles,
        num_packets_per_node,
        packet_inject_period,
        routing_algorithm,
        traffic_pattern,
        total_packets_sent,
        total_packets_recieved,
        total_latency,
        overall_max_latency,
        num_node_deadlock_detected
    );

    std::cout << "Total packets recieved: " << total_packets_recieved << std::endl;
    std::cout << "Total packets sent: " << total_packets_sent << std::endl;
    std::cout << "Average latency: " << (float)total_latency/(num_packets_per_node*NUM_NODES) << std::endl;
    std::cout << "Max latency: " << overall_max_latency << std::endl;
    std::cout << "Number of nodes detect deadlock: " << num_node_deadlock_detected << std::endl;
    

    std::cout << "Simulation SUCCESSFUL!!!" << std::endl;
//         std::cout << "Simulation FAILED :(" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    return 0;
}
