#define WPCAP
#define HAVE_REMOTE
#include <windows.h>
#include <stdio.h>
#include "tracert.h"
#include "univ_packet.h"

static struct _cb_argv
{
              unsigned char _cnt;
              char _target[16];      
}CB_ARGV = {0,{0}};

unsigned char send_tracert(pcap_t* i_pcap,unsigned char* i_packet,const char* i_dip,
                           unsigned int i_packet_size)
{             
              unsigned int l_err = 0;
              static unsigned char l_ttl = 1;
            
              ((struct tcpip4_packet*)i_packet)->ip.ttl = l_ttl;
              ((struct tcpip4_packet*)i_packet)->ip.ch_sum = 0;
              ((struct tcpip4_packet*)i_packet)->ip.ch_sum = univ_net_checksum(&((struct tcpip4_packet*)i_packet)->ip,
                                                                               sizeof(struct ip4_packet)); 
              
              if(pcap_sendpacket(i_pcap,i_packet,i_packet_size) != 0) {
                 printf(" [!].Tracert error :(\r\n\r\n");
                 return 0;                                                         
              }
        
              while(pcap_dispatch(i_pcap,1,tracert_callback,(unsigned char*)&CB_ARGV) != 1) {/*TODO  Error handling !*/  
                    if(l_err++ == TR_WAIT_FOR_REP) {
                       printf("    [%d]. ************ s_ttl:%d (time_out) \r\n",
                              l_ttl - 1,
                              l_ttl);
                       ++CB_ARGV._cnt;
                     
                       if(l_ttl == TR_MAX_HOP) {
                           printf("    [%d]. ************ s_ttl:%d (too many hosts to target)\r\n",l_ttl,l_ttl - 1);
                           return 0;    
                       }
                       l_ttl++;                     
                        return 1;
                     }
             } 
                
             if(!strcmp(CB_ARGV._target,i_dip)) {
                       printf("s_ttl:%d (target)\r\n",l_ttl);
                       return 0;                                   
             }   
                        
             printf("s_ttl:%d (pass)\r\n",l_ttl);
             l_ttl++;
             return 1;                 
}

void tracert_callback(unsigned char* i_argv, 
                      const struct pcap_pkthdr* i_header,
                      const unsigned char* i_pkt_data)
{
    char* l_dip;
    struct in_addr  l_raddr;
    struct ip4_packet* l_ip = ((struct ip4_packet*)(i_pkt_data + sizeof(struct eth_packet)));
    l_raddr.s_addr = l_ip->source_ip;
    l_dip = inet_ntoa(l_raddr);
    
    memcpy(&((struct _cb_argv*)i_argv)->_target,l_dip,15);
    printf("    [%d]. %s d_ttl:%d ",((struct _cb_argv*)i_argv)->_cnt,
                                    l_dip,l_ip->ttl);   
    ((struct _cb_argv*)i_argv)->_cnt++;      
}
