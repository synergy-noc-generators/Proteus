/*
Description: This is the topology ring file

Authors:    Abhimanyu Bambhaniya (abambhaniya3@gatech.edu)
            Yangyu Chen (yangyuchen@gatech.edu)
*/

#include "Router.h"
#include "VN.h"
#include "common.h"
#include <stdio.h>
// #define DEBUG

////////////////////Torus Config/////////////////////////////
//
//  0 - 1 - 2 - 3  
//  |   |   |   |
//  4 - 5 - 6 - 7
//  |   |   |   |   
//  8 - 9 -10 - 11
//  |   |   |   |
//  12- 13- 14- 15
//
//
///////////////////////////////////////////////////////////////
void torus( 
        int       deadlock_cycles,
        int       num_packets_per_node,
        int       packet_inject_period, // EX. =10, every 10 cycles, a packet will be generated at each node
        int       num_packets_sent[NUM_NODES],
        int       num_packets_recieved[NUM_NODES],
        int       added_latency[NUM_NODES], 
        int       max_latency[NUM_NODES], 
        int       deadlock_detected[NUM_NODES]
        )
{
#pragma HLS interface s_axilite register port=return
#pragma HLS interface s_axilite register port=deadlock_cycles
#pragma HLS interface s_axilite register port=packet_inject_period
#pragma HLS interface s_axilite register port=num_packets_sent
#pragma HLS interface s_axilite register port=num_packets_per_node
#pragma HLS interface s_axilite register port=num_packets_recieved
#pragma HLS interface s_axilite register port=added_latency
#pragma HLS interface s_axilite register port=max_latency
#pragma HLS interface s_axilite register port=deadlock_detected

    static Router node[NUM_NODES]; 
    static VN noc_vn(NUM_NODES);
   
    int routing_algorithm = 1;
    int traffic_pattern = 0;

    noc_vn = VN(deadlock_cycles, num_packets_per_node, packet_inject_period, routing_algorithm, traffic_pattern,NUM_NODES);

    Packet link_east[NUM_NODES];
    Packet link_west[NUM_NODES];
    Packet link_north[NUM_NODES];
    Packet link_south[NUM_NODES];

    INT16 onoff_switch_east[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
    INT16 onoff_switch_west[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
    INT16 onoff_switch_north[NUM_NODES]; // 16 bits reserve for credit-base switch in the future
    INT16 onoff_switch_south[NUM_NODES]; // 16 bits reserve for credit-base switch in the future

    for (int i = 0 ; i < NUM_NODES; i++)
    {
        node[i] = Router(i,routing_algorithm,traffic_pattern);
    }

    int total_packets_recieved = 0;
    while(total_packets_recieved < num_packets_per_node*NUM_NODES)
//     while(noc_vn.get_current_cycle() < 1000)
    {

        for (int i = 0 ; i < (NUM_NODES); i++)
        {
            //NOte: The EAST inp will come from West[i+1] amd west inp will come from east[i-1]
            //        L_N[i]^  | L_S[i-NODES_PER_ROW]
            //              |  | 
            //              |  |
            //      L_W[i]  |  ⇩   L_W[i+1]
            //      <-----|------|<------
            //            |Node  |        
            //            |  i   |
            //    L_E[i-1]|      |L_E[i]
            //      ----->|------|------>      
            //              ^   |
            //              |   |
            //              |   |   L_S[i]
            //              |   ⬇︎
            //      L_N[i+NODES_PER_ROW]
            //
            /////////////////////////////////////
            //In phase one following things happens, Packets are read from Link and written to the BUffers
            //New Packets are generated in the buffer, Route is computed for each package that is to be sent out.
            if((i/NODES_PER_ROW==0) && (i%NODES_PER_ROW==0))        //NW    0
                node[i].router_phase_one( link_south[i+NUM_NODES-NODES_PER_ROW],link_west[i+1],link_east[i+NODES_PER_ROW-1], link_north[i+NODES_PER_ROW], noc_vn);
            else if((i/NODES_PER_ROW==0) && (i%NODES_PER_ROW == (NUM_COLS-1)))       //NE    3
                node[i].router_phase_one( link_south[i+NUM_NODES-NODES_PER_ROW],link_west[i-NODES_PER_ROW+1],link_east[i-1], link_north[i+NODES_PER_ROW], noc_vn);
            else if((i/NODES_PER_ROW == (NUM_ROWS-1)) && (i%NODES_PER_ROW == 0))       //SW    12
                node[i].router_phase_one( link_south[i-NODES_PER_ROW],link_west[i+1],link_east[i+NODES_PER_ROW-1], link_north[i+NODES_PER_ROW-NUM_NODES], noc_vn);
            else if((i/NODES_PER_ROW == (NUM_ROWS-1)) && (i%NODES_PER_ROW == (NUM_COLS-1)))       //SE    15
                node[i].router_phase_one( link_south[i-NODES_PER_ROW],link_west[i-NODES_PER_ROW+1],link_east[i-1], link_north[i+NODES_PER_ROW-NUM_NODES], noc_vn);
            else if((i/NODES_PER_ROW==0))       //N
                node[i].router_phase_one( link_south[i+NUM_NODES-NODES_PER_ROW],link_west[i+1],link_east[i-1], link_north[i+NODES_PER_ROW], noc_vn);
            else if((i/NODES_PER_ROW == (NUM_ROWS-1)))       //S
                node[i].router_phase_one( link_south[i-NODES_PER_ROW],link_west[i+1],link_east[i-1], link_north[i+NODES_PER_ROW-NUM_NODES], noc_vn);
            else if((i%NODES_PER_ROW == 0))       //W
                node[i].router_phase_one( link_south[i-NODES_PER_ROW],link_west[i+1],link_east[i+NODES_PER_ROW-1], link_north[i+NODES_PER_ROW], noc_vn);
            else if((i%NODES_PER_ROW == (NUM_COLS-1)))       //E
                node[i].router_phase_one( link_south[i-NODES_PER_ROW],link_west[i-NODES_PER_ROW+1],link_east[i-1], link_north[i+NODES_PER_ROW], noc_vn);
            else
                node[i].router_phase_one( link_south[i-NODES_PER_ROW],link_west[i+1],link_east[i-1], link_north[i+NODES_PER_ROW], noc_vn);
        }

        for (int i = 0; i < NUM_NODES; i++)
        {
            onoff_switch_east[i] = node[i].on_off_switch_update(EAST);
            onoff_switch_west[i] = node[i].on_off_switch_update(WEST);
            onoff_switch_north[i] = node[i].on_off_switch_update(NORTH);
            onoff_switch_south[i] = node[i].on_off_switch_update(SOUTH);
#ifdef DEBUG
            std::cout << "Node : "<<i<< " , onoff switch E W N S = "<< onoff_switch_east[i] << onoff_switch_west[i] << onoff_switch_north[i] << onoff_switch_south[i] << std::endl;
#endif
        }

       for(int i = 0 ; i< NUM_NODES; i++)
       { 
            //In Phase 2, The packets are written to the links for the next cycle.
            if(i/NODES_PER_ROW==0)                                                              // First Row(North most)
                link_north[i] = node[i].router_phase_two(onoff_switch_south[NUM_NODES - NODES_PER_ROW + i], NORTH);
            else 
                link_north[i] = node[i].router_phase_two(onoff_switch_south[i-NODES_PER_ROW], NORTH);

            if(i/NODES_PER_ROW == (NUM_ROWS-1))                                                 // Last Row(South Most)
                link_south[i] = node[i].router_phase_two(onoff_switch_north[i-(NUM_NODES - NODES_PER_ROW)], SOUTH);
            else
                link_south[i] = node[i].router_phase_two(onoff_switch_north[i+NODES_PER_ROW], SOUTH);

            if(i%NODES_PER_ROW == 0)                                                            // West Row
                link_west[i] = node[i].router_phase_two(onoff_switch_east[i+NODES_PER_ROW-1], WEST);            
            else
                link_west[i] = node[i].router_phase_two(onoff_switch_east[i-1], WEST);
            
            if(i%NODES_PER_ROW == (NUM_COLS-1))                                                 //East Row
                link_east[i] = node[i].router_phase_two(onoff_switch_west[i-NODES_PER_ROW-1], EAST);
            else
                link_east[i] = node[i].router_phase_two(onoff_switch_west[i+1], EAST);

        //          This Function will help in getting the function statistics
#ifdef DEBUG
            printf("LInk West %d: %d %d %d %d \n", i, link_west[i].valid, (int)link_west[i].timestamp, (int)link_west[i].source, (int)link_west[i].dest);
            printf("LInk East %d: %d %d %d %d \n", i, link_east[i].valid, (int)link_east[i].timestamp, (int)link_east[i].source, (int)link_east[i].dest);
            printf("LInk North %d: %d %d %d %d \n", i, link_north[i].valid, (int)link_north[i].timestamp, (int)link_north[i].source, (int)link_north[i].dest);
            printf("LInk South %d: %d %d %d %d \n", i, link_south[i].valid, (int)link_south[i].timestamp, (int)link_south[i].source, (int)link_south[i].dest);
#endif
        }   

        total_packets_recieved = 0;
        for(int i = 0 ; i< NUM_NODES; i++)
        {   
            total_packets_recieved += node[i].get_packets_recieved();
#ifdef DEBUG
//         std::cout << "Node : "<<i<< " , num packets added till now = "<< node[i].get_packets_sent() << std::endl;
#endif
        }
        noc_vn.inc_cycle();
    }

    for(int i = 0 ; i< NUM_NODES; i++)
    {   
        num_packets_recieved[i] = node[i].get_packets_recieved();
        num_packets_sent[i] = node[i].get_packets_sent();
        added_latency[i] =  node[i].get_added_latency();
        max_latency[i] = node[i].get_max_latency();
        deadlock_detected[i] = node[i].get_deadlock_info();
    }
    //printf("packet recieved: %d", (int)total_packets_recieved);
    //printf("packet sent: %d", (int)total_packets_sent);

}
