#include "CHRECMON.h"
#include <stdlib.h>
#include <stdio.h>
#include "libcnyfwd.h"
#include "MON_DEFINE.h"
#include <stdint.h>
#include "PERFMON.h"
#include "string.h"
#include <iostream>

using namespace std;

CHREC_MON::CHREC_MON(){
    this->portEnable = false;
}

CHREC_MON::~CHREC_MON(){
    if(this->portEnable == true && cny_fwd_close()){
        fprintf(stderr, "Error: Unable to close FWD socket.\n");
    }
}

CHREC_MON::CHREC_MON(uint32_t portNum){
    if(portNum == 0){
        this->portEnable = false;
    }else if(portNum <= 1024 || portNum > 65536){
        printf ("Illegal port number for peekpoke.\n");
        exit(EXIT_FAILURE);
    }else{
        this->portEnable = true;
        this->portNum = portNum;
    }

    if(cny_fwd_open(this->portNum) && this->portEnable){
        fprintf(stderr, "Error: Unable to open FWD socket.\n");
        exit(EXIT_FAILURE);
    }

    if(this->portEnable){
        if(cny_fwd_read("aemc0", 8, &this->aemc_id) < 0){
            fprintf(stderr, "Error: CSR read returned error -1.\n");
            exit(EXIT_FAILURE);
        }

        if(cny_fwd_read("aemc0", 0, &this->aemc_time_date) < 0){
            fprintf(stderr, "Error: CSR read returned error -2.\n");
            exit(EXIT_FAILURE);
        }

        if(cny_fwd_read("hix", 0, &this->hix_id) < 0){
            fprintf(stderr, "Error: CSR read returned error -3.\n");
            exit(EXIT_FAILURE);
        }

        if(cny_fwd_read("hix", 0, &this->hix_time_date) < 0){
            fprintf(stderr, "Error: CSR read returned error -4.\n");
            exit(EXIT_FAILURE);
        }

        printf("  hix id:0x%lx time_date:0x%lx\n", this->hix_id, this->hix_time_date);
        printf("aemc0 id:0x%lx time_date:0x%lx\n", this->aemc_id, this->aemc_time_date);
    }

    this->pcnt[0] = 0;
    this->pcnt[1] = 0;
    this->pcnt[2] = 0;
}

void CHREC_MON::CHREC_MON_READ(){
    if(portEnable == false){
        printf("CHRECMON is not enabled, all the results will be 0!!\n");
    }else{
        this->PERS_PERFMON.monitor_read(BASE0_PERS, this->portNum, 0, this);
        this->MC_PERFMON.monitor_read(BASE0_MC, this->portNum, 1, this);
        this->HMC_INTF_PERFMON.monitor_read(BASE0_HMC_INTF, this->portNum, 2, this);
    }
}
