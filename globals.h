#ifndef GLOBALS_H
#define GLOBALS_H

static const int NUM_PEs           = 2;   // Number of Processing Elements
static const int MAX_PRODUCED_ITEMS= 8;   // Max partial items a PE can produce
static const int MB_SIZE           = 8;   // Movement buffer size
static const int MAX_INGEST        = 4;   // Max items to ingest from input per cycle

#endif // GLOBALS_H