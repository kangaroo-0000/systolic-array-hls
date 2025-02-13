#include "systolic_array.h"

void systolic_array(
    hls::stream<MBItem> &inputStream,
    hls::stream<MBItem> &outputStream,
    hls::stream<float> &aStream,
    hls::stream<float> &bStream,
    bool accumulateMode,
    int totalCycles
) {
#pragma HLS INTERFACE axis     port=inputStream
#pragma HLS INTERFACE axis     port=outputStream
#pragma HLS INTERFACE axis     port=aStream
#pragma HLS INTERFACE axis     port=bStream
#pragma HLS INTERFACE s_axilite port=accumulateMode
#pragma HLS INTERFACE s_axilite port=totalCycles
#pragma HLS INTERFACE s_axilite port=return

    static PEState peState[NUM_PEs];
#pragma HLS ARRAY_PARTITION variable=peState complete dim=1

    // Streams between PEs
    static hls::stream<MBItem> peStreams[NUM_PEs + 1];
#pragma HLS STREAM variable=peStreams depth=32
#pragma HLS ARRAY_PARTITION variable=peStreams complete dim=1

    // 1) Move all items from inputStream => peStreams[0] before we start
    while (!inputStream.empty()) {
#pragma HLS PIPELINE II=1
        peStreams[0].write(inputStream.read());
    }

    // 2) Run for totalCycles
    for(int cycle = 0; cycle < totalCycles; cycle++) {
#pragma HLS PIPELINE II=1

        // (a) Read horizA/horizB if available
        for (int i = 0; i < NUM_PEs; i++) {
#pragma HLS UNROLL
            if (!aStream.empty() && !bStream.empty()) {
                peState[i].horizA = aStream.read();
                peState[i].horizB = bStream.read();
            }
        }

        // (b) Each PE does one pe_cycle
        for(int i = 0; i < NUM_PEs; i++) {
#pragma HLS UNROLL
            pe_cycle(
                i,               // myIndex
                NUM_PEs,         // totalPEs
                cycle,
                peState[i],
                peStreams[i+1],  // downStream
                peStreams[i],    // inputStream
                accumulateMode
            );
        }
    }

    // 3) Finally flush what's left in peStreams[NUM_PEs]
    while(!peStreams[NUM_PEs].empty()) {
#pragma HLS PIPELINE II=1
        outputStream.write(peStreams[NUM_PEs].read());
    }
}
