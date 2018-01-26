#ifndef TPROBE_UPTIME_H
#define TPROBE_UPTIME_H

#include "univ_tcp_options.h"

#define TP_UPTIME_DEBUG     0
#define TP_TICK_SAMPLE      1

struct tprobe_raw_uptime
{
       unsigned long _last_ts;
       unsigned long _clock_freq;
};


struct tprobe_raw_uptime* tprobe_uptime_init(pcap_t* i_dsc,unsigned char* i_packet);

unsigned long tprobe_uptime_calc_freq(unsigned long i_raw_freq);

void tprobe_uptime_calculate(unsigned long i_cfreq,unsigned long i_lts,struct tm* i_out_time);

#define tprobe_uptime_free(i_raw_uptime)\
do\
{\
  univ_free(i_raw_uptime);\
}while(0)


#endif
