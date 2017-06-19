#ifndef _PERF_MONITOR_
#define _PERF_MONITOR_
#include "MON_DEFINE.h"
#include <stdint.h>

class CHREC_MON;

class PERF_MON{
public:
    uint64_t cyc[AES] = {0};//total cycles
    uint64_t cnt0[AES][MAXPORTS][CNTS] = {{{0}}};//Absolute counter
    //cnt0[0][0][4] # of read request
    //cnt0[0][0][5] # of write request
    //cnt0[0][0][2] # of request stall
    //cnt0[0][0][3] # of response stall
    uint64_t cnt1[AES][MAXPORTS][RW] = {{{0}}};//Latency counter
    //cnt1[0][0][0] total overlapping latency of read/write accesses
    //cnt1[0][0][1] total overlapping latency of write accesses
    //cnt1[0][0][2] total overlapping latency of read accesses
    //cnt1[0][0][3] sum latency of all memory read/write accesses
    //cnt1[0][0][4] sum latency of all memory write accesses
    //cnt1[0][0][5] sum latency of all memory read accesses
    uint64_t cnt2[AES][RW][BINS] = {{{0}}};//histogram counter
    void monitor_read(uint64_t offset, uint32_t portNum, uint32_t rank, CHREC_MON* chrecMon);//rank: the order of perf_monitor;
};

#endif
