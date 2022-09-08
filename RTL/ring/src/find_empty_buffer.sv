`timescale 1ns / 1ps


module find_empty_buffer #(
    parameter BUFFER_SIZE = 4,
    parameter PACKET_SIZE = 49,
    parameter PTR_LEN = 2
) (
    input clk,
    input rst_n,

    input [PACKET_SIZE - 1 : 0] buffer [BUFFER_SIZE - 1 : 0],
    
    output logic [PTR_LEN - 1 : 0]    empty_pos,
    output logic empty_pos_found
);

    reg [PTR_LEN - 1 : 0] empty_pos_inner;
    reg                   empty_pos_found_inner;  

    // hard code as buffer size of 4, index 0 takes the priority
    always_comb begin : check_empty
        if (buffer[0][PACKET_SIZE - 1] == 1'b0) begin
            empty_pos_inner = 2'b00;
            empty_pos_found_inner = 1'b1;
        end
        else if (buffer[1][PACKET_SIZE - 1] == 1'b0) begin
            empty_pos_inner = 2'b01;
            empty_pos_found_inner = 1'b1;
        end
        else if (buffer[2][PACKET_SIZE - 1] == 1'b0) begin
            empty_pos_inner = 2'b10;
            empty_pos_found_inner = 1'b1;
        end
        else if (buffer[3][PACKET_SIZE - 1] == 1'b0) begin
            empty_pos_inner = 2'b11;
            empty_pos_found_inner = 1'b1;
        end
        else begin
            empty_pos_inner = 2'b00;
            empty_pos_found_inner = 1'b0;
        end
    end

    assign empty_pos = empty_pos_inner;
    assign empty_pos_found = empty_pos_found_inner;

endmodule
