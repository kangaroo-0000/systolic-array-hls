#ifndef PE_STATE_H
#define PE_STATE_H

#include "mb_item.h"
#include "globals.h"

// 2-phase pipeline approach
enum Phase {
    PHASE_LOAD = 0,
    PHASE_MULT = 1
};

// Each PE has:
//  - MB[0] and MB[1] for local weights (real, imag)
//  - MB[2..(2+topCount-1)] for items in transit
//  - partialReal,partialImag for accumulation (if accumulateMode==true)
//  - horizA,horizB for horizontal inputs (optional usage)
//  - a "phase" that toggles between LOAD vs MULT each cycle
struct PEState {
    MBItem MB[MB_SIZE];
    int    topCount;

    float  partialReal;
    float  partialImag;

    float  horizA;
    float  horizB;

    Phase  phase;

    PEState()
      : topCount(0),
        partialReal(0.f), partialImag(0.f),
        horizA(0.f), horizB(0.f),
        phase(PHASE_LOAD)
    {
        // MB[0], MB[1] => local weight
        MB[0] = MBItem(true, true,  false, -1, 0.f,0.f,false, 0);
        MB[1] = MBItem(true, false, false, -1, 0.f,0.f,false, 0);

        for(int i=2; i<MB_SIZE; i++){
            MB[i] = MBItem();
        }
    }
};

#endif // PE_STATE_H
