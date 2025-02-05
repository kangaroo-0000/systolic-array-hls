// ==============================================================
// Generated by Vitis HLS v2023.2
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// ==============================================================
`timescale 1 ns / 1 ps
module systolic_array_systolic_array_stream_MBItem_0_stream_MBItem_0_bool_int_peState_6_RAM_AUTO_1R1W (
     
    address0, ce0,
    d0, we0, 
    q0, 
     
    reset, clk);

parameter DataWidth = 1;
parameter AddressWidth = 3;
parameter AddressRange = 8;
 
input[AddressWidth-1:0] address0;
input ce0;
input[DataWidth-1:0] d0;
input we0; 
output reg[DataWidth-1:0] q0; 

input reset;
input clk;

(* ram_style = "auto"  *)reg [DataWidth-1:0] ram[0:AddressRange-1];

initial begin
    $readmemh("./systolic_array_systolic_array_stream_MBItem_0_stream_MBItem_0_bool_int_peState_6_RAM_AUTO_1R1W.dat", ram);
end 

 





//read first
always @(posedge clk)  
begin 
    if (ce0) begin
        if (we0) 
            ram[address0] <= d0; 
        q0 <= ram[address0];

    end
end 
 
 

endmodule

