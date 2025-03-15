`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 10/07/2024 08:07:57 PM
// Design Name: 
// Module Name: processing_element_tb
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

module ProcessingElement_tb;

    // Parameters
    parameter DATA_WIDTH   = 16;
    parameter WEIGHT_WIDTH = 16;
    parameter WEIGHT_DEPTH = 4;
    parameter CLK_PERIOD   = 40; // Clock period in ns (50 MHz clock)

    // Clock and Reset
    reg clk;
    reg rst_n;

    // AXI Stream Interfaces
    // Input Data Interface (s_axis)
    reg                      s_axis_tvalid;
    wire                     s_axis_tready;
    reg [DATA_WIDTH-1:0]     s_axis_tdata;

    // Output Data Interface (m_axis)
    wire                     m_axis_tvalid;
    reg                      m_axis_tready;
    wire [DATA_WIDTH-1:0]    m_axis_tdata;

    // Previous Stage Data Interface (v_data_in)
    reg                      v_data_in_tvalid;
    wire                     v_data_in_tready;
    reg [DATA_WIDTH-1:0]     v_data_in_tdata;

    // Computation Result Output Interface (v_data_out)
    wire                     v_data_out_tvalid;
    reg                      v_data_out_tready;
    wire [DATA_WIDTH-1:0]    v_data_out_tdata;

    // Weight Input Interface (v_weight_in)
    reg                      v_weight_in_tvalid;
    wire                     v_weight_in_tready;
    reg [WEIGHT_WIDTH-1:0]   v_weight_in_tdata;

    // Weight Output Interface (v_weight_out)
    wire                     v_weight_out_tvalid;
    reg                      v_weight_out_tready;
    wire [WEIGHT_WIDTH-1:0]  v_weight_out_tdata;

    // Weight Input Interface (weight_in)
    reg                      weight_in_tvalid;
    wire                     weight_in_tready;
    reg [WEIGHT_WIDTH-1:0]   weight_in;

    // Control Signals
    reg                      weight_load_enable;
    reg                      input_select;       // Not used 
    reg                      mode;               // 0 for addition, 1 for subtraction
    reg                      add_sub_enable;     // 1 to enable add/sub, 0 to bypass

    // Instantiate the ProcessingElement module
    ProcessingElement #(
        .DATA_WIDTH(DATA_WIDTH),
        .WEIGHT_WIDTH(WEIGHT_WIDTH),
        .WEIGHT_DEPTH(WEIGHT_DEPTH)
    ) uut (
        .clk(clk),
        .rst_n(rst_n),
        // AXI Stream Interfaces
        .s_axis_tvalid(s_axis_tvalid),
        .s_axis_tready(s_axis_tready),
        .s_axis_tdata(s_axis_tdata),
        .m_axis_tvalid(m_axis_tvalid),
        .m_axis_tready(m_axis_tready),
        .m_axis_tdata(m_axis_tdata),
        .v_data_in_tvalid(v_data_in_tvalid),
        .v_data_in_tready(v_data_in_tready),
        .v_data_in_tdata(v_data_in_tdata),
        .v_data_out_tvalid(v_data_out_tvalid),
        .v_data_out_tready(v_data_out_tready),
        .v_data_out_tdata(v_data_out_tdata),
        .v_weight_in_tvalid(v_weight_in_tvalid),
        .v_weight_in_tready(v_weight_in_tready),
        .v_weight_in_tdata(v_weight_in_tdata),
        .v_weight_out_tvalid(v_weight_out_tvalid),
        .v_weight_out_tready(v_weight_out_tready),
        .v_weight_out_tdata(v_weight_out_tdata),
        .weight_in(weight_in),
        .weight_in_tvalid(weight_in_tvalid),
        .weight_in_tready(weight_in_tready),
        .weight_load_enable(weight_load_enable),
        .input_select(input_select),
        .mode(mode),
        .add_sub_enable(add_sub_enable)
    );

    // Clock Generation
    initial begin
        clk = 1'b0;
        forever #(CLK_PERIOD/2) clk = ~clk;
    end

    // Reset Generation
    initial begin
        rst_n = 1'b0;
        #(CLK_PERIOD*5);
        rst_n = 1'b1;
    end

    // Testbench Variables
    integer i;
    reg [DATA_WIDTH-1:0] input_data [0:39];
    reg [DATA_WIDTH-1:0] v_data_input_data [0:39];
    reg [WEIGHT_WIDTH-1:0] weights [0:WEIGHT_DEPTH-1];

    // Arrays to Track Sent Data and Expected Outputs
    reg [DATA_WIDTH-1:0] sent_input_data [0:39];
    reg [WEIGHT_WIDTH-1:0] sent_weight_data [0:39];
    reg sent_mode [0:39]; // 1 bit for mode
    reg sent_add_sub_enable [0:39];
    real expected_output_real [0:39];
    real received_output_real;
    real expected_v_data_out;
    real diff;
    integer sent_index;
    integer m_axis_out_index;
    integer v_data_out_index;

    // Internal Variables for Weight Index Tracking
    integer weight_use_index;

    // Initialize Test Data
    initial begin
        // Initialize input data with specific values for verification
        // IEEE 754 half-precision representations
        // 16'h3C00 = +1.0, 16'h4000 = +2.0, 16'h4400 = +3.0
        // 16'hC000 = -2.0, 16'hBC00 = -1.0, 16'h4200 = +2.5
        // 16'h3800 = +0.5, 16'h3E00 = +0.75, 16'h4080 = +4.0, 16'hC200 = -3.0
        input_data[0]  = 16'h3C00; // +1.0
        input_data[1]  = 16'h4000; // +2.0
        input_data[2]  = 16'h4400; // +3.0
        input_data[3]  = 16'hC000; // -2.0
        input_data[4]  = 16'hBC00; // -1.0
        input_data[5]  = 16'h4200; // +2.5
        input_data[6]  = 16'h3800; // +0.5
        input_data[7]  = 16'h3E00; // +0.75
        input_data[8]  = 16'h4080; // +4.0
        input_data[9]  = 16'hC200; // -3.0
        // Repeat data for more iterations
        for (i = 10; i < 40; i = i + 1) begin
            input_data[i] = input_data[i % 10];
        end

        // Initialize vertical data (computation results from above)
        for (i = 0; i < 40; i = i + 1) begin
            v_data_input_data[i] = 16'h0000; // +0
        end

        // Initialize weights with predefined values
        weights[0] = 16'h4000; // +2.0
        weights[1] = 16'hC000; // -2.0
        weights[2] = 16'h3800; // +0.5
        weights[3] = 16'h4200; // +2.5

        // Initialize control signals
        weight_load_enable = 1'b0;
        input_select = 1'b1; // Not used in current module
        mode = 1'b0; // Addition
        add_sub_enable = 1'b1; // Enable addition/subtraction
        m_axis_tready = 1'b1;
        v_data_out_tready = 1'b1;
        v_weight_out_tready = 1'b1;

        // Initialize other signals
        s_axis_tvalid = 1'b0;
        s_axis_tdata  = {DATA_WIDTH{1'b0}};
        v_data_in_tvalid = 1'b0;
        v_data_in_tdata  = {DATA_WIDTH{1'b0}};
        v_weight_in_tvalid = 1'b0;
        v_weight_in_tdata  = {WEIGHT_WIDTH{1'b0}};
        weight_in_tvalid = 1'b0;
        weight_in        = {WEIGHT_WIDTH{1'b0}};

        // Initialize indices
        sent_index = 0;
        m_axis_out_index = 0;
        v_data_out_index = 0;

        // Wait for reset de-assertion
        @(posedge rst_n);

        // Load weights into the ProcessingElement
        for (i = 0; i < WEIGHT_DEPTH; i = i + 1) begin
            @(posedge clk);
            weight_in <= weights[i];
            weight_in_tvalid <= 1'b1;
            weight_load_enable <= 1'b1;

            // Wait for weight_in_tready to be high
            wait (weight_in_tready);
            @(posedge clk);
            // De-assert after transfer
            weight_in_tvalid <= 1'b0;
            weight_load_enable <= 1'b0;
            $display("Loaded Weight[%0d]: %h (%0f)", i, weights[i], to_real(weights[i]));
        end

        // Allow some time for weight propagation
        #(CLK_PERIOD*5);

        // Send input data via s_axis and vertical data via v_data_in simultaneously
        for (i = 0; i < 40; i = i + 1) begin
            @(posedge clk);
            s_axis_tdata <= input_data[i];
            s_axis_tvalid <= 1'b1;

            v_data_in_tdata <= v_data_input_data[i];
            v_data_in_tvalid <= 1'b1;

            // Keep track of which weight is being used
            weight_use_index = (i) % WEIGHT_DEPTH;

            // Compute expected output based on mode and add_sub_enable
            if (add_sub_enable) begin
                if (mode == 1'b0) begin
                    // Addition Mode: Output = (input * weight) + v_data_in_tdata
                    expected_output_real[sent_index] = to_real(input_data[i]) * to_real(weights[weight_use_index]) + to_real(v_data_input_data[i]);
                end else begin
                    // Subtraction Mode: Output = (input * weight) - to_real(v_data_in_tdata)
                    expected_output_real[sent_index] = to_real(input_data[i]) * to_real(weights[weight_use_index]) - to_real(v_data_input_data[i]);
                end 
            end else begin
                // Bypass Addition/Subtraction: Output = input * weight
                expected_output_real[sent_index] = to_real(input_data[i]) * to_real(weights[weight_use_index]);
            end

            sent_input_data[sent_index] = input_data[i];
            sent_weight_data[sent_index] = weights[weight_use_index];
            sent_mode[sent_index] = mode;
            sent_add_sub_enable[sent_index] = add_sub_enable;

            // Print the equation being tested
            $display("=== Test Iteration %0d ===", sent_index);
            $display("Input: %h (%0f)", input_data[i], to_real(input_data[i]));
            $display("Vertical Input: %h (%0f)", v_data_input_data[i], to_real(v_data_input_data[i]));
            $display("Weight Used: %h (%0f)", weights[weight_use_index], to_real(weights[weight_use_index]));
            $display("Mode: %s", mode ? "Subtraction" : "Addition");
            $display("Add/Sub Enable: %b", add_sub_enable);
            $display("Expected Output: %0f", expected_output_real[sent_index]);
            $display("----------------------------");

            // Wait for both s_axis_tready and v_data_in_tready to be high
            wait (s_axis_tready && v_data_in_tready);
            @(posedge clk);
            // De-assert after transfer
            s_axis_tvalid <= 1'b0;
            v_data_in_tvalid <= 1'b0;

            sent_index = sent_index + 1;
        end

        // Allow some time for outputs to propagate
        #(CLK_PERIOD*20);

        // Finish simulation after all data is processed
        wait (v_data_out_index >= sent_index && m_axis_out_index >= sent_index);
        #(CLK_PERIOD*10);
        $display("Ending Simulation");
        $finish;
    end

    // Helper Function: Convert 16-bit IEEE 754 Half-Precision to Real Number
    function real to_real(input [15:0] fp);
        reg sign;
        reg [4:0] exponent;
        reg [9:0] fraction;
        real value;
        begin
            sign = fp[15];
            exponent = fp[14:10];
            fraction = fp[9:0];
            if (exponent == 0 && fraction == 0)
                value = 0.0;
            else if (exponent == 0)
                value = $pow(-1, sign) * $pow(2, -14) * (fraction / 1024.0);
            else if (exponent == 31) begin
                if (fraction == 0)
                    value = ($pow(-1, sign) * (1.0/0.0)); // Infinity
                else
                    value = ($pow(-1, sign) * 0.0); // NaN treated as zero
            end
            else
                value = $pow(-1, sign) * $pow(2, exponent - 15) * (1 + (fraction / 1024.0));
            to_real = value;
        end
    endfunction

    // Monitor Output Data
    initial begin
        $monitor("Time: %t | m_axis_tvalid: %b | m_axis_tdata: %h (%0f) | v_data_out_tvalid: %b | v_data_out_tdata: %h (%0f) | v_weight_out_tvalid: %b | v_weight_out_tdata: %h (%0f)", 
                 $time, m_axis_tvalid, m_axis_tdata, to_real(m_axis_tdata), 
                 v_data_out_tvalid, v_data_out_tdata, to_real(v_data_out_tdata), 
                 v_weight_out_tvalid, v_weight_out_tdata, to_real(v_weight_out_tdata));
    end

    // Capture and Verify Output Data
    always @(posedge clk) begin
        // Verify m_axis outputs (Horizontal Data Passing)
        if (m_axis_tvalid && m_axis_tready && m_axis_out_index < sent_index) begin
            if (m_axis_tdata == sent_input_data[m_axis_out_index]) begin
                $display("m_axis Output Verification [Iteration %0d]: PASS", m_axis_out_index);
            end else begin
                $display("m_axis Output Verification [Iteration %0d]: FAIL", m_axis_out_index);
            end
            m_axis_out_index = m_axis_out_index + 1;
        end

        // Verify v_data_out outputs (Computation Result)
        if (v_data_out_tvalid && v_data_out_tready && v_data_out_index < sent_index) begin
            received_output_real = to_real(v_data_out_tdata);
            expected_v_data_out = expected_output_real[v_data_out_index];
            diff = received_output_real - expected_v_data_out;
            if (diff < 0) diff = -diff;
            if (diff < 0.001) begin
                $display("v_data_out Verification [Iteration %0d]: PASS", v_data_out_index);
                $display("Received: %0f | Expected: %0f", received_output_real, expected_v_data_out);
            end else begin
                $display("v_data_out Verification [Iteration %0d]: FAIL", v_data_out_index);
                $display("Received: %0f | Expected: %0f", received_output_real, expected_v_data_out);
            end
            v_data_out_index = v_data_out_index + 1;
        end

        // Verify v_weight_out outputs (Weight Propagation)
        if (v_weight_out_tvalid && v_weight_out_tready) begin
            $display("v_weight_out: %h (%0f)", v_weight_out_tdata, to_real(v_weight_out_tdata));
            // Optional: Verify if the weight being propagated is correct
        end
    end

endmodule


