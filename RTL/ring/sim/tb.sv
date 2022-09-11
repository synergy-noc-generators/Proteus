`timescale 1ns / 1ps
//##########################################################
// Ring  Testbench"
// Author: Abhimanyu Bambhaniya"
// Contact: abambhaniya3@gatech.edu"
//##########################################################

module tb();

logic rst_n,clk;

initial 
begin
    clk = 0;
end

always #1 clk = !clk;

initial begin
	rst_n = 1'b0;
	#4
	rst_n = 1'b1;
    #10000;
    $finish();
end

ring #(
    .NUM_NODES(4),
    .PACKET_SIZE(49), // 1 bit VALID, 16 bits timestamp, 16 bits source, 16 bits destination
    .BUFFER_SIZE(4),
    .INJECT_CYCLE(2), // 1 packet every 2 cycles
    .NUM_PACKETS_PER_NODE(20), // how many packets each node will inject
    .ROUTING(0), // 0 means XY
    .TRAFFIC_PATTERN(0) // 0 means bit complement
)

u_inst_ring ( .*);




endmodule
