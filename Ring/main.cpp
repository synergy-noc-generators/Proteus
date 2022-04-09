#include <iostream>
#include <fstream>
#include <cmath>

#include "common.h"
#include "Router.h"
#include "VN.h"

using namespace std;
void ring(INT16       deadlock_cycles, INT16       num_packets_per_node, INT16       inject_rate, INT16       num_packets_sent[NUM_NODES], INT16       num_packets_recieved[NUM_NODES], INT16       avg_latency[NUM_NODES], INT16       max_latency[NUM_NODES], INT16     deadlock_detected[NUM_NODES]);
int main ()
{
    
INT16       deadlock_cycles=10000;
INT16       num_packets_per_node=10;
INT16       inject_rate=1;
INT16       num_packets_sent[NUM_NODES];
INT16       num_packets_recieved[NUM_NODES];
INT16       avg_latency[NUM_NODES];
INT16       max_latency[NUM_NODES];
INT16       deadlock_detected[NUM_NODES];   
   
ring(
deadlock_cycles,
num_packets_per_node,
inject_rate,
num_packets_sent,
num_packets_recieved,
avg_latency,
max_latency,
deadlock_detected
);
    
    

    std::cout << "Simulation SUCCESSFUL!!!" << std::endl;
//         std::cout << "Simulation FAILED :(" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    return 0;
}
