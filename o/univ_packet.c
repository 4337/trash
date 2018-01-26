#include <stdio.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <string.h>
#include "univ_net_utils.h"
#include "univ_packet.h"

unsigned char* tcpip4_add_tcp_options(unsigned char* i_packet,struct u_tcp_option* i_options,unsigned int i_opt_cnt) /*FIX ADD ip_opt_len */
{
    if(i_packet != NULL) {
       if(i_opt_cnt > 0) {
               unsigned char* l_packet = NULL;
               size_t l_opt_size = u_tcpoptions_align_size(i_options,i_opt_cnt);
               l_packet = (unsigned char*)univ_realloc(i_packet,sizeof(struct tcpip4_packet) + l_opt_size);   /* FIXME : */
            
               if(l_packet != NULL) {
                   char l_success = 1;
                   i_packet = l_packet;
                   
                   u_tcpoptions_set_option(i_packet + sizeof(struct tcpip4_packet),i_options,i_opt_cnt,l_opt_size); 

                   ((struct tcpip4_packet*)i_packet)->ip.total_len  = (unsigned short)htons(sizeof(struct tcp_packet) +
                                                                                            sizeof(struct ip4_packet) +
                                                                                            (l_opt_size));
                   ((struct tcpip4_packet*)i_packet)->ip.ch_sum     = 0;
                   ((struct tcpip4_packet*)i_packet)->ip.ch_sum     = univ_net_checksum(&((struct tcpip4_packet*)i_packet)->ip,
                                                                                         sizeof(struct ip4_packet));
                   ((struct tcpip4_packet*)i_packet)->tcp.hdr_len   = (sizeof(struct tcp_packet) + l_opt_size) /  NET_DWORD_SIZE; 
                   if(tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + l_opt_size) == 0) l_success = 0;
                   if(l_success != 0) return i_packet; 
               }
               univ_free(i_packet);
            }
    }
    return NULL;      
}

int eth2_init(struct eth_packet* i_packet,char* i_sip,
              char* i_dip,unsigned short i_type)
{ 
   
    DWORD l_ret;                      
    unsigned long l_mac[6];              
    unsigned char* l_mptr;               
    PIP_ADAPTER_INFO l_info;
    IPAddr l_dst_ip,l_src_ip,l_gate;        
    unsigned long l_mlen = NET_MAC_SIZE;

    l_info = univ_iface_info_init();      /* Pobieramy informacje dt.karty sieciowej */
    if(l_info == NULL) return -1;
    
    l_dst_ip = inet_addr(i_dip);          /* ustawiamy docelowy ip dla pakietu arp "who has" */
    l_src_ip = inet_addr(i_sip);          /* ustawiamy Ÿród³owy adress dla pakietu arp */ 
    l_gate   = inet_addr(l_info->GatewayList.IpAddress.String); /* gdyby ip docelowe nie nale¿a³o do sieci lokalnej
                                                                   ustawiamy jako docelowy adres Bramy */
    l_mptr = (unsigned char*)l_mac;  /* obecnoœæ tej "zmiennej" jest dyskusyjna ;)*/                    
    
    if(stricmp(i_dip,"127.0.0.1")) {        /* gdyby chodzi³o o loopback ustawiamy mak src i dst na nasz w³asny mak (FIX IT !)*/
       l_ret = SendARP(l_dst_ip,l_src_ip,l_mac,&l_mlen);    /* wys³amy zapytanie arp o docelowy adress ip */
       if(l_ret != NO_ERROR)  { /* send request for gate */    /* jeœli siê nie uda³o */
          l_mlen = NET_MAC_SIZE;
          l_ret = SendARP(l_gate,l_src_ip,l_mac,&l_mlen);  /* próbujemy wys³aæ zapytanie o brame */
          if(l_ret != NO_ERROR) {
             univ_iface_info_free(l_info);                 /* jeœli kolejny b³¹d to niestetsy zwracamy 0 i koñczymy zabawê*/ 
             return 0;                                     /* nic wiêcej zrobiæ nie mo¿emy */
          }
       }         
     } else memcpy(l_mptr,l_info->Address,l_mlen);   /* loopback */
     
     i_packet->s_eth[0] = l_info->Address[0];      /*fill eth_packet */
     i_packet->s_eth[1] = l_info->Address[1];      /* Wype³niamy nag³ówek protoko³u */
     i_packet->s_eth[2] = l_info->Address[2];      /* uzyskanymi wartoœciami */
     i_packet->s_eth[3] = l_info->Address[3];      /* mak Ÿród³owy */
     i_packet->s_eth[4] = l_info->Address[4];
     i_packet->s_eth[5] = l_info->Address[5];
     
     i_packet->d_eth[0] = l_mptr[0];              /* a teraz docelowy mak */
     i_packet->d_eth[1] = l_mptr[1];
     i_packet->d_eth[2] = l_mptr[2];
     i_packet->d_eth[3] = l_mptr[3];
     i_packet->d_eth[4] = l_mptr[4];
     i_packet->d_eth[5] = l_mptr[5];    

     i_packet->type = htons(i_type);  /* ustawiamy rodzaj protoko³u warstwy wy¿szej */
   
    univ_iface_info_free(l_info);   /* zwalniamy  zasoby */
    return 1;                      /* hurra mamy to co chcemy */
}

