`timescale 1ns / 1ps

module ring #(
    parameter NUM_NODES = 4,
    parameter PACKET_SIZE = 49, // 1 bit VALID, 16 bits timestamp, 16 bits source, 16 bits destination
    parameter BUFFER_SIZE = 4,
    parameter INJECT_CYCLE = 2, // 1 packet every 2 cycles
    parameter NUM_PACKETS_PER_NODE = 20, // how many packets each node will inject
    parameter ROUTING = 0, // 0 means XY
    parameter TRAFFIC_PATTERN = 0 // 0 means bit complement
)(
    clk,
    rst_n
);

    input clk;
    input rst_n;

    reg [63 : 0] total_packet_sent [NUM_NODES - 1 : 0];
    reg [63 : 0] total_packet_recieve [NUM_NODES - 1 : 0];
    reg [63 : 0] total_latency [NUM_NODES - 1 : 0];

    reg [PACKET_SIZE - 1 : 0] link_east [NUM_NODES - 1 : 0];
    reg [PACKET_SIZE - 1 : 0] link_west [NUM_NODES - 1 : 0];

    reg     onoff_switch_east [NUM_NODES - 1 : 0];
    reg     onoff_switch_west [NUM_NODES - 1 : 0];

    reg [15 : 0]    clk_counter;
    reg [15 : 0]    inject_clk_ref;

    always_ff @(posedge clk or negedge rst_n) begin
        if (~rst_n) begin
            clk_counter <= 0;
            inject_clk_ref <= 0;
        end
        else begin
            clk_counter <= clk_counter + 1'b1;
            inject_clk_ref <= (inject_clk_ref == INJECT_CYCLE - 1) ? 0 : inject_clk_ref + 1'b1;
        end
    end
    

        router #(
            .NUM_NODES(NUM_NODES),
            .ROUTER_ID(0),
            .PACKET_SIZE(PACKET_SIZE),
            .BUFFER_SIZE(BUFFER_SIZE),
            .INJECT_CYCLE(INJECT_CYCLE),
            .NUM_PACKETS_PER_NODE(NUM_PACKETS_PER_NODE),
            .ROUTING(ROUTING),
            .TRAFFIC_PATTERN(TRAFFIC_PATTERN)
        ) node0 (
            .clk(clk),
            .rst_n(rst_n),
            .clk_counter(clk_counter),
            .inject_clk_ref(inject_clk_ref),
            .link_east_in(link_west[1]),
            .link_west_in(link_east[NUM_NODES - 1]),
            .backpressure_east_rd(onoff_switch_west[1]),
            .backpressure_west_rd(onoff_switch_east[NUM_NODES - 1]),
            .link_east_out(link_east[0]),
            .link_west_out(link_west[0]),
            .backpressure_east_wr(onoff_switch_east[0]),
            .backpressure_west_wr(onoff_switch_west[0]),

            .total_packet_sent(total_packet_sent[0]),
            .total_packet_recieve(total_packet_recieve[0]),
            .total_latency(total_latency[0])
        );

    genvar i;
    generate
        for (i = 1; i < NUM_NODES - 1; i = i + 1) begin
            router #(
                .NUM_NODES(NUM_NODES),
                .ROUTER_ID(i),
                .PACKET_SIZE(PACKET_SIZE),
                .BUFFER_SIZE(BUFFER_SIZE),
                .INJECT_CYCLE(INJECT_CYCLE),
                .NUM_PACKETS_PER_NODE(NUM_PACKETS_PER_NODE),
                .ROUTING(ROUTING),
                .TRAFFIC_PATTERN(TRAFFIC_PATTERN)
            ) node_middle (
                .clk(clk),
                .rst_n(rst_n),
                .clk_counter(clk_counter),
                .inject_clk_ref(inject_clk_ref),
                .link_east_in(link_west[i + 1]),
                .link_west_in(link_east[i - 1]),
                .backpressure_east_rd(onoff_switch_west[i + 1]),
                .backpressure_west_rd(onoff_switch_east[i - 1]),
                .link_east_out(link_east[i]),
                .link_west_out(link_west[i]),
                .backpressure_east_wr(onoff_switch_east[i]),
                .backpressure_west_wr(onoff_switch_west[i]),

                .total_packet_sent(total_packet_sent[i]),
                .total_packet_recieve(total_packet_recieve[i]),
                .total_latency(total_latency[i])
        );
    end
    endgenerate

        router #(
            .NUM_NODES(NUM_NODES),
            .ROUTER_ID(NUM_NODES - 1),
            .PACKET_SIZE(PACKET_SIZE),
            .BUFFER_SIZE(BUFFER_SIZE),
            .INJECT_CYCLE(INJECT_CYCLE),
            .NUM_PACKETS_PER_NODE(NUM_PACKETS_PER_NODE),
            .ROUTING(ROUTING),
            .TRAFFIC_PATTERN(TRAFFIC_PATTERN)
        ) node_last (
            .clk(clk),
            .rst_n(rst_n),
            .clk_counter(clk_counter),
            .inject_clk_ref(inject_clk_ref),
            .link_east_in(link_west[0]),
            .link_west_in(link_east[NUM_NODES - 2]),
            .backpressure_east_rd(onoff_switch_west[0]),
            .backpressure_west_rd(onoff_switch_east[NUM_NODES - 2]),
            .link_east_out(link_east[NUM_NODES - 1]),
            .link_west_out(link_west[NUM_NODES - 1]),
            .backpressure_east_wr(onoff_switch_east[NUM_NODES - 1]),
            .backpressure_west_wr(onoff_switch_west[NUM_NODES - 1]),

            .total_packet_sent(total_packet_sent[NUM_NODES - 1]),
            .total_packet_recieve(total_packet_recieve[NUM_NODES - 1]),
            .total_latency(total_latency[NUM_NODES - 1])
        );
    
endmodule
