#ifndef SYSTOLIC_ARRAY_H
#define SYSTOLIC_ARRAY_H

#include <hls_stream.h>
#include <queue>
#include "pe_cycle.h"

// Define the number of Processing Elements (PEs)
static const int NUM_PEs = 2;

// Declare the top-level function for synthesis
void systolic_array(
    hls::stream<MBItem> &inputStream,
    hls::stream<MBItem> &outputStream,
    bool accumulateMode,
    int totalCycles
);

#endif // SYSTOLIC_ARRAY_H