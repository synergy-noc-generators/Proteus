`timescale 1ns / 1ps

module switch_allocator #(
    parameter OUT_PORT = 2'b01,
    parameter PACKET_SIZE = 49,
    parameter BUFFER_SIZE = 4
)(
    clk,
    rst_n,
    backpressure,
    buffer_high_prior,
    buffer_high_prior_route_info,
    buffer_low_prior,
    buffer_low_prior_route_info,
    out_packet,
    out_packet_pos,
    out_packet_pos_valid,
    out_packet_pos_in_high
);

    input clk, rst_n;

    input backpressure;

    input [PACKET_SIZE - 1 : 0] buffer_high_prior [BUFFER_SIZE - 1 : 0];
    input [PACKET_SIZE - 1 : 0] buffer_low_prior [BUFFER_SIZE - 1 : 0];

    input [1 : 0] buffer_high_prior_route_info [BUFFER_SIZE - 1 : 0];
    input [1 : 0] buffer_low_prior_route_info [BUFFER_SIZE - 1 : 0];


    output logic [PACKET_SIZE -1 : 0] out_packet;
    output [15 : 0]             out_packet_pos;
    output                      out_packet_pos_valid;
    output                      out_packet_pos_in_high;

    logic [15 : 0] min_high_temp0, min_high_temp1, min_high;
    logic min_high_temp0_valid, min_high_temp1_valid, min_high_valid;

    always_comb begin
        if (~rst_n || backpressure) begin
            min_high_temp0 = 0;
            min_high_temp0_valid = 0;
        end
        else begin
            if (buffer_high_prior[0][PACKET_SIZE - 1] == 0 && buffer_high_prior[1][PACKET_SIZE - 1] == 1) begin
                min_high_temp0 = 1;
                min_high_temp0_valid = 1;
            end
            else if (buffer_high_prior[0][PACKET_SIZE - 1] == 1 && buffer_high_prior[1][PACKET_SIZE - 1] == 0) begin
                min_high_temp0 = 0;
                min_high_temp0_valid = 1;
            end
            else if (buffer_high_prior[0][PACKET_SIZE - 1] == 1 && buffer_high_prior[1][PACKET_SIZE - 1] == 1) begin
                min_high_temp0 = (buffer_high_prior[0][47 : 32] >= buffer_high_prior[1][47 : 32]) ? 1 : 0;
                min_high_temp0_valid = 1;
            end
            else begin
                min_high_temp0 = 0;
                min_high_temp0_valid = 0;
            end
        end
    end

    always_comb begin
        if (~rst_n || backpressure) begin
            min_high_temp1 = 0;
            min_high_temp1_valid = 0;
        end
        else begin
            if (buffer_high_prior[2][PACKET_SIZE - 1] == 0 && buffer_high_prior[3][PACKET_SIZE - 1] == 1) begin
                min_high_temp1 = 3;
                min_high_temp1_valid = 1;
            end
            else if (buffer_high_prior[2][PACKET_SIZE - 1] == 1 && buffer_high_prior[3][PACKET_SIZE - 1] == 0) begin
                min_high_temp1 = 2;
                min_high_temp1_valid = 1;
            end
            else if (buffer_high_prior[2][PACKET_SIZE - 1] == 1 && buffer_high_prior[3][PACKET_SIZE - 1] == 1) begin
                min_high_temp1 = buffer_high_prior[2][47 : 32] >= buffer_high_prior[3][47 : 32] ? 3 : 2;
                min_high_temp1_valid = 1;
            end
            else begin
                min_high_temp1 = 0;
                min_high_temp1_valid = 0;
            end
        end
    end

    always_comb begin
        if (~rst_n || backpressure) begin
            min_high = 0;
            min_high_valid = 0;
        end
        else begin
            if (!min_high_temp0_valid && min_high_temp1_valid) begin
                min_high = min_high_temp1;
                min_high_valid = min_high_temp1_valid;
            end
            else if (min_high_temp0_valid && !min_high_temp1_valid) begin
                min_high = min_high_temp0;
                min_high_valid = min_high_temp0_valid;
            end
            else if (min_high_temp0_valid && min_high_temp1_valid) begin
                min_high = buffer_high_prior[min_high_temp0] >= buffer_high_prior[min_high_temp1] ? min_high_temp1 : min_high_temp0;
                min_high_valid = 1;
            end
            else begin
                min_high = 0;
                min_high_valid = 0;
            end
        end
    end

    logic [15 : 0] min_low_temp0, min_low_temp1, min_low;
    logic min_low_temp0_valid, min_low_temp1_valid, min_low_valid;

    always_comb begin
        if (~rst_n || backpressure) begin
            min_low_temp0 = 0;
            min_low_temp0_valid = 0;
        end
        else begin
            if (buffer_low_prior[0][PACKET_SIZE - 1] == 0 && buffer_low_prior[1][PACKET_SIZE - 1] == 1) begin
                min_low_temp0 = 1;
                min_low_temp0_valid = 1;
            end
            else if (buffer_low_prior[0][PACKET_SIZE - 1] == 1 && buffer_low_prior[1][PACKET_SIZE - 1] == 0) begin
                min_low_temp0 = 0;
                min_low_temp0_valid = 1;
            end
            else if (buffer_low_prior[0][PACKET_SIZE - 1] == 1 && buffer_low_prior[1][PACKET_SIZE - 1] == 1) begin
                min_low_temp0 = buffer_low_prior[0][47 : 32] >= buffer_low_prior[1][47 : 32] ? 1 : 0;
                min_low_temp0_valid = 1;
            end
            else begin
                min_low_temp0 = 0;
                min_low_temp0_valid = 0;
            end
        end
    end

    always_comb begin
        if (~rst_n || backpressure) begin
            min_low_temp1 = 0;
            min_low_temp1_valid = 0;
        end
        else begin
            if (buffer_low_prior[2][PACKET_SIZE - 1] == 0 && buffer_low_prior[3][PACKET_SIZE - 1] == 1) begin
                min_low_temp1 = 3;
                min_low_temp1_valid = 1;
            end
            else if (buffer_low_prior[2][PACKET_SIZE - 1] == 1 && buffer_low_prior[3][PACKET_SIZE - 1] == 0) begin
                min_low_temp1 = 2;
                min_low_temp1_valid = 1;
            end
            else if (buffer_low_prior[2][PACKET_SIZE - 1] == 1 && buffer_low_prior[3][PACKET_SIZE - 1] == 1) begin
                min_low_temp1 = buffer_low_prior[2][47 : 32] >= buffer_low_prior[3][47 : 32] ? 3 : 2;
                min_low_temp1_valid = 1;
            end
            else begin
                min_low_temp1 = 0;
                min_low_temp1_valid = 0;
            end
        end
    end

    always_comb begin
        if (~rst_n || backpressure) begin
            min_low = 0;
            min_low_valid = 0;
        end
        else begin
            if (!min_low_temp0_valid && min_low_temp1_valid) begin
                min_low = min_low_temp1;
                min_low_valid = min_low_temp1_valid;
            end
            else if (min_low_temp0_valid && !min_low_temp1_valid) begin
                min_low = min_low_temp0;
                min_low_valid = min_low_temp0_valid;
            end
            else if (min_low_temp0_valid && min_low_temp1_valid) begin
                min_low = buffer_low_prior[min_low_temp0] >= buffer_low_prior[min_low_temp1] ? min_low_temp1 : min_low_temp0;
                min_low_valid = 1;
            end
            else begin
                min_low = 0;
                min_low_valid = 0;
            end
        end
    end

    reg [PACKET_SIZE -1 : 0] out_packet_inner;
    reg [15 : 0]             out_packet_pos_inner;
    reg                      out_packet_pos_valid_inner;
    reg                      out_packet_pos_in_high_inner;


    always_ff @(posedge clk or negedge rst_n) begin
        if (~rst_n ) begin
            out_packet_inner <= 0;
        end
        else if(backpressure) begin
            out_packet_inner <= 0;
        end
        else begin
            if (min_high_valid && buffer_high_prior_route_info[min_high] != 0) begin
                out_packet_inner <= buffer_high_prior[min_high];
            end
            else begin
                if (min_low_valid && buffer_low_prior_route_info[min_low] == OUT_PORT) begin
                    out_packet_inner <= buffer_low_prior[min_low];
                end
                else begin
                    out_packet_inner <= 0;
                end
            end
        end
    end

    always_comb begin
        if (~rst_n || backpressure) begin
            out_packet_pos_inner = 0;
            out_packet_pos_valid_inner = 0;
            out_packet_pos_in_high_inner = 0;
        end
        else begin
            if (min_high_valid && buffer_high_prior_route_info[min_high] != 0) begin
               out_packet_pos_inner = min_high;
               out_packet_pos_valid_inner = min_high_valid;
               out_packet_pos_in_high_inner = 1'b1;
            end
            else begin
                if (min_low_valid && buffer_low_prior_route_info[min_low] == OUT_PORT) begin
                    out_packet_pos_inner = min_low;
                    out_packet_pos_valid_inner = min_low_valid;
                    out_packet_pos_in_high_inner = 1'b0;
                end
                else begin
                    out_packet_pos_inner = 0;
                    out_packet_pos_valid_inner = 0;
                    out_packet_pos_in_high_inner = 0;
                end
            end
        end
    end


    assign out_packet = out_packet_inner;
    assign out_packet_pos = out_packet_pos_inner;
    assign out_packet_pos_valid = out_packet_pos_valid_inner;
    assign out_packet_pos_in_high = out_packet_pos_in_high_inner;


endmodule
