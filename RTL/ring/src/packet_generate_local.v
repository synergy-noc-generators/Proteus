`timescale 1ns / 1ps


// cannot do mod in verilog for counting inject time

module packet_generate_local #(
    parameter NUM_NODES = 8,
    parameter ROUTER_ID = 0,
    parameter TRAFFIC_PATTERN = 0,
    parameter PACKET_SIZE = 49,
    parameter BUFFER_SIZE = 4,
    parameter NUM_PACKETS_PER_NODE = 20,
    parameter INJECT_CYCLE = 2
) (
    clk,
    rst_n,
    clk_counter,
    inject_clk_ref,

    packet_wr_en,
    packet,
    
    total_packet_sent
);

    input clk, rst_n;

    input [15 : 0] clk_counter;
    input [15 : 0] inject_clk_ref;

    input packet_wr_en;

    output [PACKET_SIZE - 1 : 0] packet;
    output [63 : 0]              total_packet_sent;

    wire [15 : 0] router_src;
    reg  [15 : 0] router_dst;

    assign router_src = ROUTER_ID;

    reg [PACKET_SIZE - 1 : 0] packet_inner;

    reg [15 : 0] incre_cnt, decre_cnt, packets_wait_cnt;
    reg [63 : 0] total_packet_sent_inner;

    assign incre_cnt = inject_clk_ref == INJECT_CYCLE - 1 ? 1'b1 : 1'b0;
    assign decre_cnt = packet_wr_en && packets_wait_cnt > 0 ? 1'b1 : 1'b0;

    always @(*) begin
        if (~rst_n) begin
            router_dst = 0;
        end
        else begin
            if (TRAFFIC_PATTERN == 0) begin // bit complement
                router_dst = NUM_NODES - router_src - 1;
            end
            else begin
                router_dst = 0;
            end
        end
    end

    always @(posedge clk or negedge rst_n) begin
        if (~rst_n) begin
            packet_inner <= 0;
            packets_wait_cnt <= 0;
            total_packet_sent_inner <= 0;
        end
        else begin
            if (packet_wr_en && packets_wait_cnt > 0) begin
                packet_inner <= {1'b1, clk_counter, router_src, router_dst};
                total_packet_sent_inner <= total_packet_sent_inner + 1'b1;
            end

            packets_wait_cnt <= packets_wait_cnt + incre_cnt + decre_cnt;
        end
    end

    assign packet = packet_inner;
    assign total_packet_sent = total_packet_sent_inner;

endmodule