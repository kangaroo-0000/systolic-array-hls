#ifndef SYSTOLIC_ARRAY_H
#define SYSTOLIC_ARRAY_H

#include <hls_stream.h>
#include "pe_cycle.h"
#include "globals.h"

// Top-level function
void systolic_array(
    hls::stream<MBItem> &inputStream,
    hls::stream<MBItem> &outputStream,
    hls::stream<float> &aStream,
    hls::stream<float> &bStream,
    bool accumulateMode,
    int totalCycles
);

#endif // SYSTOLIC_ARRAY_H
