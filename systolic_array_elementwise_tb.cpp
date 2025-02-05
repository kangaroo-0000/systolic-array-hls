#include <iostream>
#include <iomanip>
#include "systolic_array.h"  // your 2-phase pipeline, with element-wise logic

int main()
{
    // Create input/output streams
    hls::stream<MBItem> inputStream("inputStream");
    hls::stream<MBItem> outputStream("outputStream");

    // Set element-wise mode => no accumulation
    bool accumulateMode = false;  
    int totalCycles = 20;

    //---------------------------------------------
    // 1) Provide (p0,q0)=(2,4) for PE0
    //    => real =>(2,0), imag =>(0,4), destPE=0
    //---------------------------------------------
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

    //---------------------------------------------
    // 2) Provide (p1,q1)=(3,5) for PE1
    //    => real =>(3,0), imag =>(0,5), destPE=1
    //---------------------------------------------
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

    //---------------------------------------------
    // 3) Provide partial(1,1) => go to PE0
    //    => In element-wise, that yields ( (1+ i1)*(2+ i4) ) => (-2,6)
    //    This partial will produce an output from PE0 only
    //---------------------------------------------
    inputStream.write(
        MBItem(
            /*isWeight=*/false,
            /*isRealPart=*/true, // partial => real vs complex is arbitrary here
            /*needsMultiply=*/true,
            /*destPE=*/0,  // => only PE0 uses it
            /*real=*/1.f, /*imag=*/1.f,
            /*startOfFrame=*/false
        )
    );

    //---------------------------------------------
    // 4) Provide partial(2,2) => go to PE1
    //    => In element-wise, that yields ( (2+ i2)*(3+ i5) ) => (-4,16)
    //    This partial won't be used by PE0 at all
    //---------------------------------------------
    inputStream.write(
        MBItem(
            false, true, true, 1,
            2.f, 2.f,
            false
        )
    );

    // Now run
    systolic_array(inputStream, outputStream, accumulateMode, totalCycles);

    // Print
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Final output items:\n";
    int idx = 0;
    while (!outputStream.empty()){
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
