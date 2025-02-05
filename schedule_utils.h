#ifndef SCHEDULE_UTILS_H
#define SCHEDULE_UTILS_H

// Not used

inline bool isRealReceiveWindow(int /*peIndex*/, int cycle) {
    return ((cycle % 4) == 0);
}
inline bool isRealSendWindow(int /*peIndex*/, int cycle) {
    return ((cycle % 4) == 1);
}
inline bool isComplexReceiveWindow(int /*peIndex*/, int cycle) {
    return ((cycle % 4) == 2);
}
inline bool isComplexSendWindow(int /*peIndex*/, int cycle) {
    return ((cycle % 4) == 3);
}

// partial results can move every cycle
inline bool shouldMovePartialDown(int /*peIndex*/, int /*cycle*/) {
    return true;
}

#endif // SCHEDULE_UTILS_H
