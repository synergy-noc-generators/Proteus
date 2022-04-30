/*
Description: This is the testbench for the top function Ring.cpp

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (yangyuchen@gatech.edu)
*/

#include <iostream>
#include <fstream>
#include <cmath>

#include "common.h"
#include "Router.h"
#include "VN.h"

using namespace std;

void torus( 
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
    
    int deadlock_cycles = 100000;
    int num_packets_per_node = 2000;
    int routing_algorithm = XY;
    int traffic_pattern = BIT_COMPLEMENT;
    int total_packets_sent = 0;
    int total_packets_recieved = 0;
    long total_latency = 0;
    int overall_max_latency = 0;
    int num_node_deadlock_detected = 0;
    for(int traffic_pattern = 1 ; traffic_pattern < 5;traffic_pattern++)
    {
//     traffic_pattern = 2;
    int packet_inject_period = 1;
    for (;packet_inject_period<=7;packet_inject_period++)
    { 
    torus (
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

//     std::cout << "Injection Rate: " << packet_inject_period << std::endl;
//     std::cout << "Total packets recieved: " << total_packets_recieved << std::endl;
//     std::cout << "Total packets sent: " << total_packets_sent << std::endl;
//     std::cout << "Average latency: " << (float)total_latency/(num_packets_per_node*NUM_NODES) << std::endl;
    std::cout << (float)total_latency/(num_packets_per_node*NUM_NODES) << ",";
//     std::cout << "Max latency: " << overall_max_latency << std::endl;
//     std::cout << "Number of nodes detect deadlock: " << num_node_deadlock_detected << std::endl;
    
    }
    std::cout << std::endl;
}
    std::cout << " \n Simulation SUCCESSFUL!!!" << std::endl;
//         std::cout << "Simulation FAILED :(" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    return 0;
}
