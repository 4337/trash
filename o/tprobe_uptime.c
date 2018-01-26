#define HAVE_REMOTE
#define WPCAP

#include <windows.h>
#include <time.h>
#include <pcap.h>
#include <math.h>
#include "tprobe_runtime.h"
#include "univ_packet.h"
#include "tprobe_uptime.h"
#include "univ_mem.h"


void tprobe_uptime_calculate(unsigned long i_cfreq,unsigned long i_lts,struct tm* i_out_time)
{
     i_out_time->tm_mday = (i_lts / i_cfreq) / 86400;
     i_out_time->tm_sec  = (i_lts / i_cfreq) % 86400;
     i_out_time->tm_hour = i_out_time->tm_sec / 3600;
     i_out_time->tm_sec  = i_out_time->tm_sec % 3600;
     i_out_time->tm_min  = i_out_time->tm_sec / 60;
     i_out_time->tm_sec  = i_out_time->tm_sec % 60;
}

unsigned long tprobe_uptime_calc_freq(unsigned long i_raw_freq)
{
         switch(i_raw_freq)
         {
             case 0:  return 1; 
             case 1 ... 10:    return i_raw_freq;
             case 11 ... 50:   return (i_raw_freq + 3) / 5 * 5;
             case 51 ... 100:  return (i_raw_freq + 7) / 10 * 10;
             case 101 ... 500: return (i_raw_freq + 33) / 50 * 50; 
             default: return (i_raw_freq + 67) / 100 * 100;                 
         }
}

/*
   ¯eby próbki by³y w miarê poprawne musimy 
   po pierwsze zapisaæ czas przyjœcia pierwszego pakietu odpowiedzi w sekundach 
   oraz pierwszego timestampu.
   potem odj¹æ  czas pierwszego pakietu od  czasu pakietów nastêpnych 
   Zak³adaj¹c ¿e chcemy obliczyæ  iloœæ tikniêæ w czasie jednej sekundy 
   sprawdzamy czy rezultat tego odejmowania jest wiêkszy b¹dŸ równy 1 
   Je¿eli jest mniejszy :), wysy³amy/odbieramy kolejne pakiety
   Je¿eli jest wiêkszy ,wynik odejmowania timestampów dzielimy przez wynik odejmowania 
   czasów przyjœcia pakietów .
   Je¿eli jest równy,wynik odejmowania timestampów zostaje taki jak jest 
   Sprawdzamy czy wynik odejmowania timestampów ma tendencje rosn¹c¹ 
   jeœli tak przyjmujemy ¿e jest on poprawny jeœli nie przyjmujemy ¿e jest chujowy
*/


