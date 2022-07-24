`timescale 1ns / 1ps

module router #(
    parameter NUM_NODES = NUM_NODES,
    parameter ROUTER_ID = 0,
    parameter PACKET_SIZE = 49, // 1 bit VALID, 16 bits timestamp, 16 bits source, 16 bits destination
    parameter BUFFER_SIZE = 4,
    parameter INJECT_CYCLE = 2, // 1 packet every 2 cycles
    parameter NUM_PACKETS_PER_NODE = 20, // how many packets each node will inject
    parameter ROUTING = 0, // 0 means XY
    parameter TRAFFIC_PATTERN = 0, // 0 means bit complement
    parameter BUFFER_THRESHOLD = 1 // threshold for backpressure to start
)(
    clk,
    rst_n,
    clk_counter,
    inject_clk_ref,

    link_east_in,
    link_west_in,

    backpressure_east_rd, // output valid/stall is depend on reading this
    backpressure_west_rd,

    link_eat_out,
    link_west_out,

    backpressure_east_wr, // update this, other node depend on this
    backpressure_west_wr,

    total_packet_sent,
    total_packet_recieve,
    total_latency
);

    localparam ptr_len = $clog2(BUFFER_SIZE);

    input clk, rst_n;
    input [15 : 0] clk_counter;
    input [15 : 0] inject_clk_ref;

    input [PACKET_SIZE - 1 : 0] link_east_in, link_west_in;
    input backpressure_east_rd, backpressure_west_rd;

    output [PACKET_SIZE - 1 : 0] link_east_out, link_west_out;
    output backpressure_east_wr, backpressure_west_wr;

    output [63 : 0] total_packet_sent, total_packet_recieve, total_latency;

    wire [63 : 0] total_packet_sent_inner;
    reg  [63 : 0] total_packet_recieve_inner, total_latency_inner;

    // TODO: arbitrate using mux for each buffer by its timestamp.
    reg [PACKET_SIZE - 1 : 0] buffer_east [BUFFER_SIZE - 1 : 0];
    reg [PACKET_SIZE - 1 : 0] buffer_west [BUFFER_SIZE - 1 : 0];
    reg [PACKET_SIZE - 1 : 0] buffer_local [BUFFER_SIZE - 1 : 0];

    // reg [ptr_len - 1 : 0] east_rd_ptr, west_rd_ptr, local_rd_ptr;
    // reg [ptr_len - 1 : 0] east_wr_ptr, west_wr_ptr, local_wr_ptr;

    wire [ptr_len - 1 : 0] empty_east, empty_west, empty_local;
    wire                   empty_east_valid, empty_west_valid, empty_local_valid;

    // find all the empty buffer position, and if the *_valid is 0, means no empty spot is found
    find_empty_buffer #(
        .BUFFER_SIZE(BUFFER_SIZE),
        .PACKET_SIZE(PACKET_SIZE)
        .PTR_LEN(ptr_len),
    ) east (
        .clk(clk),
        .rst_n(rst_n),
        .buffer(buffer_east),
        .empty_pos(empty_east),
        .empty_pos_valid(empty_east_valid)
    );

    find_empty_buffer #(
        .BUFFER_SIZE(BUFFER_SIZE),
        .PACKET_SIZE(PACKET_SIZE)
        .PTR_LEN(ptr_len),
    ) west (
        .clk(clk),
        .rst_n(rst_n),
        .buffer(buffer_west),
        .empty_pos(empty_west),
        .empty_pos_valid(empty_west_valid)
    );

    find_empty_buffer #(
        .BUFFER_SIZE(BUFFER_SIZE),
        .PACKET_SIZE(PACKET_SIZE)
        .PTR_LEN(ptr_len),
    ) local (
        .clk(clk),
        .rst_n(rst_n),
        .buffer(buffer_local),
        .empty_pos(empty_local),
        .empty_pos_valid(empty_local_valid)
    );

    wire [PACKET_SIZE - 1 : 0]  packet;

    // local packets generate depends on inject cycle and total number of packets
    packet_generate_local #(
        .NUM_NODES(NUM_NODES),
        .ROUTER_ID(ROUTER_ID),
        .TRAFFIC_PATTERN(TRAFFIC_PATTERN),
        .PACKET_SIZE(PACKET_SIZE),
        .BUFFER_SIZE(BUFFER_SIZE),
        .NUM_PACKETS_PER_NODE(NUM_PACKETS_PER_NODE),
        .INJECT_CYCLE(INJECT_CYCLE)
    ) packet_gen (
        .clk(clk),
        .rst_n(rst_n),
        .clk_counter(clk_counter),
        .inject_clk_ref(inject_clk_ref),
        .packet_wr_en(empty_local_valid),
        .packet(buffer_local[empty_local]),
        .total_packet_sent(total_packet_sent_inner)
    );


    reg recieve_cnt = 0;
    reg latency_add = 0;

    genvar i;
    generate
        for (i = 0; i < BUFFER_SIZE; i = i + 1) begin
            always @(*) begin
                if (buffer_east[i][PACKET_SIZE - 1] == 1'b1 && buffer_east[i][15:0] == ROUTER_ID) begin
                    recieve_cnt = recieve_cnt + 1;
                    latency_add = latency_add + (clk_counter - buffer_east[i][47:32]);
                end

                if (buffer_west[i][PACKET_SIZE - 1] == 1'b1 && buffer_west[i][15:0] == ROUTER_ID) begin
                    recieve_cnt = recieve_cnt + 1;
                    latency_add = latency_add + (clk_counter - buffer_east[i][47:32]);
                end

                if (buffer_local[i][PACKET_SIZE - 1] == 1'b1 && buffer_local[i][15:0] == ROUTER_ID) begin
                    recieve_cnt = recieve_cnt + 1;
                    latency_add = latency_add + (clk_counter - buffer_east[i][47:32]);
                end
            end

            always @(posedge clk or negedge rst_n) begin
                if (~rst_n) begin
                    buffer_east <= 0;
                    buffer_west <= 0;
                    buffer_local <= 0;
                end
                else begin
                    if (buffer_east[i][PACKET_SIZE - 1] == 1'b1 && buffer_east[i][15:0] == ROUTER_ID) begin
                        buffer_east[i] <= 0;
                    end
                    else begin
                        buffer_east[i] <= buffer_east[i];
                    end

                    if (buffer_west[i][PACKET_SIZE - 1] == 1'b1 && buffer_west[i][15:0] == ROUTER_ID) begin
                        buffer_west[i] <= 0;
                    end
                    else begin
                        buffer_west[i] <= buffer_west[i];
                    end

                    if (buffer_local[i][PACKET_SIZE - 1] == 1'b1 && buffer_local[i][15:0] == ROUTER_ID) begin
                        buffer_local[i] <= 0;
                    end
                    else begin
                        buffer_local[i] <= buffer_local[i];
                    end
                end
            end
        end
    endgenerate


    always @(posedge clk or negedge rst_n) begin
        if (~rst_n) begin
            total_packet_recieve_inner <= 0;
            total_latency <= 0;
        end
        else begin
            total_packet_recieve_inner <= total_packet_recieve_inner + recieve_cnt;
            total_latency <= total_latency + latency_add;
        end
    end

    // next step: buffer write of input data, Router compute (for each packet, decide where it goes)
    // update backpressure, then switch allocator, for each direction, depending on backpressure, write to output wire





endmodule