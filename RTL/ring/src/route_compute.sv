`timescale 1ns / 1ps

module route_info_update #(
    parameter ROUTER_ID = 0,
    parameter ROUTING = 0, // 0 means random oblivous
    parameter PACKET_SIZE = 49,
    parameter IN_PORT = 0, // 0x0 local, 0x1 east, 0x2 west
)(
    clk,
    rst_n,
    in_buffer,
    route_update_en,
    out_dir
);

    input clk, rst_n;

    input [PACKET_SIZE - 1] in_buffer;
    input                   route_update_en;
    
    output [1 : 0] out_dir;

    reg [1 : 0] out_dir_inner;


    always_ff @(posedge clk or negedge rst_n) begin
        if (~rst_n) begin
            out_dir_inner <= 2'b00;
        end
        else if (route_update_en) begin
            if (ROUTING == 0) begin
                if (IN_PORT == 2'b01) begin // east
                    if (in_buffer[15 : 0] == ROUTER_ID) begin
                        out_dir_inner <= 2'b00; // local -> evict
                    end
                    else begin
                        out_dir_inner <= 2'b10;
                    end
                end
                else if (IN_PORT == 2'b10) begin // west
                    if (in_buffer[15 : 0] == ROUTER_ID) begin
                        out_dir_inner <= 2'b00; // local -> evict
                    end 
                    else begin
                        out_dir_inner <= 2'b01;
                    end
                end else begin // local, just use the LSB of timestamp to decide
                    if (in_buffer[15 : 0] == ROUTER_ID) begin
                        out_dir_inner <= 2'b00; // local -> evict
                    end 
                    else begin
                        out_dir_inner <= in_buffer[32] == 1'b1 ? 2'b10 : 2'b01;
                    end
                end
            end
            else begin
                out_dir_inner <= out_dir_inner;
            end
        end
        else begin
            out_dir_inner <= out_dir_inner;
        end
    end

    assign out_dir = out_dir_inner;

endmodule