#include "systolic_array.h"

void systolic_array(
    hls::stream<MBItem> &inputStream,
    hls::stream<MBItem> &outputStream,
    bool accumulateMode,
    int totalCycles
) {
#pragma HLS INTERFACE axis port=inputStream
#pragma HLS INTERFACE axis port=outputStream
#pragma HLS INTERFACE s_axilite port=accumulateMode
#pragma HLS INTERFACE s_axilite port=totalCycles
#pragma HLS INTERFACE s_axilite port=return

    static PEState peState[NUM_PEs];
#pragma HLS ARRAY_PARTITION variable=peState complete dim=1

    static hls::stream<MBItem> peStreams[NUM_PEs + 1];
#pragma HLS STREAM variable=peStreams depth=32
#pragma HLS ARRAY_PARTITION variable=peStreams complete dim=1

    // Transfer input data to first PE's stream
    while(!inputStream.empty()) {
#pragma HLS PIPELINE II=1
        peStreams[0].write(inputStream.read());
    }

    // Initialize horizontal inputs
    peState[0].horizA = 1.f;
    peState[0].horizB = 1.f;
    peState[1].horizA = 2.f;
    peState[1].horizB = 2.f;

    // Process data through PE array
    for(int cycle = 0; cycle < totalCycles; cycle++) {
#pragma HLS PIPELINE II=1
        for(int i = 0; i < NUM_PEs; i++) {
#pragma HLS UNROLL
            pe_cycle(
                i,
                NUM_PEs,
                cycle,
                peState[i],
                peStreams[i+1],
                peStreams[i],
                accumulateMode
            );
        }
    }

    // Flush final results to output
    while(!peStreams[NUM_PEs].empty()) {
#pragma HLS PIPELINE II=1
        outputStream.write(peStreams[NUM_PEs].read());
    }
}