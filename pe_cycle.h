#ifndef PE_CYCLE_H
#define PE_CYCLE_H

#include <hls_stream.h>
#include <iostream>
#include "pe_state.h"
#include "globals.h"

/////////////////////////////////////////////////////
// Utility: complex multiply (a + jb)*(p + jq)
/////////////////////////////////////////////////////
inline void complexMul(float a, float b, float p, float q, float &rOut, float &iOut)
{
#pragma HLS inline
    // (a + j b) * (p + j q) = (a*p - b*q) + j(a*q + b*p)
    rOut = a*p - b*q;
    iOut = a*q + b*p;
}

/////////////////////////////////////////////////////
// Debug print: show MB contents
/////////////////////////////////////////////////////
inline void debugPrintBuffer(int peIndex, int cycle, const PEState &pe, const char* msg)
{
#ifndef __SYNTHESIS__  // Only print in SW simulation
    std::cout << "\n[DEBUG] PE" << peIndex << " " << msg
              << " cycle=" << cycle
              << " phase=" << (pe.phase == PHASE_LOAD ? "LOAD" : "MULT")
              << " topCount=" << pe.topCount
              << " accum=(" << pe.partialReal << "," << pe.partialImag << ")"
              << " (horizA=" << pe.horizA << ",horizB=" << pe.horizB << ")"
              << "\n";
    for (int i = 0; i < MB_SIZE; i++) {
        const MBItem &itm = pe.MB[i];
        std::cout << "   MB[" << i << "] ";
        if (itm.isWeight) {
            std::cout << "[Weight]";
            std::cout << (itm.isRealPart ? "(Real)" : "(Imag)");
        } else {
            std::cout << "[Partial]";
            std::cout << (itm.isRealPart ? "(Real)" : "(Imag)");
        }
        std::cout << (itm.needsMultiply ? "(needsM)" : "(done)")
                  << " destPE=" << itm.destPE
                  << " =>(" << itm.real << "," << itm.imag << ")"
                  << " tag=" << itm.tag;
        if (itm.startOfFrame) {
            std::cout << " [SoF]";
        }
        std::cout << "\n";
    }
#endif
}

/////////////////////////////////////////////////////
// passDownWithPriority: 
//   pass partials first, then weights if passWeights==true
/////////////////////////////////////////////////////
inline void passDownWithPriority(
    PEState &pe,
    hls::stream<MBItem> &downStream,
    bool passWeights
) {
#pragma HLS inline
    // 1) pass partials
    int i = 2;
    while (i < 2 + pe.topCount) {
        MBItem &itm = pe.MB[i];
        if (!itm.isWeight) {
            downStream.write(itm);
            // shift MB
            for (int j = i; j < (2 + pe.topCount - 1); j++) {
                pe.MB[j] = pe.MB[j+1];
            }
            pe.topCount--;
        } else {
            i++;
        }
    }

    // 2) optionally pass weights
    if (passWeights) {
        i = 2;
        while (i < 2 + pe.topCount) {
            MBItem &itm = pe.MB[i];
            if (itm.isWeight) {
                downStream.write(itm);
                for (int j = i; j < (2 + pe.topCount - 1); j++) {
                    pe.MB[j] = pe.MB[j+1];
                }
                pe.topCount--;
            } else {
                i++;
            }
        }
    }
}

/////////////////////////////////////////////////////
// passDown_loadPhase:
//   pass any partial that doesn't belong here 
//   or no longer needs multiply
/////////////////////////////////////////////////////
inline void passDown_loadPhase(
    int myIndex,
    PEState &pe,
    hls::stream<MBItem> &downStream
) {
#pragma HLS inline
    int i = 2;
    while (i < 2 + pe.topCount) {
        MBItem &itm = pe.MB[i];
        // If partial and not for me or no longer needs multiply => pass down
        if (!itm.isWeight) {
            if (itm.destPE != myIndex || !itm.needsMultiply) {
                downStream.write(itm);
                for (int j = i; j < (2 + pe.topCount - 1); j++) {
                    pe.MB[j] = pe.MB[j+1];
                }
                pe.topCount--;
            } else {
                i++;
            }
        } else {
            i++;
        }
    }
}

