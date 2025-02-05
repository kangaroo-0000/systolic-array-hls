#ifndef PE_CYCLE_H
#define PE_CYCLE_H

#include <hls_stream.h>
#include <ap_int.h>
#include <iostream>
#include "pe_state.h"


#define MAX_PRODUCED_ITEMS 8

inline void complexMul(float a, float b, float p, float q, float &rOut, float &iOut)
{
#pragma HLS inline
    rOut = a*p - b*q;
    iOut = a*q + b*p;
}

inline void debugPrintBuffer(int peIndex, int cycle, const PEState &pe, const char* msg)
{
#ifndef __SYNTHESIS__
    std::cout << "\n[DEBUG] PE" << peIndex << " " << msg
              << " cycle=" << cycle
              << " phase=" << (pe.phase==PHASE_LOAD ? "LOAD" : "MULT")
              << " topCount=" << pe.topCount
              << " accum=(" << pe.partialReal << "," << pe.partialImag << ")\n";
    for (int i = 0; i < 8; i++) {
        const MBItem &itm = pe.MB[i];
        std::cout << "   MB[" << i << "] ";
        if (itm.isWeight) {
            std::cout << "[Weight]";
            std::cout << (itm.isRealPart ? "(Real)" : "(Complex)");
        } else {
            std::cout << "[Partial]";
            std::cout << (itm.isRealPart ? "(Real)" : "(Complex)");
        }
        std::cout << (itm.needsMultiply ? "(needsM)" : "(done)")
                  << " destPE=" << itm.destPE
                  << " =>(" << itm.real << "," << itm.imag << ")";
        if (itm.startOfFrame)
            std::cout << " [SoF]";
        std::cout << "\n";
    }
#endif
}

inline void passDownWithPriority(
    PEState &pe,
    hls::stream<MBItem> &downStream,
    bool passWeights
) {
#pragma HLS inline
    int i = 2;
    while (i < 2 + pe.topCount) {
        MBItem &itm = pe.MB[i];
        if (!itm.isWeight) {
            downStream.write(itm);
            for (int j = i; j < (2 + pe.topCount - 1); j++) {
                pe.MB[j] = pe.MB[j + 1];
            }
            pe.topCount--;
        } else {
            i++;
        }
    }

    if (passWeights) {
        i = 2;
        while (i < 2 + pe.topCount) {
            MBItem &itm = pe.MB[i];
            if (itm.isWeight) {
                downStream.write(itm);
                for (int j = i; j < (2 + pe.topCount - 1); j++) {
                    pe.MB[j] = pe.MB[j + 1];
                }
                pe.topCount--;
            } else {
                i++;
            }
        }
    }
}

inline void passDown_loadPhase(
    int myIndex,
    PEState &pe,
    hls::stream<MBItem> &downStream
) {
#pragma HLS inline
    int i = 2;
    while (i < 2 + pe.topCount) {
        MBItem &itm = pe.MB[i];
        if (!itm.isWeight) {
            if (itm.destPE != myIndex || !itm.needsMultiply) {
                downStream.write(itm);
                for (int j = i; j < (2 + pe.topCount - 1); j++) {
                    pe.MB[j] = pe.MB[j + 1];
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

    if (pe.phase == PHASE_LOAD) {
        int itemsIngested = 0;
        while (!inputStream.empty() &&
            itemsIngested < 4 &&
            pe.topCount < 6)
        {
            MBItem itm = inputStream.read();
            itemsIngested++;
            if (itm.startOfFrame && accumulateMode) {
                pe.partialReal = 0.f;
                pe.partialImag = 0.f;
            }
            pe.MB[2 + pe.topCount] = itm;
            pe.topCount++;
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER INGEST");

        bool loadedSomething = true;
        while (loadedSomething) {
            loadedSomething = false;
            for (int i = 2; i < 2 + pe.topCount; i++) {
                MBItem &itm = pe.MB[i];
                if (itm.isWeight && itm.destPE == myIndex) {
                    if (itm.isRealPart) {
                        pe.MB[0] = itm;
                    } else {
                        pe.MB[1] = itm;
                    }
                    for (int j = i; j < (2 + pe.topCount - 1); j++) {
                        pe.MB[j] = pe.MB[j + 1];
                    }
                    pe.topCount--;
                    loadedSomething = true;
                    break;
                }
            }
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER IMMEDIATE LOAD WEIGHTS");

        passDown_loadPhase(myIndex, pe, downStream);

        debugPrintBuffer(myIndex, cycle, pe, "END PHASE_LOAD");
        pe.phase = PHASE_MULT;
    }
    else {
        MBItem produced[MAX_PRODUCED_ITEMS];
        int producedCount = 0;
        
        float pVal = pe.MB[0].real;
        float qVal = pe.MB[1].imag;

        int i = 2;
        while (i < 2 + pe.topCount) {
            MBItem &itm = pe.MB[i];
            if (!itm.isWeight && itm.destPE == myIndex && itm.needsMultiply) {
                float outR, outI;
                complexMul(pe.horizA, pe.horizB, pVal, qVal, outR, outI);

                if (accumulateMode) {
                    float accumR = pe.partialReal + itm.real;
                    float accumI = pe.partialImag + itm.imag;
                    accumR += outR;
                    accumI += outI;
                    pe.partialReal = accumR;
                    pe.partialImag = accumI;

                    MBItem newPartial(false, true, true,
                        myIndex + 1,
                        accumR, accumI, false);
                    if (myIndex == (totalPEs - 1)) {
                        newPartial.needsMultiply = false;
                        newPartial.destPE = -1;
                    }
                    produced[producedCount++] = newPartial;
                }
                else {
                    MBItem subR(false, true, false,
                        myIndex + 1, outR, 0.f, false);
                    MBItem subI(false, false, false,
                        myIndex + 1, 0.f, outI, false);
                    if (myIndex == (totalPEs - 1)) {
                        subR.needsMultiply = false; subR.destPE = -1;
                        subI.needsMultiply = false; subI.destPE = -1;
                    }
                    produced[producedCount++] = subR;
                    produced[producedCount++] = subI;
                }

                for (int j = i; j < (2 + pe.topCount - 1); j++) {
                    pe.MB[j] = pe.MB[j + 1];
                }
                pe.topCount--;
            }
            else {
                i++;
            }
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER MULTIPLY");

        for (int i = 0; i < producedCount; i++) {
            if (pe.topCount < 6) {
                pe.MB[2 + pe.topCount] = produced[i];
                pe.topCount++;
            }
        }

        debugPrintBuffer(myIndex, cycle, pe, "AFTER INSERT PRODUCED");

        bool passWeights = true;
        passDownWithPriority(pe, downStream, passWeights);

        debugPrintBuffer(myIndex, cycle, pe, "END PHASE_MULT");

        pe.phase = PHASE_LOAD;
    }
}

#endif // PE_CYCLE_H