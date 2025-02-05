#ifndef MB_ITEM_H
#define MB_ITEM_H

// MBItem:
//  - isWeight => weight (real or complex) or partial
//  - isRealPart => real vs. complex
//  - needsMultiply => partial that still needs multiply in this PE
//  - destPE => which PE index uses this item
//  - real, imag => numeric data
//  - startOfFrame => optional to reset accumulators
struct MBItem {
    bool  isWeight;
    bool  isRealPart;
    bool  needsMultiply;
    int   destPE;
    float real;
    float imag;
    bool  startOfFrame;

    MBItem()
      : isWeight(false), isRealPart(true),
        needsMultiply(true), destPE(-1),
        real(0.f), imag(0.f),
        startOfFrame(false)
    {}

    MBItem(bool w, bool r, bool nm, int dpe,
           float re, float im,
           bool sof=false)
      : isWeight(w), isRealPart(r),
        needsMultiply(nm), destPE(dpe),
        real(re), imag(im),
        startOfFrame(sof)
    {}
};

#endif // MB_ITEM_H
