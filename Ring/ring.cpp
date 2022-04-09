#include "Router.h"
#include "VN.h"
#include "common.h"
#include <stdio.h>

void ring( 
        INT16       deadlock_cycles,
        INT16       num_packets_per_node,
        INT16       inject_rate,
        INT16       num_packets_sent[NUM_NODES],
        INT16       num_packets_recieved[NUM_NODES],
        INT16       avg_latency[NUM_NODES], 
        INT16       max_latency[NUM_NODES], 
        INT16     deadlock_detected[NUM_NODES]
        )
{
#pragma HLS interface s_axilite register port=return
#pragma HLS interface s_axilite register port=deadlock_cycles
#pragma HLS interface s_axilite register port=inject_rate
#pragma HLS interface s_axilite register port=num_packets_sent
#pragma HLS interface s_axilite register port=num_packets_per_node
#pragma HLS interface s_axilite register port=num_packets_recieved
#pragma HLS interface s_axilite register port=avg_latency
#pragma HLS interface s_axilite register port=max_latency
#pragma HLS interface s_axilite register port=deadlock_detected

    static Router node[NUM_NODES]; 
    static VN noc_vn(NUM_NODES);
   
    int routing_algorithm = 0;
    int traffic_pattern = 0;

    noc_vn = VN(deadlock_cycles, num_packets_per_node, inject_rate, routing_algorithm, traffic_pattern,NUM_NODES);

    Packet link_east[NUM_NODES]; // MODIFICATION: instead of the datawith define, we can make it packet granularity wire
    Packet link_west[NUM_NODES];

    INT16 onoff_switch_east[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
    INT16 onoff_switch_west[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
//     int buffer_size = 4;

//     node[0] = Router(0,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node,
//             link_east[0],link_west[0], link_east[NUM_NODES-1], link_west[1]); // THINK: maybe the router level do not need to know the id of next, Ring level is good enough. The router provide the data when needed, and ring allocate it to the right place
    for (int i = 0 ; i < NUM_NODES; i++)
    {
        node[i] = Router(i,routing_algorithm,traffic_pattern,NUM_NODES);
    }
//     node[NUM_NODES-1] = Router(NUM_NODES-1,buffer_size,routing_algorithm,traffic_pattern,NUM_NODES,num_packet_per_node
//             link_east[NUM_NODES-1],link_west[NUM_NODES-1],link_east[NUM_NODES-2],link_west[0]);
    int total_packets_recieved = 0;
    int total_packets_sent = 0;
//     while(total_packets_recieved < num_packets_per_node*NUM_NODES)
    for( int j = 0; j< 10000; j++)
    {
        node[0].router_phase_one( link_west[1],link_east[NUM_NODES-1], noc_vn);

        for (int i = 1 ; i < (NUM_NODES-1); i++)
        {
            //NOte: The EAST inp will come from West[i+1] amd west inp will come from east[i-1]
            //
            //      L_W[i]        L_W[i+1]
            //      <-----|------|<------
            //            |Node  |        
            //            |  i   |
            //    L_E[i-1]|      |L_E[i]
            //      ----->|------|------>      
            //
            /////////////////////////////////////
            //In phase one following things happens, Packets are read from Link and written to the BUffers
            //New Packets are generated in the buffer, Route is computed for each package that is to be sent out.
            node[i].router_phase_one( link_west[i+1],link_east[i-1], noc_vn);
        }
        node[NUM_NODES-1].router_phase_one( link_west[0],link_east[NUM_NODES-2], noc_vn);

        for (int i = 0; i < NUM_NODES; i++)
        {
            onoff_switch_east[i] = node[i].on_off_switch_update(EAST);
            onoff_switch_west[i] = node[i].on_off_switch_update(WEST);
        }

       for(int i = 0 ; i< NUM_NODES; i++)
       { 
            //In Phase 2, The packets are written to the links for the next cycle.
            if(i==0)
                link_west[0] = node[0].router_phase_two(onoff_switch_east[NUM_NODES-1], WEST);
            else
                link_west[i] = node[i].router_phase_two(onoff_switch_east[i-1], WEST);
            if(i==(NUM_NODES-1))
                link_east[NUM_NODES-1] = node[NUM_NODES-1].router_phase_two(onoff_switch_west[0], EAST);
            else
                link_east[i] = node[i].router_phase_two(onoff_switch_west[i+1], EAST);

        //          This Function will help in getting the function statistics
            //printf("LInk West %d: %d %d %d %d \n", i, link_west[i].valid, (int)link_west[i].timestamp, (int)link_west[i].source, (int)link_west[i].dest);
            //printf("LInk East %d: %d %d %d %d \n", i, link_east[i].valid, (int)link_east[i].timestamp, (int)link_east[i].source, (int)link_east[i].dest);
        }    
            
       total_packets_recieved = 0;
       total_packets_sent = 0;

       for(int i = 0 ; i< NUM_NODES; i++)
        {   
            //std::cout << "Node : "<<i<< " , num packets added till now = "<< node[i].get_packets_sent() << std::endl;
            total_packets_recieved += node[i].get_packets_recieved();
            total_packets_sent += node[i].get_packets_sent();
        }
        noc_vn.inc_cycle();
    }
    //printf("packet recieved: %d", (int)total_packets_recieved);
    //printf("packet sent: %d", (int)total_packets_sent);

}
