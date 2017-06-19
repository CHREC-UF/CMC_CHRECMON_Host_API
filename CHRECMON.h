#ifndef _CHREC_MON_
#define _CHREC_MON_

#include "PERFMON.h"
#include <iostream>
#include "libcnyfwd.h"
#include <stdint.h>
#include "MON_DEFINE.h"

using namespace std;

class CHREC_MON{
        friend class PERF_MON;
public:
        PERF_MON PERS_PERFMON;
        PERF_MON MC_PERFMON;
        PERF_MON HMC_INTF_PERFMON;
        CHREC_MON(uint32_t portNum);
        CHREC_MON();
        ~CHREC_MON();
        void CHREC_MON_READ();
private:
        bool portEnable;
        uint32_t portNum;
        int pcnt[NUM_MONITOR];
        uint64_t aemc_id, aemc_time_date;//aemc information
        uint64_t hix_id, hix_time_date;//hix information
};

#endif
