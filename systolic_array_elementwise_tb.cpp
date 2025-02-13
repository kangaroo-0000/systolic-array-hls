#include <iostream>
#include <iomanip>
#include "systolic_array.h"

int main()
{
    // Create streams
    hls::stream<MBItem> inputStream("inputStream");
    hls::stream<MBItem> outputStream("outputStream");
    hls::stream<float>  aStream("aStream");
    hls::stream<float>  bStream("bStream");

    bool accumulateMode = false;

    // ---------------------------
    // (A) Load weights into PEs
    // ---------------------------
    //   PE0 => (p0,q0) = (2,4)
    //   PE1 => (p1,q1) = (3,5)

    inputStream.write(MBItem(
        /*isWeight=*/true, /*isRealPart=*/true, /*needsMultiply=*/false,
        /*destPE=*/0, 2.f, 0.f, false, /*tag=*/101
    ));
    inputStream.write(MBItem(
        true, false, false,
        0, 0.f, 4.f, false, 102
    ));
    inputStream.write(MBItem(
        true, true, false,
        1, 3.f, 0.f, false, 103
    ));
    inputStream.write(MBItem(
        true, false, false,
        1, 0.f, 5.f, false, 104
    ));

    // First call => e.g. 5 cycles
    std::cout << "\n======== LOAD WEIGHTS PHASE ========\n";
    systolic_array(
        inputStream, outputStream, 
        aStream, bStream, // we are not feeding a/b yet
        accumulateMode,
        5
    );

    // Drain any leftover from the output stream after the load phase
    while (!outputStream.empty()) {
        MBItem leftover = outputStream.read();
        std::cout << "[Load-Phase Output] "
                  << (leftover.isWeight?"[Weight]":"[Partial]")
                  << (leftover.isRealPart?"(Real)":"(Imag)")
                  << (leftover.needsMultiply?"(needsM)":"(done)")
                  << " =>(" << leftover.real << "," << leftover.imag << ")"
                  << " destPE=" << leftover.destPE
                  << " tag=" << leftover.tag
                  << "\n";
    }


    // ---------------------------
    // (B) Provide partial items 
    // ---------------------------
    // We'll send 2 partials:
    //   (1+ j1) => destPE=0 => tag=777
    //   (2+ j2) => destPE=1 => tag=888
    inputStream.write(MBItem(false, true, true, 0, 1.f, 1.f, false, 777));
    inputStream.write(MBItem(false, true, true, 1, 2.f, 2.f, false, 888));

    // Provide horizontal inputs each cycle:
    //   For 2 PEs => (A,B) for PE0, (A,B) for PE1
    //   Suppose for 10 cycles => each cycle => 
    //       PE0 => (1,1), PE1 => (2,2)
    for(int cyc=0; cyc<10; cyc++) {
        // PE0 => (1,1)
        aStream.write(1.f);
        bStream.write(1.f);

        // PE1 => (2,2)
        aStream.write(2.f);
        bStream.write(2.f);
    }

    // Second call => run 10 cycles
    std::cout << "\n======== MAIN RUN PHASE ========\n";
    systolic_array(
        inputStream, outputStream, 
        aStream, bStream,
        accumulateMode,
        10
    );

    // Print final outputs
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n======== FINAL OUTPUT ITEMS ========\n";
    while(!outputStream.empty()) {
        MBItem out = outputStream.read();
        std::cout << (out.isWeight ? "[Weight]" : "[Partial]")
                  << (out.isRealPart ? "(Real)" : "(Imag)")
                  << (out.needsMultiply ? "(needsM)" : "(done)")
                  << " =>(" << out.real << "," << out.imag << ")"
                  << " destPE=" << out.destPE
                  << " tag=" << out.tag
                  << "\n";
    }

    return 0;
}
