`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/14/2022 09:55:03 AM
// Design Name: 
// Module Name: rgbled
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module rgbled(
    output reg [2:0] rgb,
    input wire [6:0] brightness,
    input wire [7:0] red,
    input wire [7:0] green,
    input wire [7:0] blue,
    input wire clk
    );
    
    // cycle counter
    reg [14:0] counter;
    
    // number of clock cycles after which we restart the duty cycle (controls brightness)
    wire [14:0] cycles;
    
    assign cycles = brightness > 100 ? 255
                                     : (brightness == 0 ? 0
                                                        : 255 * 100 / brightness);
    
    // increment counter or reset if it reached value of 'cycles'
    always @(posedge clk) begin
        if (counter < cycles) begin
            counter <= counter + 1;
        end
        else begin
            counter = 0;
        end
    end
    
    // turn colour on if value its value is smaller than the cycle counter
    always @(posedge clk) begin
        if (cycles > 0) begin
            rgb[0] <= red   > counter ? 1 : 0;
            rgb[1] <= blue  > counter ? 1 : 0;
            rgb[2] <= green > counter ? 1 : 0;
        end
        else begin
            // brightness = 0 -> turn off all colours
            rgb <= 3'b000;
        end
    end
    
    
endmodule
