`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: NYU
// Engineer: Chung-Mou Pan
// 
// Create Date: 10/07/2024 08:03:33 PM
// Design Name: 
// Module Name: processing_element
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

module ProcessingElement #(
    parameter DATA_WIDTH   = 16, // 16-bit floating-point (half-precision)
    parameter WEIGHT_WIDTH = 16, // 16-bit floating-point
    parameter WEIGHT_DEPTH = 4   // Number of weights in the register file
)(
    input  wire                      clk,
    input  wire                      rst_n,
    
    // AXI Stream Slave Interface for Horizontal Data
    input  wire                      s_axis_tvalid,
    output wire                      s_axis_tready,
    input  wire [DATA_WIDTH-1:0]     s_axis_tdata,
    
    // AXI Stream Master Interface for Horizontal Data
    output wire                      m_axis_tvalid,
    input  wire                      m_axis_tready,
    output wire [DATA_WIDTH-1:0]     m_axis_tdata,
    
    // AXI Stream Slave Interface for Vertical Data (Computation Result from Above)
    input  wire                      v_data_in_tvalid,
    output wire                      v_data_in_tready,
    input  wire [DATA_WIDTH-1:0]     v_data_in_tdata,
    
    // AXI Stream Master Interface for Vertical Data (Computation Result to Below)
    output wire                      v_data_out_tvalid,
    input  wire                      v_data_out_tready,
    output wire [DATA_WIDTH-1:0]     v_data_out_tdata,
    
    // AXI Stream Slave Interface for Vertical Weight (Weight from Above)
    input  wire                      v_weight_in_tvalid,
    output wire                      v_weight_in_tready,
    input  wire [WEIGHT_WIDTH-1:0]   v_weight_in_tdata,
    
    // AXI Stream Master Interface for Vertical Weight (Weight to Below)
    output wire                      v_weight_out_tvalid,
    input  wire                      v_weight_out_tready,
    output wire [WEIGHT_WIDTH-1:0]   v_weight_out_tdata,
    
    // Control Signals
    input  wire [WEIGHT_WIDTH-1:0]   weight_in,          // Weight input (external loading)
    input  wire                      weight_in_tvalid,
    output wire                      weight_in_tready,
    input  wire                      weight_load_enable, // Control signal for weight loading phase
    input  wire                      input_select,       // MUX control signal for horizontal data flow (unused in this code)
    input  wire                      mode,               // 0 for addition, 1 for subtraction
    input  wire                      add_sub_enable      // 1 to enable add/sub, 0 to bypass
);

    // Internal registers and wires
    reg [DATA_WIDTH-1:0]             selected_h_input;
    reg [DATA_WIDTH-1:0]             v_data_in_reg;
    reg [WEIGHT_WIDTH-1:0]           selected_weight;
    
    reg                              s_axis_valid_reg;
    reg                              v_data_in_valid_reg;
    reg                              v_weight_in_valid_reg;
    reg                              valid_internal;
    
    // Weight register file and management
    reg [WEIGHT_WIDTH-1:0]           weight_reg_file [0:WEIGHT_DEPTH-1];
    reg [WEIGHT_DEPTH-1:0]           weight_slot_full;
    
    // Indices for computation and propagation
    reg [$clog2(WEIGHT_DEPTH)-1:0]   comp_weight_index;       // For weight loading
    reg [$clog2(WEIGHT_DEPTH)-1:0]   comp_weight_read_index;  // For computation
    reg [$clog2(WEIGHT_DEPTH)-1:0]   prop_weight_index;

    // Weight Loading and Propagation Logic
    integer i;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            // Initialize weight register file and indices
            for (i = 0; i < WEIGHT_DEPTH; i = i + 1) begin
                weight_reg_file[i] <= {WEIGHT_WIDTH{1'b0}};
            end
            weight_slot_full        <= {WEIGHT_DEPTH{1'b0}};
            comp_weight_index       <= 0;
            comp_weight_read_index  <= 0;
            prop_weight_index       <= 0;
        end else begin
            // Weight Loading
            if (weight_load_enable && weight_in_tvalid && weight_in_tready) begin
                if (!weight_slot_full[comp_weight_index]) begin
                    weight_reg_file[comp_weight_index] <= weight_in;
                    weight_slot_full[comp_weight_index] <= 1'b1;
                    comp_weight_index <= (comp_weight_index + 1) % WEIGHT_DEPTH;
                end
            end

            // Weight Propagation
            if (v_weight_in_tvalid && v_weight_in_tready) begin
                if (!weight_slot_full[prop_weight_index]) begin
                    weight_reg_file[prop_weight_index] <= v_weight_in_tdata;
                    weight_slot_full[prop_weight_index] <= 1'b1;
                    prop_weight_index <= (prop_weight_index + 1) % WEIGHT_DEPTH;
                end
            end

            // Clear slot after passing weight down logic 
        end
    end

    // Flow Control for Weight Input
    assign v_weight_in_tready = !(&weight_slot_full);

    // Weight Output (Propagation)
    assign v_weight_out_tdata  = weight_reg_file[prop_weight_index];
    assign v_weight_out_tvalid = weight_slot_full[prop_weight_index];

    // Weight Input Ready Signal
    assign weight_in_tready = weight_load_enable && (!(&weight_slot_full));

    // Capture s_axis input
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            selected_h_input <= {DATA_WIDTH{1'b0}};
            s_axis_valid_reg <= 1'b0;
        end else if (s_axis_tvalid && s_axis_tready) begin
            selected_h_input <= s_axis_tdata;
            s_axis_valid_reg <= 1'b1;
        end else if (valid_internal && m_axis_tready && v_data_out_tready) begin
            s_axis_valid_reg <= 1'b0;
        end
    end

    // Capture v_data_in input
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            v_data_in_reg <= {DATA_WIDTH{1'b0}};
            v_data_in_valid_reg <= 1'b0;
        end else if (v_data_in_tvalid && v_data_in_tready) begin
            v_data_in_reg <= v_data_in_tdata;
            v_data_in_valid_reg <= 1'b1;
        end else if (valid_internal && m_axis_tready && v_data_out_tready) begin
            v_data_in_valid_reg <= 1'b0;
        end
    end

    // Selected Weight for Computation
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            selected_weight <= {WEIGHT_WIDTH{1'b0}};
            v_weight_in_valid_reg <= 1'b0;
        end else if (weight_slot_full[comp_weight_read_index]) begin
            selected_weight <= weight_reg_file[comp_weight_read_index];
            v_weight_in_valid_reg <= 1'b1;
        end else begin
            v_weight_in_valid_reg <= 1'b0;
        end
    end

    // Set valid_internal when all inputs are valid
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            valid_internal <= 1'b0;
            comp_weight_read_index <= 0; // Initialize the computation index
        end else if (s_axis_valid_reg && v_data_in_valid_reg && v_weight_in_valid_reg) begin
            valid_internal <= 1'b1;
        end else if (valid_internal && m_axis_tready && v_data_out_tready) begin
            valid_internal <= 1'b0;
            // Increment comp_weight_read_index after each computation
            comp_weight_read_index <= (comp_weight_read_index + 1) % WEIGHT_DEPTH;
        end
    end

    // Ready signals
    assign s_axis_tready     = !s_axis_valid_reg;
    assign v_data_in_tready  = !v_data_in_valid_reg;
    // v_weight_in_tready is defined above

    // Instantiate the floating-point multiplier IP core (floating_point_0)
    wire fp_mul_valid;
    wire [DATA_WIDTH-1:0] fp_mul_result;
    wire fp_mul_ready = add_sub_enable ? 1'b1 : (m_axis_tready && v_data_out_tready);

    floating_point_0 fp_multiplier_inst (
        .aclk(clk),
        .s_axis_a_tvalid(s_axis_valid_reg),
        .s_axis_a_tready(),
        .s_axis_a_tdata(selected_h_input),
        .s_axis_b_tvalid(v_weight_in_valid_reg),
        .s_axis_b_tready(),
        .s_axis_b_tdata(selected_weight),
        .m_axis_result_tvalid(fp_mul_valid),
        .m_axis_result_tready(fp_mul_ready),
        .m_axis_result_tdata(fp_mul_result)
    );

    // Instantiate the floating-point adder/subtractor IP core (floating_point_1)
    wire fp_add_sub_valid;
    wire [DATA_WIDTH-1:0] fp_add_sub_result;
    wire fp_add_sub_ready = m_axis_tready && v_data_out_tready;

    floating_point_1 fp_addsub_inst (
        .aclk(clk),
        .s_axis_a_tvalid(fp_mul_valid && add_sub_enable),
        .s_axis_a_tready(),
        .s_axis_a_tdata(fp_mul_result),
        .s_axis_b_tvalid(v_data_in_valid_reg && add_sub_enable),
        .s_axis_b_tready(),
        .s_axis_b_tdata(v_data_in_reg),
        .s_axis_operation_tvalid(fp_mul_valid && v_data_in_valid_reg && add_sub_enable),
        .s_axis_operation_tready(),
        .s_axis_operation_tdata({7'b0, mode}), // Assuming operation is 8 bits with mode as LSB
        .m_axis_result_tvalid(fp_add_sub_valid),
        .m_axis_result_tready(fp_add_sub_ready),
        .m_axis_result_tdata(fp_add_sub_result)
    );

    // Multiplexer for Output Selection
    wire [DATA_WIDTH-1:0] computation_result;
    assign computation_result = add_sub_enable ? fp_add_sub_result : fp_mul_result;

    // Output Assignment
    assign m_axis_tvalid = s_axis_valid_reg;
    assign m_axis_tdata  = s_axis_tdata;

    // Vertical Data Output (Computation Result)
    assign v_data_out_tvalid = add_sub_enable ? fp_add_sub_valid : fp_mul_valid;
    assign v_data_out_tdata  = computation_result;

endmodule