unsigned char tcp_got_options(struct tcp_packet* i_tcp)
{
              unsigned char l_optlen = (unsigned char)sizeof(struct tcp_packet) / 
                                        NET_DWORD_SIZE;
              if(i_tcp->hdr_len > l_optlen) 
                 return (i_tcp->hdr_len - l_optlen) * NET_DWORD_SIZE;
              return 0;        
}

unsigned char ip4_got_options(struct ip4_packet* i_ip)
{
              unsigned char l_optlen = (unsigned char)sizeof(struct ip4_packet) / 
                                        NET_DWORD_SIZE;
              if(i_ip->h_len > l_optlen) 
                 return (i_ip->h_len - l_optlen) * NET_DWORD_SIZE;
              return 0;
}

unsigned char*  tcpip4_add_data(unsigned char* i_packet,
                                char* i_data,size_t i_opt_off,size_t i_size)   /* FIX IT !!!!! */
{
                unsigned char* l_packet = (unsigned char*)univ_realloc(i_packet,
                                                                       sizeof(struct tcpip4_packet) + i_opt_off + 
                                                                       i_size);
                 
                if(l_packet == NULL) {
                   univ_free(i_packet);
                   return NULL;         
                }
                
                i_packet = l_packet;
                
                if(i_data != NULL)
                   memcpy(i_packet + 
                          sizeof(struct tcpip4_packet) + i_opt_off,
                          i_data,
                          i_size);
              
                ((struct tcpip4_packet*)i_packet)->ip.total_len  = (unsigned short)htons(sizeof(struct tcp_packet) +
                                                                         sizeof(struct ip4_packet) + i_opt_off + 
                                                                         i_size);
                ((struct tcpip4_packet*)i_packet)->ip.ch_sum     = 0;
                ((struct tcpip4_packet*)i_packet)->ip.ch_sum     = univ_net_checksum(&((struct tcpip4_packet*)i_packet)->ip,
                                                                                           sizeof(struct ip4_packet));
                                                            
                if(tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_size + i_opt_off) == 0) {  /* ideksowanie zerowe ? */                                          
                   univ_free(i_packet);
                   return  NULL;                                            
                }
                return i_packet;
}

/*   */
unsigned short univ_net_checksum(void* i_heads,int i_head_size)
{
               long l_chsum = 0;
               unsigned short* l_heads = (unsigned short*)i_heads;
               
               while(i_head_size > 1) {
                     l_chsum += *l_heads++;
                     i_head_size -= sizeof(unsigned short);           
               }
        
               if(i_head_size == 1) {
                  unsigned short l_odd = 0;
                  *((unsigned char*)&l_odd) = *(unsigned char*)l_heads;  
                  l_chsum += l_odd;
               }
               	
               l_chsum = (l_chsum >> 16) + (l_chsum & 0xffff);
       	       l_chsum = l_chsum + (l_chsum >> 16);
       	       
               return (unsigned short)~l_chsum;      
}

