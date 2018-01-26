#define HAVE_REMOTE
#define WPCAP
#include <stdio.h>
#include <winsock2.h>
#include <pcap.h>
#include "univ_mem.h"
#include "tprobe_ui.h"
#include "univ_packet.h"
#include "tprobe_osf.h"
#include "tprobe_signatures.h"
#include "tprobe_net_ports.h"
#include "tprobe_runtime.h"
#include "wcallback.h"

void wpcap_tcp_callback(unsigned char* i_argv, 
                        const struct pcap_pkthdr* i_header,
                        const unsigned char* i_pkt_data)
{        
     int l_a,l_b;      
     struct in_addr l_daddr;                        
     unsigned char l_ip_opt,l_tcp_opt;
     
     l_daddr.s_addr = ((struct tcpip4_packet*)i_pkt_data)->ip.dest_ip;
     
     ((struct _TCP_CB_ARGV*)i_argv)->resp_ptr_buf = i_pkt_data;
     ((struct _TCP_CB_ARGV*)i_argv)->header       = i_header;
     
     l_ip_opt  = ip4_got_options(&((struct tcpip4_packet*)i_pkt_data)->ip);
     l_tcp_opt = tcp_got_options(&((struct tcpip4_packet*)(i_pkt_data + l_ip_opt))->tcp);

	 /* addd check if no options */
     
     TPROBE_OUT(" [+].Recv reply:    (TCP/IPv4) %s:%d <- %s:%d\r\n"
                "                      Mac's:  %02x-%02x-%02x-%02x-%02x-%02x <- %02x-%02x-%02x-%02x-%02x-%02x\r\n\r\n"
                " [+].Port status:   port (%d) on (%s) [%s] r_ttl: %d\r\n\r\n",                   
                inet_ntoa(l_daddr),
                ((struct ui_options*)(((struct _TCP_CB_ARGV*)i_argv))->opt_ptr)->s_port,
                ((struct ui_options*)(((struct _TCP_CB_ARGV*)i_argv))->opt_ptr)->target,
                ((struct ui_options*)(((struct _TCP_CB_ARGV*)i_argv))->opt_ptr)->d_port,
                ((struct tcpip4_packet*)i_pkt_data)->eth.d_eth[0],
                ((struct tcpip4_packet*)i_pkt_data)->eth.d_eth[1],
                ((struct tcpip4_packet*)i_pkt_data)->eth.d_eth[2],
                ((struct tcpip4_packet*)i_pkt_data)->eth.d_eth[3],
                ((struct tcpip4_packet*)i_pkt_data)->eth.d_eth[4],
                ((struct tcpip4_packet*)i_pkt_data)->eth.d_eth[5],
                ((struct tcpip4_packet*)i_pkt_data)->eth.s_eth[0],
                ((struct tcpip4_packet*)i_pkt_data)->eth.s_eth[1],
                ((struct tcpip4_packet*)i_pkt_data)->eth.s_eth[2],
                ((struct tcpip4_packet*)i_pkt_data)->eth.s_eth[3],
                ((struct tcpip4_packet*)i_pkt_data)->eth.s_eth[4],
                ((struct tcpip4_packet*)i_pkt_data)->eth.s_eth[5],
                ((struct ui_options*)(((struct _TCP_CB_ARGV*)i_argv))->opt_ptr)->d_port,
                ((struct ui_options*)(((struct _TCP_CB_ARGV*)i_argv))->opt_ptr)->target,
                tprobe_port_status(&((struct tcpip4_packet*)i_pkt_data)->tcp),
                ((struct tcpip4_packet*)i_pkt_data)->ip.ttl);
                     
      if(((struct ui_options*)(((struct _TCP_CB_ARGV*)i_argv))->opt_ptr)->pof != 0) { /* active os */   
          struct signatures_db* l_sig_ptr;
          TPROBE_OUT(" [+].Os f-prints: ");
  
          l_sig_ptr = tprobe_sig_init();   
          if((l_sig_ptr == NULL) || (tprobe_sig_parse_file(l_sig_ptr,TPROBE_SIG_FILENAME) == 0)) {
             TPROBE_OUT("fail (internal error [_sig_file_coruppted_])\r\n\r\n");          
          } else {
             char l_noempty = 0;
             struct signatures_db* l_next;
             
             for(l_next=l_sig_ptr;
                 l_next!=NULL;
                 l_next=l_next->next) {
                 tprobe_cmp_signature((void*)i_pkt_data,l_next,l_ip_opt,l_tcp_opt); 
                 
				 if(l_next->procent >= TPROBE_MIN_SCORE) {
                    if(l_noempty == 0) TPROBE_OUT("%s (%.1f%%)\r\n",l_next->name,l_next->procent);  
                    else  TPROBE_OUT("                  %s (%.1f%%)\r\n",l_next->name,l_next->procent); 
                   l_noempty++;
                 }      
             }
             if(l_noempty == 0) TPROBE_OUT("[unknow]\r\n");
             tprobe_sig_free(l_sig_ptr);  
          }  
      }            
      TPROBE_OUT("\r\n [+].Raw reply:  (RAW) | 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A \r\n"
             "                +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
             
      for(l_a=0;l_a<i_header->caplen;l_a += 10) {
          TPROBE_OUT("                | 0x%02x | ",l_a);                               
          for(l_b=0;
             ((l_b<10) && (l_a + l_b < i_header->caplen));
             l_b++) TPROBE_OUT("0x%02x ",*(i_pkt_data + l_a + l_b));
          TPROBE_OUT("\r\n");                                        
      }
      
      TPROBE_OUT("\r\n [+].RSEQ Number hex(0x%02lx) dec(%lu)\r\n"
                 "    .RACK Number hex(0x%02lx) dec(%lu)\r\n\r\n",
                 (unsigned long)ntohl(((struct tcpip4_packet*)(i_pkt_data + l_ip_opt))->tcp.seq_number),
                 ntohl(((struct tcpip4_packet*)(i_pkt_data + l_ip_opt))->tcp.seq_number),
                 (unsigned long)ntohl(((struct tcpip4_packet*)(i_pkt_data + l_ip_opt))->tcp.ack_number),
                 ntohl(((struct tcpip4_packet*)(i_pkt_data + l_ip_opt))->tcp.ack_number));
      
      if((l_a = tcp_got_options(&((struct tcpip4_packet*)(i_pkt_data + l_ip_opt))->tcp)) > 0) {
         unsigned char* l_got_mss = u_tcpoptions_get_option((unsigned char*)i_pkt_data + sizeof(struct tcpip4_packet) + l_ip_opt,UTCP_OPT_MSS,l_a);                           
         if(l_got_mss != NULL) {
            unsigned short* l_mss_p = ((unsigned short*)(l_got_mss + 2));
            unsigned short  l_mss   = ntohs(*l_mss_p);
            const char* l_mtu = tprobe_cmp_mtu(l_mss);
            if(l_mtu != NULL) TPROBE_OUT(" [+].Remote connection type guessed by MTU: (%s) MTU:%d\r\n\r\n",l_mtu,l_mss + sizeof(struct ip4_packet) + sizeof(struct tcp_packet));   
         }                          
      }               
}
