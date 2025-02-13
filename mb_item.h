#ifndef MB_ITEM_H
#define MB_ITEM_H

// MBItem is the data structure flowing through the systolic array.
//  - isWeight indicates it's a weight item vs. partial
//  - isRealPart indicates real vs. imaginary
//  - needsMultiply => partial that still needs multiply in this PE
//  - destPE => which PE index it’s destined for
//  - real, imag => numeric data
//  - startOfFrame => signal to reset accumulators in accumulate mode
//  - tag => an ID to track which input partial it came from, for debugging
struct MBItem {
    bool  isWeight;
    bool  isRealPart;
    bool  needsMultiply;
    int   destPE;
    float real;
    float imag;
    bool  startOfFrame;

    unsigned int tag;   // optional ID for debugging correlation

    MBItem()
      : isWeight(false), isRealPart(true),
        needsMultiply(true), destPE(-1),
        real(0.f), imag(0.f),
        startOfFrame(false),
        tag(0)
    {}

    MBItem(bool w, bool r, bool nm, int dpe,
           float re, float im,
           bool sof=false,
           unsigned int tg=0)
      : isWeight(w), isRealPart(r),
        needsMultiply(nm), destPE(dpe),
        real(re), imag(im),
        startOfFrame(sof),
        tag(tg)
    {}
};

#endif // MB_ITEM_H
