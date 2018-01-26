#define HAVE_REMOTE
#define WPCAP

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <pcap.h>
#include <string.h>
#include <time.h>
#include "tprobe_ui.h"
#include "tprobe_runtime.h"
#include "univ_net_utils.h"
#include "univ_win_sock.h"
#include "univ_tcp_options.h"
#include "univ_packet.h"
#include "wpcap.h"
#include "wcallback.h"
#include "tprobe_uptime.h"
#include "univ_mem.h"
#include "tracert.h"
#include "univ_macros.h"

#define  TP_TCP_OPT_SIZE 4
#define  TP_FILTER_SIZE  168

int main(int argc,char** argv)
{
    pcap_t* l_pcap; 
    unsigned char* l_packet;
    struct ui_options* l_opt;
    unsigned int l_err_cnt = 0;
    register int l_return  = -1;
    char l_sip[NET_IP_ADDR + 1];
    char l_pcap_err[PCAP_ERRBUF_SIZE + 1];
    
    TPROBE_SHOW_BANNER();
    
    if(tprobe_runtime() != 1) {
       TPROBE_OUT(" [!].Runtime error :(\r\n\r\n");
       return -1;                    
    }
    
    if(argc == 1) return -1;
    
    l_opt = tprobe_get_cmdline(argc,argv);
    if(l_opt == NULL) return -1;
    
    if(univ_winsock_init() != 0) {
       TPROBE_OUT(" [!].univ_winsock_init fail!\r\n\r\n");                        
       goto __ui__;                                       
    }
    if(univ_get_selfip(l_sip,NET_IP_ADDR) != 0) {
       TPROBE_OUT(" [!].Debug univ_get_selfip fail!\r\n\r\n");                          
       goto __winsock__;
    }         
   
    l_pcap = (pcap_t*)wpcap_init(l_sip,l_opt->time_out,
                                 l_pcap_err,
                                 PCAP_ERRBUF_SIZE,(const unsigned char)l_opt->interactive);   
   
    if(l_pcap == NULL) {
       TPROBE_OUT(" [!].wpcap_init fail!\r\n"
                  "     it can be problem with internet connection :(\r\n\r\n");
       goto __winsock__;              
    }
   
    if(l_opt->target_set != 0) {
       struct in_addr l_dip;                    
       struct hostent* l_host;     
       
       #ifdef T_PROBE_LOCALHOST_NO_SUPPORT
        if((strstr(l_opt->target,"127.") != NULL) ||  
           (!stricmp(l_opt->target,l_sip))) {
            TPROBE_OUT(" [!].Error no support for localhost :(\r\n\r\n");
            goto __pcap__;                                  
        }
       #endif 
       
       l_dip.s_addr = inet_addr(l_opt->target);
      
       if(l_dip.s_addr == INADDR_NONE) {/* hostname */
          l_host = gethostbyname(l_opt->target);
          if(l_host == NULL) {
             TPROBE_OUT(" [!].Error bad target HN\r\n\r\n");
             goto __pcap__;        
          }
          memcpy(&l_dip,l_host->h_addr_list[0],l_host->h_length);               
       } else if(((l_host = gethostbyaddr((char*)&l_dip,4,AF_INET)) == NULL) &&  /* ipv4 */
                  (univ_is_private_ipv4(&l_dip) == 0)) {
                TPROBE_OUT(" [!].Error bad target IP\r\n\r\n");
                goto __pcap__;    
       }                           
       memcpy(l_opt->target,inet_ntoa(l_dip),sizeof(l_opt->target));             
    } else {
        TPROBE_OUT(" [!].Error no target!\r\n"
                   "     use -T xxx.xxx.xxx.xxx v4 || xxx.xxxx.xxx\r\n\r\n");
        goto __pcap__;    
    }
    if((l_opt->s_port == 0) || (l_opt->d_port == 0)) { /* ports */
        TPROBE_OUT(" [!].Error no -S &&/|| -D options!\r\n"
                   "     you don't define source and/or destination port number!\r\n\r\n");
        goto __pcap__;                   
    } else if(l_opt->packet_set == 0) {                 /* packet type */
              TPROBE_OUT(" [!].Error no tcp packet option!\r\n"
                         "     you don't define -s || -A || -a || -m :(\r\n\r\n");
            goto __pcap__;
    } else {   
            unsigned int l_opt_size;
            struct tcp_seqack l_numbers;
            char l_filtr[TP_FILTER_SIZE + 1] = {'\0'};
            struct _TCP_CB_ARGV TCP_CB_ARGV = {(unsigned char*)l_opt,NULL,NULL};
            struct u_tcp_option tcp_options[TP_TCP_OPT_SIZE] = {{UTCP_OPT_MSS,UTCP_FILL_SIZE(0x04,245789234),4},     
                                                                {UTCP_OPT_SACKP,UTCP_FILL_SIZE(0x02,245789234),2}, 
                                                                {UTCP_OPT_NOP,0x00,1},
                                                                {UTCP_OPT_EOOL,0x00,1}};
                                              
            void(*l_tcp_type[])(struct tcpip4_packet*,
                                struct tcp_seqack*,
                                size_t,size_t) = {tcp_syn,tcp_synack,
                                                  tcp_ack,tcp_malformed};
        
            TPROBE_OUT(" [+].Create TCP/IPv4 Packet\r\n\r\n"
				       " [!].src_port:%d\r\n"
					   " [!].dst_port:%d\r\n" 
					   " [!].data_size:%d\r\n"
					   " [!].target:%s\r\n"
					   " [!].0f:%s\r\n"
					   " [!].opt:%s\r\n"
                       " [!].tracert:%s\r\n\r\n",
                       l_opt->s_port,l_opt->d_port,
                       l_opt->data_size,l_opt->target,
                       (l_opt->pof) ? "Enabled" : "Disabled",
                       (l_opt->packet_opt == NET_SEND_SYN) ? "SYN" : 
                       (l_opt->packet_opt == NET_SEND_SYNACK) ? "SYN/ACK" :
                       (l_opt->packet_opt == NET_SEND_ACK) ? "ACK" : "MALLFORMED",
                       (l_opt->tracert) ? "Enabled" : "Disabled");
    
             l_packet = tcpip4_init(l_sip,l_opt->target,                 /* create packet */
                                    l_opt->s_port,l_opt->d_port);
   
             if(l_packet == NULL) {
                TPROBE_OUT(" [!].tcpip4_init fail!\r\n\r\n");
                goto __pcap__;            
             } else if((l_packet = tcpip4_add_tcp_options(l_packet,
				                                          (struct u_tcp_option*)&tcp_options,
														   TP_TCP_OPT_SIZE)) == NULL) goto __pcap__;
               else l_opt_size   = u_tcpoptions_align_size(tcp_options,TP_TCP_OPT_SIZE);
        
            if((l_opt->data_size != 0) && (l_opt->data != NULL)) {    /* append data if exisist */
                l_packet = tcpip4_add_data(l_packet,l_opt->data,l_opt_size,l_opt->data_size);
               if(l_packet == NULL) {
                  TPROBE_OUT(" [!].Debug tcpip4_add_data fail\r\n\r\n");      
                  goto __pcap__;          
               }                
            }
        
            l_numbers.rseq = htonl((unsigned long)tcp_generate_ins() << 2);
            l_numbers.rack = htonl((unsigned long)tcp_generate_ins());
        
            l_tcp_type[l_opt->packet_opt]((struct tcpip4_packet*)l_packet,
                                          &l_numbers,l_opt_size,l_opt->data_size);
                                      
            l_opt->packet_size = sizeof(struct tcpip4_packet) + l_opt->data_size + l_opt_size;
        
            snprintf(l_filtr,TP_FILTER_SIZE,"src host %s and src port %d and dst port %d",
                     l_opt->target,l_opt->d_port,l_opt->s_port);
        
            if(wpcap_set_filter(l_pcap,l_filtr,0,0) != 1) {
               TPROBE_OUT(" [!].wpcap_set_filter fail!\r\n"
                          "     filter(%s)\r\n\r\n",l_filtr);  
               goto __packet__;                  
            }
        
            TPROBE_OUT(" [+].Sending packet: (TCP/IPv4) %s:%d -> %s:%d\r\n\r\n",
                       l_sip,l_opt->s_port,
                       l_opt->target,l_opt->d_port);
                  
            #if TPROBE_DEBUG==1 
               _DUMP_PKT(l_packet);
            #endif           
        
            if(pcap_sendpacket(l_pcap,l_packet,l_opt->packet_size) != 0) {/* send packet */
               TPROBE_OUT(" [!].Sending error (%s)\r\n\r\n",pcap_geterr(l_pcap));
               goto __packet__;                                                         
            } 
        
             while((pcap_dispatch(l_pcap,1,wpcap_tcp_callback,(unsigned char*)&TCP_CB_ARGV) != 1) && 
                   (l_err_cnt++ < NO_RESPONSE_ERROR_CNT)) {
                   char* l_perr = pcap_geterr(l_pcap);
                   if((l_perr != NULL) && (*l_perr != '\0')) {
                       TPROBE_OUT(" [!].Internal error pcap_dispatch() %s\r\n\r\n",l_perr);
                       goto __packet__;
                   }                    
             }
        
            if(TCP_CB_ARGV.resp_ptr_buf != NULL) {      
               if((l_opt->packet_opt == NET_SEND_SYN) || (l_opt->packet_opt == NET_SEND_MALLFORMED)) {
               unsigned char* l_p_copy = (unsigned char*)univ_malloc(l_opt->packet_size + 1);
               if(l_p_copy == NULL) {
                  TPROBE_OUT(" [!].Internal error rst_send copy_packet fail!\r\n\r\n");     
                  goto __packet__;
               } else {
                       unsigned char l_ip_opt_len = ip4_got_options(&((struct tcpip4_packet*)TCP_CB_ARGV.resp_ptr_buf)->ip);   /* grrrr zapomnia³em !!! */
                   
                       memcpy(l_p_copy,l_packet,l_opt->packet_size);
                       tcp_get_seqack((void*)&((struct tcpip4_packet*)(TCP_CB_ARGV.resp_ptr_buf + l_ip_opt_len))->tcp,&l_numbers);
                       tcp_rst((struct tcpip4_packet*)l_p_copy,&l_numbers,l_opt_size,l_opt->data_size);
                       pcap_sendpacket(l_pcap,l_p_copy,l_opt->packet_size);
                        univ_free(l_p_copy);  
                        Sleep(WAIT_FOR_LOST_PACKET);
               }      
            }
            if(l_opt->log != 0) {
               char l_name[MAX_PATH + 1];    
               pcap_dumper_t* l_pfile_h = NULL;
               struct tm* l_time = localtime(&(time_t)TCP_CB_ARGV.header->ts.tv_sec);
               
               snprintf(l_name,MAX_PATH,"[%d-%d-%d_%d-%d-%d][%s].pcap",l_time->tm_year + 1900,l_time->tm_mon + 1,
                                                                       l_time->tm_mday,l_time->tm_hour,
                                                                       l_time->tm_min,l_time->tm_sec,l_opt->target);                                                            
               l_pfile_h = pcap_dump_open(l_pcap,l_name);
               
               if(l_pfile_h != NULL) {
                  struct pcap_pkthdr l_h2 = {0};
                  l_h2.ts     = TCP_CB_ARGV.header->ts;    
                  l_h2.caplen = l_opt->packet_size;
                  l_h2.len = l_opt->packet_size;
                   
                  pcap_dump((unsigned char*)l_pfile_h,&l_h2,l_packet);   
                  pcap_dump((unsigned char*)l_pfile_h,TCP_CB_ARGV.header,TCP_CB_ARGV.resp_ptr_buf); 
                  pcap_dump_close(l_pfile_h); 
                  TPROBE_OUT(" [+].Log was safe in to %s \r\n\r\n",l_name);
               } else TPROBE_OUT(" [!].Internal error (logging : file_open fail)\r\n\r\n"); 
            }
            if(l_opt->uptime != 0) {
               struct tprobe_raw_uptime* l_raw_uptime = tprobe_uptime_init(l_pcap,l_packet);
                  
               if(l_raw_uptime == NULL) TPROBE_OUT(" [!].Remnote system uptime is unknow RwFRQ:0x00 Hz TS:0x00 (Fuck d'e clock)\r\n\r\n");   
               else {
                    struct tm l_rtime = {0};   
                    unsigned long l_freq = tprobe_uptime_calc_freq(l_raw_uptime->_clock_freq);
                      
                    tprobe_uptime_calculate(l_freq,l_raw_uptime->_last_ts,&l_rtime);
                    TPROBE_OUT(" [+].Remote system uptime guess: (%u days, %02d:%02d:%02d) FRQ:0x%x Hz TS:0x%04x\r\n\r\n",
                               l_rtime.tm_mday,l_rtime.tm_hour,l_rtime.tm_min,l_rtime.tm_sec,l_raw_uptime->_clock_freq,
                               l_raw_uptime->_last_ts); 
                    tprobe_uptime_free((struct tprobe_raw_uptime* )l_raw_uptime);
               }   
            }              
        } else TPROBE_OUT(" [+].Port status:  port (%d) on (%s) [filtered] r_ttl: [unknow]\r\n\r\n",
                          l_opt->d_port,l_opt->target);
        if(l_opt->tracert == 1) {
           snprintf(l_filtr,TP_FILTER_SIZE,"( (tcp dst port %d and src port %d) and dst host %s) "
                                           "or ( (icmp[icmptype]==icmp-timxceed) and (dst host %s) )",
                                           l_opt->s_port,l_opt->d_port,l_sip,l_sip);
                           
           if(wpcap_set_filter(l_pcap,l_filtr,0,0) != 1) {
               TPROBE_OUT(" [-].Set Filter Error: [%s] !!\r\r",l_filtr);
               goto __packet__;                                        
           }    
           TPROBE_OUT(" [+].Trace to target: (%s)\r\n\r\n",l_opt->target);
           while(send_tracert(l_pcap,l_packet,l_opt->target,l_opt->packet_size) != 0);
        }
        l_return = 0;
    } 
    __packet__: net_packet_free(l_packet);
    __pcap__: wpcap_free(l_pcap);                 
    __winsock__: univ_winsock_free();
    __ui__: tprobe_ui_free(l_opt);         
   
    return l_return;
}

