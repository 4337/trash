#ifndef XXX_WCALLBACK_H
#define XXX_WCALLBACK_H

struct _TCP_CB_ARGV
{
       unsigned char* opt_ptr;
       const unsigned char* resp_ptr_buf;   
       const struct pcap_pkthdr* header;
};

void wpcap_tcp_callback(unsigned char* i_argv, 
                        const struct pcap_pkthdr* i_header,
                        const unsigned char* i_pkt_data);


#endif
