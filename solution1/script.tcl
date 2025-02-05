############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project final_systolic_array_hls
set_top systolic_array
add_files final_systolic_array_hls/mb_item.h
add_files final_systolic_array_hls/pe_cycle.h
add_files final_systolic_array_hls/pe_state.h
add_files final_systolic_array_hls/systolic_array.cpp
add_files final_systolic_array_hls/systolic_array.h
add_files -tb final_systolic_array_hls/systolic_array_elementwise_tb.cpp
open_solution "solution1" -flow_target vivado
set_part {xcvu11p-flga2577-1-e}
create_clock -period 10 -name default
source "./final_systolic_array_hls/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
