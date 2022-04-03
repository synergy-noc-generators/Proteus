#include "Router.h"
#include "VN.h"
#include "common.h"
#include <stdio.h>

void ring( 
        INT16       deadlock_cycles,
        INT16       num_packets_per_node,
        INT16       num_packets_sent[NUM_NODES],
        INT16       num_packets_recieved[NUM_NODES],
        INT16       avg_latency[NUM_NODES], 
        INT16       max_latency[NUM_NODES], 
        ap_unit     deadlock_detected[NUM_NODES],
        )
{
#pragma HLS interface s_axilite register port=return
#pragma HLS interface s_axilite register port=deadlock_cycles
#pragma HLS interface s_axilite register port=num_packets_sent
#pragma HLS interface s_axilite register port=num_packets_per_node
#pragma HLS interface s_axilite register port=num_packets_recieved
#pragma HLS interface s_axilite register port=avg_latency
#pragma HLS interface s_axilite register port=max_latency
#pragma HLS interface s_axilite register port=deadlock_detected

    static Router node[NUM_NODES]; 

    Packet link_east[NUM_NODES]; // MODIFICATION: instead of the datawith define, we can make it packet granularity wire
    Packet link_west[NUM_NODES];

    INT16 onoff_switch_east[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
    INT16 onoff_switch_west[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
    int buffer_size = 4;

    node[0] = Router(0,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node,
                    link_east[0],link_west[0], link_east[NUM_NODES-1], link_west[1]); // THINK: maybe the router level do not need to know the id of next, Ring level is good enough. The router provide the data when needed, and ring allocate it to the right place
    for (int i = 1 ; i < NUM_NODES -1; i++)
    {
        node[i] = Router(i,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node,
                    link_east[i],link_west[i],link_east[i-1],link_west[i+1]);
    }
        node[NUM_NODES-1] = Router(NUM_NODES-1,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node
                    link_east[NUM_NODES-1],link_west[NUM_NODES-1],link_east[NUM_NODES-2],link_west[0]);

    while(total_packets_recieved < num_packet_per_node*NUM_NODES)
    {
        node[i].start_router();
    }
}