struct tprobe_raw_uptime* tprobe_uptime_init(pcap_t* i_dsc,unsigned char* i_packet)
{
       if((i_dsc != NULL) && (i_packet != NULL)) {
          unsigned char* l_p_copy = (unsigned char*)univ_malloc(sizeof(struct tcpip4_packet));
          if(l_p_copy != NULL) {  
             struct u_tcp_option tcp_options[5] = {{UTCP_OPT_WSOPT,UTCP_FILL_SIZE(0x03,0x0a0a0a0a),0x03},
                                                   {UTCP_OPT_NOP,0x00,1},
                                                   {UTCP_OPT_MSS,UTCP_FILL_SIZE(0x04,245789234),4},
                                                   {UTCP_OPT_TS,UTCP_FILL_SIZE(0x0a,0xffffffff),10},
                                                   {UTCP_OPT_SACKP,2,2}};
             memcpy(l_p_copy,i_packet,sizeof(struct tcpip4_packet));
             if((l_p_copy = tcpip4_add_tcp_options(l_p_copy,(struct u_tcp_option*)&tcp_options,5)) != NULL) {/* dodajemy opcje do pakietu (w³¹czamy timestamp) */
                 char l_err = 0;   
                 int  l_ret = 0,l_loop = 0;                                                                  /* return dla pcap_next_ex i licznik pêtli*/                          
                 struct tcp_seqack l_numbers;
                 unsigned long l_first_time = 0;                                                             /* zmienna na czas pierwszego pakietu */ 
                 unsigned long l_ts_probe[2] = {0};                                                          /* tablica timestamp-ów */
                 struct pcap_pkthdr* l_head = NULL;                                                          /* pcap potrzebuje */
                 const u_char* l_remote_pkt = NULL; 
                 struct tprobe_raw_uptime* l_raw_ut = NULL;
                 unsigned int l_topt_size   = u_tcpoptions_align_size(tcp_options,5);
                   
                 l_numbers.rseq = htonl((unsigned long)tcp_generate_ins() << 2);                             /* generujemy numery sekwencyjne pakietu */
                 l_numbers.rack = htonl((unsigned long)tcp_generate_ins());       
                 
                 while(((l_ts_probe[0] == 0) || (l_ts_probe[1] == 0)) && (l_err < (NO_RESPONSE_ERROR_CNT * 2))) {  /* dopóki nie ma obó próbek i liczba "b³êdów" jest ok*/
                       tcp_syn((struct tcpip4_packet*)l_p_copy,&l_numbers,l_topt_size,0);                     /* ustawiamy flagi */                    
                       if(pcap_sendpacket(i_dsc,l_p_copy,sizeof(struct tcpip4_packet) + l_topt_size) != 0) l_err++;
                       else if((l_ret = pcap_next_ex(i_dsc,&l_head,&l_remote_pkt)) < 0) l_err++;
                       else if(l_ret == 0) l_err++;
                       else {
                            unsigned char l_ip_opt_len  = ip4_got_options(&((struct tcpip4_packet*)l_remote_pkt)->ip);
                            unsigned char l_tcp_opt_len = tcp_got_options(&((struct tcpip4_packet*)(l_remote_pkt + l_ip_opt_len))->tcp);
                            tcp_get_seqack((void*)&((struct tcpip4_packet*)((char*)(l_remote_pkt + l_ip_opt_len)))->tcp,&l_numbers);      /* potrzebne dla rst */
                            
                            if(l_tcp_opt_len == 0) l_err++;                                                      /* pakiet tcp nie ma opcji */
                            else {
                                 unsigned char* l_ts = u_tcpoptions_get_option((unsigned char*)l_remote_pkt + sizeof(struct tcpip4_packet) + 
                                                                               l_ip_opt_len,UTCP_OPT_TS,l_tcp_opt_len);
                                 if(l_ts != NULL) {                                                              /* pakiet ma timestamp */      
                                    if(l_first_time == 0) l_first_time = l_head->ts.tv_sec;  
                                    if(l_ts_probe[0] < l_ts_probe[1]) break;
                                    else l_ts_probe[1] = 0;                                                 
                                    if((l_head->ts.tv_sec - l_first_time) >= TP_TICK_SAMPLE) {      
                                        l_ts_probe[l_loop] = (unsigned long)htonl(*((unsigned int*)(l_ts + 2)));
                                        l_loop++;
                                    }                   
                                    while(pcap_next_ex(i_dsc,&l_head,&l_remote_pkt) > 0);       /* wyjebaæ czy nie ?!  */              
                                 }
                                 else l_err++;                                                                /* pakiet nie ma timestampu */
                            }
                            tcp_rst((struct tcpip4_packet*)l_p_copy,&l_numbers,l_topt_size,0);
                            pcap_sendpacket(i_dsc,l_p_copy,sizeof(struct tcpip4_packet) + l_topt_size);        /* error handling ? */  
                       }
                 }
                 univ_free(l_p_copy);  
                 if(l_ts_probe[1] != 0) {                                                                     /* uh div by null */
                    l_raw_ut = (struct tprobe_raw_uptime*)univ_malloc(sizeof(struct tprobe_raw_uptime));
                    if(l_raw_ut != NULL) {
                       l_raw_ut->_last_ts    = l_ts_probe[1];    
                       l_raw_ut->_clock_freq = (unsigned long)((l_ts_probe[1] - l_ts_probe[0]) / (l_head->ts.tv_sec - l_first_time));
                       return l_raw_ut;
                    }
                 }
             }         
          }        
       }
       return NULL;
}

