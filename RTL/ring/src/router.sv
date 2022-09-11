`timescale 1ns / 1ps

module router #(
    parameter NUM_NODES = 4,
    parameter ROUTER_ID = 0,
    parameter PACKET_SIZE = 49, // 1 bit VALID, 16 bits timestamp, 16 bits source, 16 bits destination
    parameter BUFFER_SIZE = 4,
    parameter INJECT_CYCLE = 2, // 1 packet every 2 cycles
    parameter NUM_PACKETS_PER_NODE = 20, // how many packets each node will inject
    parameter ROUTING = 0, // 0 means random oblivious.
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

    link_east_out,
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

    output logic [PACKET_SIZE - 1 : 0] link_east_out, link_west_out;
    output logic backpressure_east_wr, backpressure_west_wr;

    output logic [63 : 0] total_packet_sent, total_packet_recieve, total_latency;

    wire [63 : 0] total_packet_sent_inner;
    reg  [63 : 0] total_packet_recieve_inner, total_latency_inner;

    // TODO: arbitrate using mux for each buffer by its timestamp.
    reg [PACKET_SIZE - 1 : 0] buffer_east [BUFFER_SIZE - 1 : 0];
    reg [PACKET_SIZE - 1 : 0] buffer_west [BUFFER_SIZE - 1 : 0];
    reg [PACKET_SIZE - 1 : 0] buffer_local [BUFFER_SIZE - 1 : 0];

    reg [1 : 0] buffer_east_route_info [BUFFER_SIZE - 1 : 0];
    reg [1 : 0] buffer_west_route_info [BUFFER_SIZE - 1 : 0];
    reg [1 : 0] buffer_local_route_info [BUFFER_SIZE - 1 : 0];

    // reg [ptr_len - 1 : 0] east_rd_ptr, west_rd_ptr, local_rd_ptr;
    // reg [ptr_len - 1 : 0] east_wr_ptr, west_wr_ptr, local_wr_ptr;

    wire [ptr_len - 1 : 0] empty_east, empty_west, empty_local;
    wire                   empty_east_found, empty_west_found, empty_local_found;

    logic [15 : 0] out_packet_pos_east, out_packet_pos_west;
    logic          out_packet_pos_valid_east_out, out_packet_pos_valid_west_out;
    logic          out_packet_pos_in_east, out_packet_pos_in_west;
    // find all the empty buffer position, and if the *_valid is 0, means no empty spot is found
    find_empty_buffer #(
        .BUFFER_SIZE(BUFFER_SIZE),
        .PACKET_SIZE(PACKET_SIZE),
        .PTR_LEN(ptr_len)
    ) east (
        .clk(clk),
        .rst_n(rst_n),
        .buffer(buffer_east),
        .empty_pos(empty_east),
        .empty_pos_found(empty_east_found)
    );

    find_empty_buffer #(
        .BUFFER_SIZE(BUFFER_SIZE),
        .PACKET_SIZE(PACKET_SIZE),
        .PTR_LEN(ptr_len)
    ) west (
        .clk(clk),
        .rst_n(rst_n),
        .buffer(buffer_west),
        .empty_pos(empty_west),
        .empty_pos_found(empty_west_found)
    );

    find_empty_buffer #(
        .BUFFER_SIZE(BUFFER_SIZE),
        .PACKET_SIZE(PACKET_SIZE),
        .PTR_LEN(ptr_len)
    ) local_find_empyt_buffer (
        .clk(clk),
        .rst_n(rst_n),
        .buffer(buffer_local),
        .empty_pos(empty_local),
        .empty_pos_found(empty_local_found)
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
        .INJECT_CYCLE(INJECT_CYCLE),
        .ROUTING(ROUTING)
    ) packet_gen (
        .clk(clk),
        .rst_n(rst_n),
        .clk_counter(clk_counter),
        .inject_clk_ref(inject_clk_ref),
        .packet_wr_en(1'b1),
        .packet(buffer_local[empty_local]),
        .packet_route_info(buffer_local_route_info[empty_local]),
        .total_packet_sent(total_packet_sent_inner)
    );


    logic [2:0] recieve_cnt[BUFFER_SIZE];
    logic [15:0] latency_add[BUFFER_SIZE];

    genvar i;
    generate
        for (i = 0; i < BUFFER_SIZE; i = i + 1) begin
            route_compute #(
                .ROUTER_ID(ROUTER_ID),
                .ROUTING(ROUTING),
                .PACKET_SIZE(PACKET_SIZE),
                .IN_PORT(2'b01)
            ) east (
                .clk(clk),
                .rst_n(rst_n),
                .in_buffer(buffer_east[i]),
                .route_update_en((i == empty_east && empty_east_found)),
                .out_dir(buffer_east_route_info[i])
            );

            route_compute #(
                .ROUTER_ID(ROUTER_ID),
                .ROUTING(ROUTING),
                .PACKET_SIZE(PACKET_SIZE),
                .IN_PORT(2'b10) 
            ) west (
                .clk(clk),
                .rst_n(rst_n),
                .in_buffer(buffer_west[i]),
                .route_update_en((i == empty_west && empty_west_found)),
                .out_dir(buffer_west_route_info[i])
            );

            always_comb begin
               latency_add[i] = 0; 
               recieve_cnt[i] = 0; 
               if (buffer_east[i][PACKET_SIZE - 1] == 1'b1 && buffer_east_route_info[i] == 2'b00) begin
                   recieve_cnt[i] = recieve_cnt[i] + 1;
                   latency_add[i] = latency_add[i] + (clk_counter - buffer_east[i][47:32]);
               end

               if (buffer_west[i][PACKET_SIZE - 1] == 1'b1 && buffer_west_route_info[i] == 2'b00) begin
                   recieve_cnt[i] = recieve_cnt[i] + 1;
                   latency_add[i] = latency_add[i] + (clk_counter - buffer_west[i][47:32]);
               end

               if (buffer_local[i][PACKET_SIZE - 1] == 1'b1 && buffer_local_route_info[i] == 2'b00) begin
                   recieve_cnt[i] = recieve_cnt[i] + 1;
                   latency_add[i] = latency_add[i] + (clk_counter - buffer_local[i][47:32]);
               end
            end

            always_ff @(posedge clk or negedge rst_n) begin
                if (~rst_n) begin
                    buffer_east[i] <= 0;
                    buffer_west[i] <= 0;
                    buffer_local[i] <= 0;
                end
                else begin
                    // we will not be able to overwriting leaving packet with new packet immediately,
                    // need to set the value to zero first
                    if (buffer_east[i][PACKET_SIZE - 1] == 1'b1 && buffer_east_route_info[i] == 2'b00) begin
                        buffer_east[i] <= 0;
                    end
                    else if (i == out_packet_pos_east && out_packet_pos_in_east && out_packet_pos_valid_east_out) begin
                        buffer_east[i] <= 0;
                    end
                    else if (i == empty_east && empty_east_found) begin
                        buffer_east[i] <= link_east_in;
                    end
                    else begin
                        buffer_east[i] <= buffer_east[i];
                    end

                    if (buffer_west[i][PACKET_SIZE - 1] == 1'b1 && buffer_west_route_info[i] == 2'b00) begin
                        buffer_west[i] <= 0;
                    end
                    else if (i == out_packet_pos_west && out_packet_pos_in_west && out_packet_pos_valid_west_out) begin
                        buffer_west[i] <= 0;
                    end
                    else if (i == empty_west && empty_west_found) begin
                        buffer_west[i] <= link_west_in;
                    end
                    else begin
                        buffer_west[i] <= buffer_west[i];
                    end

                    // the fulfilling of empty buffer is done through 'find_empty_buffer.v'
                    if (buffer_local[i][PACKET_SIZE - 1] == 1'b1 && buffer_local_route_info[i] == 2'b00) begin
                        buffer_local[i] <= 0;
                    end
                    else if (i == out_packet_pos_east && !out_packet_pos_in_east && out_packet_pos_valid_east_out) begin
                        buffer_local[i] <= 0;
                    end
                    else if (i == out_packet_pos_west && !out_packet_pos_in_west && out_packet_pos_valid_west_out) begin
                        buffer_local[i] <= 0;
                    end
                    else begin
                        buffer_local[i] <= buffer_local[i];
                    end
                end
            end
        end
    endgenerate


    always_ff @(posedge clk or negedge rst_n) begin
        if (~rst_n) begin
            total_packet_recieve_inner <= 0;
            total_latency <= 0;
        end
        else begin
            total_packet_recieve_inner <= total_packet_recieve_inner + recieve_cnt[0]+ recieve_cnt[1]+ recieve_cnt[2]+ recieve_cnt[3];
            total_latency <= total_latency + latency_add[0]+ latency_add[1]+ latency_add[2]+ latency_add[3];
        end
    end

    // next step: buffer write of input data, Router compute (for each packet, decide where it goes)
    // update backpressure, then switch allocator, for each direction, depending on backpressure, write to output wire
    reg [15 : 0] east_empty_num, west_empty_num;

    assign east_empty_num = BUFFER_SIZE - buffer_east[0][PACKET_SIZE - 1] - buffer_east[1][PACKET_SIZE - 1] - buffer_east[2][PACKET_SIZE - 1] - buffer_east[3][PACKET_SIZE - 1];
    assign west_empty_num = BUFFER_SIZE - buffer_west[0][PACKET_SIZE - 1] - buffer_west[1][PACKET_SIZE - 1] - buffer_west[2][PACKET_SIZE - 1] - buffer_west[3][PACKET_SIZE - 1];

    always_ff @(posedge clk or negedge rst_n) begin
        if (~rst_n) begin
            backpressure_east_wr <= 0;
            backpressure_west_wr <= 0;
        end else begin
            backpressure_east_wr <= east_empty_num >= BUFFER_THRESHOLD ? 1'b0 : 1'b1;
            backpressure_west_wr <= west_empty_num >= BUFFER_THRESHOLD ? 1'b0 : 1'b1;
        end
    end

    logic [PACKET_SIZE - 1 : 0] link_east_out_inner, link_west_out_inner;

    switch_allocator #(
        .OUT_PORT(2'b01),
        .PACKET_SIZE(PACKET_SIZE),
        .BUFFER_SIZE(BUFFER_SIZE)
    ) east_out (
        .clk(clk),
        .rst_n(rst_n),
        .backpressure(backpressure_east_rd),
        .buffer_high_prior(buffer_east),
        .buffer_low_prior(buffer_local),

        .out_packet(link_east_out_inner),
        .out_packet_pos(out_packet_pos_east),
        .out_packet_pos_valid(out_packet_pos_valid_east_out),
        .out_packet_pos_in_high(out_packet_pos_in_east)
    );

    switch_allocator #(
        .OUT_PORT(2'b10),
        .PACKET_SIZE(PACKET_SIZE),
        .BUFFER_SIZE(BUFFER_SIZE)
    ) west_out (
        .clk(clk),
        .rst_n(rst_n),
        .backpressure(backpressure_west_rd),
        .buffer_high_prior(buffer_west),
        .buffer_low_prior(buffer_local),

        .out_packet(link_west_out_inner),
        .out_packet_pos(out_packet_pos_west),
        .out_packet_pos_valid(out_packet_pos_valid_west_out),
        .out_packet_pos_in_high(out_packet_pos_in_west)
    );

    assign link_east_out = link_east_out_inner;
    assign link_west_out = link_west_out_inner;

endmodule
