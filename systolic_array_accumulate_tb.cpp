#include <iostream>
#include <iomanip>
#include "systolic_array.h"  // Must include your 2-phase pipeline code

int main()
{
    hls::stream<MBItem> inputStream("inputStream");
    hls::stream<MBItem> outputStream("outputStream");

    // Accumulate Mode = true, but we feed no partial from outside
    bool accumulateMode = true;
    int totalCycles = 20;

    // (A) Provide (p0,q0)=(2,4) for PE0
    //    => 2 items: real => (2,0), imag => (0,4)
    inputStream.write(
        MBItem(
            /*isWeight=*/true,
            /*isRealPart=*/true,
            /*needsMultiply=*/false,
            /*destPE=*/0,
            /*real=*/2.f, /*imag=*/0.f,
            /*startOfFrame=*/false
        )
    );
    inputStream.write(
        MBItem(
            true, /*isRealPart=*/false,
            false, 0,
            0.f, 4.f,
            false
        )
    );

    // (B) Provide (p1,q1)=(3,5) for PE1
    //    => 2 items: real => (3,0), imag => (0,5)
    inputStream.write(
        MBItem(
            true, true,
            false, 1,
            3.f, 0.f,
            false
        )
    );
    inputStream.write(
        MBItem(
            true, false,
            false, 1,
            0.f, 5.f,
            false
        )
    );

    // (C Feed partial(0,0), startOfFrame => reset accum
    inputStream.write(MBItem(
        /*isWeight=*/false,
        /*isRealPart=*/true, 
        /*needsMultiply=*/true,
        /*destPE=*/0,
        0.f, 0.f,
        /*startOfFrame=*/true
    ));


    // Run the pipeline
    systolic_array(inputStream, outputStream, accumulateMode, totalCycles);

    // Print results
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Final output items:\n";
    int idx = 0;
    while (!outputStream.empty()) {
        MBItem out = outputStream.read();
        std::cout << "Item " << (idx++)
                  << (out.isWeight ? " [Weight]" : " [Partial]")
                  << (out.isRealPart ? "(Real)" : "(Complex)")
                  << (out.needsMultiply ? "(needsM)" : "(done)")
                  << " destPE=" << out.destPE
                  << " =>(" << out.real << "," << out.imag << ")"
                  << " SoF=" << out.startOfFrame
                  << "\n";
    }

    return 0;
}