void  ip4_init(struct ip4_packet* i_iphead,char* i_sip,
               char* i_dip,unsigned char i_type,
               unsigned short i_totlen)
{   

               i_iphead->version    = 4;
	           i_iphead->h_len      = sizeof(struct ip4_packet) / NET_DWORD_SIZE;	
	           i_iphead->tos        = 0;
               i_iphead->total_len  = htons(i_totlen);
	           i_iphead->ident      = htons(univ_random_range(0,GetTickCount()));
               i_iphead->frag       = 0;
	           i_iphead->reserved   = 0;
	           i_iphead->df         = 1;
               i_iphead->mf         = 0;
	           i_iphead->frag1      = 0;
	           i_iphead->ttl        = NET_DEF_TTL;
	           i_iphead->proto      = i_type;
               i_iphead->source_ip  = inet_addr(i_sip);   
	           i_iphead->dest_ip    = inet_addr(i_dip);
               i_iphead->ch_sum     = 0;
               i_iphead->ch_sum     = univ_net_checksum(i_iphead,
                                                        sizeof(struct ip4_packet));
            
}

/*
   i_sip = source ip 
   i_dip = dest ip
   i_sport = source port
   i_dport = dest port
*/
unsigned char* tcpip4_init(char* i_sip,char* i_dip,                       
                           unsigned short i_sport,
                           unsigned short i_dport)
{             
               unsigned char* l_tcpip = (unsigned char*)univ_malloc(sizeof(struct tcpip4_packet));
               if(l_tcpip == NULL) return NULL;
               
               memset(l_tcpip,0,sizeof(struct tcpip4_packet)); 
                 
               if(eth2_init(&((struct tcpip4_packet*)l_tcpip)->eth,
                              i_sip,i_dip,0x0800) != 1) {
                  univ_free(l_tcpip);
                  return NULL;                                          
               }
               
               ip4_init(&((struct tcpip4_packet*)l_tcpip)->ip,
                         i_sip,i_dip,NET_PROTO_TCP,
                         sizeof(struct tcp_packet) + 
                         sizeof(struct ip4_packet)); 
              
               ((struct tcpip4_packet*)l_tcpip)->tcp.src_port   = htons(i_sport);
               ((struct tcpip4_packet*)l_tcpip)->tcp.dst_port   = htons(i_dport);                     
               ((struct tcpip4_packet*)l_tcpip)->tcp.hdr_len    = sizeof(struct tcp_packet) / NET_DWORD_SIZE;
               ((struct tcpip4_packet*)l_tcpip)->tcp.syn        = 1;
               ((struct tcpip4_packet*)l_tcpip)->tcp.win_size   = htons(NET_TCPWIN_SIZE);
           
               return l_tcpip;    
}

/* i_packet = eth + ip + tcp 
   i_size   = tcp_size  + data_size (if exists)
*/
unsigned short tcp_calculate_chsum(void* i_packet,size_t i_size)
{       
                unsigned char* l_pseg = (unsigned char*)
                                         univ_malloc(i_size + 
                                                     sizeof(struct pseudo_hdr));
                if(l_pseg == NULL) return 0;

                memset(l_pseg,0,i_size + sizeof(struct pseudo_hdr));
                
                ((struct pseudo_hdr*)l_pseg)->sip   = ((struct tcpip4_packet*)i_packet)->ip.source_ip; 
                ((struct pseudo_hdr*)l_pseg)->dip   = ((struct tcpip4_packet*)i_packet)->ip.dest_ip;
                ((struct pseudo_hdr*)l_pseg)->proto = ((struct tcpip4_packet*)i_packet)->ip.proto;
                ((struct pseudo_hdr*)l_pseg)->len   = htons(i_size);
               
                ((struct tcpip4_packet*)i_packet)->tcp.ch_sum = 0;
                
                memcpy(l_pseg + sizeof(struct pseudo_hdr),
                       &((struct tcpip4_packet*)i_packet)->tcp,i_size);
                           
                ((struct tcpip4_packet*)i_packet)->tcp.ch_sum = univ_net_checksum(l_pseg,i_size + 
                                                                                        sizeof(struct pseudo_hdr));
                univ_free(l_pseg);
                return ((struct tcpip4_packet*)i_packet)->tcp.ch_sum;       
}

