#include <iostream>
#include <fstream>
#include <cmath>

#include "common.h"
#include "Router.h"
#include "VN.h"

using namespace std;
void ring(int       deadlock_cycles, int       num_packets_per_node, int       packet_inject_period, int       num_packets_sent[NUM_NODES], int       num_packets_recieved[NUM_NODES], int       added_latency[NUM_NODES], int       max_latency[NUM_NODES], int     deadlock_detected[NUM_NODES]);
int main ()
{
    
int       deadlock_cycles=1000;
int       num_packets_per_node=200;
int       packet_inject_period=10;
int       num_packets_sent[NUM_NODES];
int       num_packets_recieved[NUM_NODES];
int       added_latency[NUM_NODES];
int       max_latency[NUM_NODES];
int       deadlock_detected[NUM_NODES];   
   
ring(
deadlock_cycles,
num_packets_per_node,
packet_inject_period,
num_packets_sent,
num_packets_recieved,
added_latency,
max_latency,
deadlock_detected
);
    
    int total_packets_recieved = 0;
    int total_packets_sent = 0;
    int total_latency = 0;
    int overall_max_latency = 0;
    int num_node_deadlock_detected = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        total_packets_recieved += num_packets_recieved[i];
        total_packets_sent += num_packets_sent[i];
        total_latency += added_latency[i];
        overall_max_latency = max_latency[i] > overall_max_latency ? max_latency[i] : overall_max_latency;
        num_node_deadlock_detected = deadlock_detected[i] != 0 ? num_node_deadlock_detected + 1 : num_node_deadlock_detected;
    }

    std::cout << "Total packets recieved: " << total_packets_recieved << std::endl;
    std::cout << "Total packets sent: " << total_packets_sent << std::endl;
    std::cout << "Average latency: " << total_latency/(num_packets_per_node*NUM_NODES) << std::endl;
    std::cout << "Max latency: " << overall_max_latency << std::endl;
    std::cout << "Number of nodes detect deadlock: " << num_node_deadlock_detected << std::endl;
    

    std::cout << "Simulation SUCCESSFUL!!!" << std::endl;
//         std::cout << "Simulation FAILED :(" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    return 0;
}
