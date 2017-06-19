#include "PERFMON.h"
#include "libcnyfwd.h"
#include "MON_DEFINE.h"
#include "CHRECMON.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void PERF_MON::monitor_read(uint64_t offset, uint32_t portNum, uint32_t rank, CHREC_MON* chrecMON){
    for(int ae=0; ae<AES; ae++){
        uint64_t off=offset<<3;
        if(cny_fwd_read("aemc0", off, &this->cyc[ae])<0){
            fprintf(stderr, "Error: CSR read returned error.\n");
            exit(EXIT_FAILURE);
        }

        uint64_t rd_pcnt;
        if(cny_fwd_read("aemc0", (off+0x7f8), &rd_pcnt)<0){
           fprintf(stderr, "Error: CSR read returned error.\n");
           exit(EXIT_FAILURE);
        }

        chrecMON->pcnt[rank] = static_cast<int> (rd_pcnt);
        off += 8;

        //Absolute counter
        for (int i=0; i<CNTS; i++){
           for (int p=0; p<chrecMON->pcnt[rank]; p++){
               if (cny_fwd_read("aemc0", off, &this->cnt0[ae][p][i])<0){
                   fprintf(stderr, "Error: CSR read returned error.\n");
                   exit(EXIT_FAILURE);
               }
               off+=8;
           } 
        }

        //Clear out the counters
        if(cny_fwd_write("aemc0", (offset * 8), 0x0, 0x0)<0){
            fprintf(stderr, "Error: CSR write returned error.\n");
            exit(EXIT_FAILURE);
        }

        //Latency counter        
        off=(offset+0x100)<<3;
        for(int rw=0; rw<RW; rw++){
            for(int p=0; p<chrecMON->pcnt[rank]; p++){
                if(cny_fwd_read("aemc0", off, &this->cnt1[ae][p][rw])<0){
                    fprintf(stderr, "Error: CSR read returned error.\n");
                    exit(EXIT_FAILURE);
                }
                off += 8;
            }
        }

        //Clear out the counters
        if(cny_fwd_write("aemc0", (offset+0x100)<<3, 0x0, 0x0)<0){
            fprintf(stderr, "Error: CSR write returned error.\n");
            exit(EXIT_FAILURE);
        }

        //Histogram counter
        off=(offset+0x200)<<3;
        for(int rw=0; rw<RW; rw++){
            for(int i=0; i<BINS; i++){
                if(cny_fwd_read("aemc0", off, &this->cnt2[ae][rw][i])<0){
                    fprintf(stderr, "Error: CSR read returned error.\n");
                    exit(EXIT_FAILURE);
                }
                off+=8;
            }
        }

        //Clear out the counters
        if(cny_fwd_write("aemc0", (offset+0x200)<<3, 0x0, 0x0)<0){
            fprintf(stderr, "Error: CSR write returned error.\n");
            exit(EXIT_FAILURE);
        }

    } 
};
