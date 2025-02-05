#ifndef PE_STATE_H
#define PE_STATE_H

#include "mb_item.h"

// We do a simple 2-phase pipeline approach: PHASE_LOAD vs. PHASE_MULT
enum Phase {
    PHASE_LOAD = 0,
    PHASE_MULT = 1
};

struct PEState {
    // Movement buffer [0..7]
    MBItem MB[8];
    int    topCount; // how many items are in [2..(2+topCount-1)]

    // For accumulate mode
    float partialReal;
    float partialImag;

    // Horizontal input (a,b)
    float horizA;
    float horizB;

    // 2-phase pipeline
    Phase phase;

    PEState()
      : topCount(0),
        partialReal(0.f), partialImag(0.f),
        horizA(0.f), horizB(0.f),
        phase(PHASE_LOAD)
    {
        // MB[0] => local real weight
        MB[0] = MBItem(true, true,  false, -1, 0.f,0.f,false);
        // MB[1] => local complex weight
        MB[1] = MBItem(true, false, false, -1, 0.f,0.f,false);
        for(int i=2; i<8; i++){
            MB[i] = MBItem();
        }
    }
};

#endif // PE_STATE_H