void tcp_get_seqack(void* i_rpacket,struct tcp_seqack* i_nums)
{
     i_nums->rseq = ntohl(((struct tcp_packet*)i_rpacket)->seq_number);
     i_nums->rack = ntohl(((struct tcp_packet*)i_rpacket)->ack_number);
}

void tcp_syn(struct tcpip4_packet* i_packet,struct tcp_seqack* i_usles,size_t i_optlen,size_t i_dsize)
{  
     i_packet->tcp.seq_number = htonl(tcp_generate_ins());
     i_packet->tcp.syn = 1;
     i_packet->tcp.ack = 0;
     i_packet->tcp.psh = 0;  
     i_packet->tcp.rst = 0;
     i_packet->tcp.fin = 0;
     i_packet->tcp.urg = 0;
     tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_optlen + i_dsize);                /*syn don't have data*/
}

void tcp_synack(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize)
{
     i_packet->tcp.seq_number = htonl(tcp_generate_ins());                             /*first packet*/
     i_packet->tcp.ack_number = htonl(tcp_ack_number(i_nums->rseq));
     i_packet->tcp.syn = 1;
     i_packet->tcp.ack = 1;
     i_packet->tcp.psh = 0;  
     i_packet->tcp.rst = 0;
     i_packet->tcp.fin = 0;
     i_packet->tcp.urg = 0;
     tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_optlen + i_dsize);
}

void tcp_ack(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize)
{
     i_packet->tcp.seq_number = htonl(i_nums->rack);                         
     i_packet->tcp.ack_number = htonl(tcp_ack_number(i_nums->rseq));
     i_packet->tcp.syn = 0;
     i_packet->tcp.ack = 1;   
     i_packet->tcp.psh = 0;  
     i_packet->tcp.rst = 0;
     i_packet->tcp.fin = 0;
     i_packet->tcp.urg = 0;
     tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_optlen + i_dsize);
}

void tcp_fin(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize)
{
     i_packet->tcp.seq_number = htonl(i_nums->rack);                           
     i_packet->tcp.ack_number = htonl(tcp_ack_number(i_nums->rseq));
     i_packet->tcp.syn = 0;
     i_packet->tcp.fin = 1;
     i_packet->tcp.urg = 1;
     i_packet->tcp.ack = 0;
     i_packet->tcp.psh = 0;  
     i_packet->tcp.rst = 0;
     tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_optlen + i_dsize);
}

void tcp_rst(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize)
{
     i_packet->tcp.seq_number = htonl(i_nums->rack);                           
     i_packet->tcp.ack_number = htonl(tcp_ack_number(i_nums->rseq));
     i_packet->tcp.urg = 0;
     i_packet->tcp.ack = 0;
     i_packet->tcp.psh = 0;  
     i_packet->tcp.fin = 0;
     i_packet->tcp.syn = 0;
     i_packet->tcp.rst = 1;
     tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_optlen + i_dsize);
}

void tcp_malformed(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize)  
{
     long int* l_ptr; 
     unsigned char l_point;
     unsigned char l_hsize = sizeof(struct tcpip4_packet) + i_dsize - sizeof(struct eth_packet);
     long int      l_rand  = univ_random_range(-2147483644,2147483644);
     
     i_packet->tcp.seq_number = htonl(i_nums->rack);                           
     i_packet->tcp.ack_number = htonl(tcp_ack_number(i_nums->rseq));
      
     __lable:
             
     l_point = univ_random_range(sizeof(struct eth_packet),l_hsize);
     
     if(l_point + 4 > l_hsize) l_point -= 4;
     
     if((((long)&i_packet[l_point] >= (long)&i_packet->ip.ch_sum) && 
        ((long)&i_packet[l_point] < (long)&i_packet->tcp.seq_number)) ||
        (((long)&i_packet[l_point] > (long)&i_packet->ip.ident) &&
        ((long)&i_packet[l_point] <  (long)&i_packet->ip.proto)))  goto __lable;
     
     l_ptr  = (long*)&i_packet[l_point];
     *l_ptr = l_rand;            
             
     tcp_calculate_chsum(i_packet,sizeof(struct tcp_packet) + i_optlen + i_dsize);
}


