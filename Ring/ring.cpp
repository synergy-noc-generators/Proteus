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

    ap_unit[DATA_WIDTH] link_in[NUM_NODES];
    ap_unit[DATA_WIDTH] link_out[NUM_NODES];
    int buffer_size= 4;

        node[0] = Router(0,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node
                    link_in[0],link_out[NUM_NODES-1]);
    for (int i = 1 ; i < NUM_NODES; i++)
    {
        node[i] = Router(i,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node
                    link_in[i],link_out[i-1]);
    }
//         node[NUM_NODES-1] = Router(NUM_NODES-1,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node
//                     link_in[i],link_out[i-1]);

    while(total_packets_recieved < num_packet_per_node*NUM_NODES)
    {
        node[i].start_router();
    }
}