/////////////////////////////////////////////////////
// pe_cycle: 2-phase pipeline
/////////////////////////////////////////////////////
inline void pe_cycle(
    int myIndex,
    int totalPEs,
    int cycle,
    PEState &pe,
    hls::stream<MBItem> &downStream,
    hls::stream<MBItem> &inputStream,
    bool accumulateMode
) {
#pragma HLS inline

    debugPrintBuffer(myIndex, cycle, pe, "BEGIN");

    //----------------------------------------------
    // PHASE_LOAD
    //----------------------------------------------
    if (pe.phase == PHASE_LOAD) {

        // (1) Ingest up to MAX_INGEST items from input if there's room
        int itemsIngested = 0;
        while (!inputStream.empty() &&
               itemsIngested < MAX_INGEST &&
               pe.topCount < (MB_SIZE - 2))
        {
            MBItem itm = inputStream.read();
            itemsIngested++;

            // If SoF => reset accum if accumulate
            if (itm.startOfFrame && accumulateMode) {
                pe.partialReal = 0.f;
                pe.partialImag = 0.f;
            }
            pe.MB[2 + pe.topCount] = itm;
            pe.topCount++;
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER INGEST");

        // (2) Immediately load any weight for me into MB[0] or MB[1]
        bool loadedSomething = true;
        while (loadedSomething) {
            loadedSomething = false;
            for (int i = 2; i < 2 + pe.topCount; i++) {
                MBItem &itm = pe.MB[i];
                if (itm.isWeight && itm.destPE == myIndex) {
                    if (itm.isRealPart) {
                        pe.MB[0] = itm; // store real weight
                    } else {
                        pe.MB[1] = itm; // store imag weight
                    }
                    // shift out
                    for(int j = i; j < (2 + pe.topCount -1); j++){
                        pe.MB[j] = pe.MB[j+1];
                    }
                    pe.topCount--;
                    loadedSomething = true;
                    break; 
                }
            }
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER LOAD WEIGHTS");

        // (3) pass down partials that are not for me or no longer need multiply
        passDown_loadPhase(myIndex, pe, downStream);

        debugPrintBuffer(myIndex, cycle, pe, "END PHASE_LOAD");

        // Switch phase => MULT
        pe.phase = PHASE_MULT;
    }

    //----------------------------------------------
    // PHASE_MULT
    //----------------------------------------------
    else {
        MBItem produced[MAX_PRODUCED_ITEMS];
        int producedCount = 0;

        // local weight from MB[0] and MB[1]
        float pVal = pe.MB[0].real;  // real weight
        float qVal = pe.MB[1].imag;  // imag weight

        int i = 2;
        while (i < 2 + pe.topCount) {
            MBItem &itm = pe.MB[i];
            // If partial for me and needsMultiply => do multiply
            if (!itm.isWeight && itm.destPE == myIndex && itm.needsMultiply) {
                float outR, outI;
                // Multiply (horizA,horizB) with local weight (pVal,qVal)
                complexMul(pe.horizA, pe.horizB, pVal, qVal, outR, outI);

                if (accumulateMode) {
                    // Add old partial + new product
                    float accumR = pe.partialReal + itm.real;
                    float accumI = pe.partialImag + itm.imag;
                    accumR += outR;
                    accumI += outI;
                    pe.partialReal = accumR;
                    pe.partialImag = accumI;

                    // Pass to next PE
                    MBItem newPartial(
                        false, /*isRealPart=*/true,
                        /*needsMultiply=*/true,
                        myIndex + 1,
                        accumR, accumI,
                        /*startOfFrame=*/false,
                        itm.tag
                    );
                    // If last PE => mark done
                    if (myIndex == (totalPEs -1)) {
                        newPartial.needsMultiply = false;
                        newPartial.destPE = -1;
                    }
                    produced[producedCount++] = newPartial;
                }
                else {
                    // NON-ACCUMULATE => produce final partial, done
                    // so it doesn't go to next PE
                    MBItem subR(
                        false, true,
                        /*needsMultiply=*/false,
                        /*destPE=*/-1,
                        outR, 0.f,
                        false,
                        itm.tag
                    );
                    MBItem subI(
                        false, false,
                        false,
                        -1,
                        0.f, outI,
                        false,
                        itm.tag
                    );
                    produced[producedCount++] = subR;
                    produced[producedCount++] = subI;
                }

                // shift out consumed item
                for(int j = i; j < (2 + pe.topCount - 1); j++){
                    pe.MB[j] = pe.MB[j+1];
                }
                pe.topCount--;
            }
            else {
                i++;
            }
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER MULTIPLY");

        // Insert produced items
        for(int idx = 0; idx < producedCount; idx++){
            if (pe.topCount < (MB_SIZE-2)) {
                pe.MB[2 + pe.topCount] = produced[idx];
                pe.topCount++;
            }
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER INSERT PRODUCED");

        // passDownWithPriority => pass partials first, then weights
        bool passWeights = true;
        passDownWithPriority(pe, downStream, passWeights);

        debugPrintBuffer(myIndex, cycle, pe, "END PHASE_MULT");

        // Switch back => LOAD
        pe.phase = PHASE_LOAD;
    }
}

#endif // PE_CYCLE_H
