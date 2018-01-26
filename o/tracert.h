#ifndef XXX_TRACERT_H
#define XXX_TRACERT_H

#include <pcap.h>

#define TR_MAX_HOP 16
#define TR_WAIT_FOR_REP 2

unsigned char send_tracert(pcap_t* i_pcap,unsigned char* i_packet,const char* i_dip,
                           unsigned int i_packet_size);
                           
void tracert_callback(unsigned char* i_argv, 
                      const struct pcap_pkthdr* i_header,
                      const unsigned char* i_pkt_data);

#endif
