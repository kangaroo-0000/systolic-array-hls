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

    // We'll run in ACCUMULATE mode
    bool accumulateMode = true;

    //----------------------------------------------------------------
    // (A) LOAD WEIGHTS FIRST
    //     PE0 => (p0,q0) = (2,4)
    //     PE1 => (p1,q1) = (3,5)
    //----------------------------------------------------------------
    inputStream.write( MBItem(
        /*isWeight=*/true, /*isRealPart=*/true, /*needsMultiply=*/false,
        /*destPE=*/0, 2.f, 0.f,
        /*startOfFrame=*/false,
        /*tag=*/101
    ));
    inputStream.write( MBItem(
        true, false, false,
        0, 0.f, 4.f,
        false,
        102
    ));
    inputStream.write( MBItem(
        true, true, false,
        1, 3.f, 0.f,
        false,
        103
    ));
    inputStream.write( MBItem(
        true, false, false,
        1, 0.f, 5.f,
        false,
        104
    ));

    // We'll do a short call => e.g. 5 cycles
    std::cout << "\n======== [ACCUM MODE] LOADING WEIGHTS ========\n";
    systolic_array(
        inputStream,
        outputStream,
        aStream, bStream,
        accumulateMode,
        5
    );

    // Drain leftover from the load phase
    while(!outputStream.empty()) {
        MBItem wout = outputStream.read();
        std::cout << "[LoadPhase output] "
                  << (wout.isWeight?"[Weight]":"[Partial]")
                  << (wout.isRealPart?"(Real)":"(Imag)")
                  << (wout.needsMultiply?"(needsM)":"(done)")
                  << " =>(" << wout.real << "," << wout.imag << ")"
                  << " destPE=" << wout.destPE
                  << " tag=" << wout.tag
                  << "\n";
    }

    //----------------------------------------------------------------
    // (B) PROVIDE A PARTIAL => (0,0), startOfFrame=true, destPE=0
    //     Because accumulateMode=true, each PE will add:
    //       partial + local_product => new partial
    //
    // Also provide horizontal inputs (a,b) each cycle
    //----------------------------------------------------------------
    // 1) The partial that starts the chain
    inputStream.write( MBItem(
        /*isWeight=*/false, /*isRealPart=*/true, /*needsMultiply=*/true,
        /*destPE=*/0,
        /*real=*/0.f, /*imag=*/0.f,
        /*startOfFrame=*/true,  // reset accum at the PE
        /*tag=*/999
    ));

    // 2) Let's feed 10 cycles of horizontal (A,B):
    //    - For PE0 => (1,1)
    //    - For PE1 => (2,2)
    for(int cyc=0; cyc<10; cyc++){
        // For PE0
        aStream.write(1.f);
        bStream.write(1.f);
        // For PE1
        aStream.write(2.f);
        bStream.write(2.f);
    }

    // Now run e.g. 10 cycles to process the partial
    std::cout << "\n======== [ACCUM MODE] MAIN RUN ========\n";
    systolic_array(
        inputStream,
        outputStream,
        aStream, bStream,
        accumulateMode,
        10
    );

    //----------------------------------------------------------------
    // (C) PRINT FINAL OUTPUT
    //----------------------------------------------------------------
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n======== [ACCUM MODE] FINAL OUTPUT ========\n";
    while (!outputStream.empty()) {
        MBItem out = outputStream.read();
        std::cout 
            << (out.isWeight?"[Weight]":"[Partial]")
            << (out.isRealPart?"(Real)":"(Imag)")
            << (out.needsMultiply?"(needsM)":"(done)")
            << " =>(" << out.real << "," << out.imag << ")"
            << " destPE=" << out.destPE
            << " tag=" << out.tag
            << "\n";
    }

    return 0;
}
