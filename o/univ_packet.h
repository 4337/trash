#ifndef UNIV_PACKETS_H
#define UNIV_PACKETS_H

#include <windows.h>
#include "univ_tcp_options.h"
#include "univ_random.h"
#include "univ_mem.h"
#include "univ_macros.h"

#define WAIT_FOR_LOST_PACKET 4500

#define NET_DEF_TTL 128
#define NET_MAC_SIZE 6   /* mac size */

#define NET_DWORD_SIZE 4

#define NET_PROTO_TCP 6
#define NET_PROTO_ICMP 1
#define NET_PROTO_UDP  17
 
#define NET_TCP_SYN 0x02
#define NET_TCP_ACK 0x10

#define NET_TCP_CWR 0x1
#define NET_TCP_ECN 0x2
#define NET_TCP_NS  0x4

#define NET_TCP_FIN 0x01
#define NET_TCP_RST 0x04
#define NET_TCP_PSH 0x08
#define NET_TCP_URG 0x20

#define NET_TCPWIN_SIZE 65535


struct eth_packet       
{
       unsigned char d_eth[NET_MAC_SIZE];
       unsigned char s_eth[NET_MAC_SIZE];
       unsigned short type;
};

#ifdef MSVC
#pragma pack(push,1)
#endif
struct ip4_packet               /**/
{   
       unsigned char h_len:4;          
       unsigned char version:4;    /*1*/     
       unsigned char tos;          /*2*/    
       unsigned short total_len;   /*4*/   
       unsigned short ident;       /*6*/    
       unsigned char  frag:5;       
       unsigned char  mf:1;
       unsigned char  df:1;
       unsigned char  reserved:1;  /*19*/
       unsigned char  frag1;       /*20*/   
       unsigned char ttl;          /*7*/      
       unsigned char proto;        /*8*/     
       unsigned short ch_sum;      /*10*/   
       unsigned long source_ip;    /*14*/
       unsigned long dest_ip;     /*18*/ 
}
#ifdef MSVC
;
#pragma pack(pop)
#else
__attribute__((__packed__));
#endif

#ifdef MSVC
#pragma pack(push,1)
#endif
struct tcp_packet
{
       unsigned short src_port;  /*2*/       
       unsigned short dst_port;   /*4*/   
       unsigned int   seq_number;      /*8*/ 
       unsigned int   ack_number;  /*12*/
       unsigned char  ns:1;       
       unsigned char  reserved1:3;     
       unsigned char  hdr_len:4;       /*13*/
       unsigned char  fin:1;          
       unsigned char  syn:1;
       unsigned char  rst:1;
       unsigned char  psh:1;
       unsigned char  ack:1;
       unsigned char  urg:1;
       unsigned char  ecn:1;
       unsigned char  cwr:1;     /*14*/
       unsigned short win_size;   /*16*/     
       unsigned short ch_sum;     /*18*/     
       unsigned short prior;      /*20*/          
}
#ifdef MSVC
;
#pragma pack(pop)
#else
__attribute__((__packed__));
#endif

#ifdef MSVC
#pragma pack(push,1)
#endif
struct pseudo_hdr
{
      unsigned long  sip;
      unsigned long  dip;
      unsigned char  zero;
      unsigned char  proto;
      unsigned short len;  
}
#ifdef MSVC
;
#pragma pack(pop);
#else
__attribute__((__packed__));
#endif

struct tcpip4_packet
{
       struct eth_packet eth;
       struct ip4_packet ip;
       struct tcp_packet tcp;
};

/* 
  Add tcp options to packet
  IN: ..
  RETURN: ..
*/
unsigned char* tcpip4_add_tcp_options(unsigned char* i_packet,struct u_tcp_option* i_options,unsigned int i_opt_cnt);
/*
  Initialize ethernet layer 
  including arp req/rep and all other shit like eg: geting SRCMAC address
  IN: i_packet = pointer to structure that contains erthernet frame format
      i_sip = source IP Address
      i_dip = destination IP Address
  RETURN: 1=OK , 0=ERROR , -1=INTERNAL_ERROR
*/
int eth2_init(struct eth_packet* i_packet,char* i_sip,char* i_dip,unsigned short i_type);

/*
  Check tcp.hdr_len if is bigger than X tcp got options 
  Return: tcpoptions size
*/
unsigned char tcp_got_options(struct tcp_packet* i_tcp);

/*
  Check hdr_len if is bigger than 5 ip got options 
  Return: ipoptions size
*/
unsigned char ip4_got_options(struct ip4_packet* i_ip);

/* Calculate Acknowledgment Number 
   IN: i_seq = previous sequence number
   RETURN: ACK_NUMBER 
*/
/*inline*/
static univ_inline
unsigned long tcp_ack_number(unsigned long i_number)
{
              return (i_number + 1);  
}
/* Generate INS number (needed when we send first packet)
   IN: void
   RETURN: INS number ;)
*/
/*inline*/
static univ_inline
unsigned long tcp_generate_ins(void)
{
              register unsigned long l_max = GetTickCount() * 2;
              return (unsigned long)univ_random_range(0,l_max) + l_max;
}

/*
Calculate checksum for IPv4/TCP/UDP/XXX header
IN: i_heads = pointer to packet buf
    i_head_size = size of packet buf
RETURN: CheckSum
*/
unsigned short univ_net_checksum(void* i_heads,int i_head_size);

/* Create and init TCP/IP4 packet 
   IN: i_sip = pointer to char that contains source ip address
       i_dip = pointer to char that contains destynation ip address
       i_sport = source port
       i_dport = destntion port 
       i_dsize = data size || 0 if no data
   RETURN: NULL = ERROR
           POINTER = NO_ERROR
*/
unsigned char* tcpip4_init(char* i_sip,char* i_dip,unsigned short i_sport,
                           unsigned short i_dport);


/*  init ipV4 header 
   IN:
   RETURN: void
*/
void  ip4_init(struct ip4_packet* i_iphead,char* i_sip,char* i_dip,
               unsigned char i_type,
               unsigned short i_totsie);

/* Free packet 
   IN: pointer to packet 
   RETURN: void
*/
/*inline*/
static univ_inline
void net_packet_free(unsigned char* i_packet)
{
     univ_free(i_packet); 
}
/* Add data to tcp/ip packet
  IN: i_packet = pointer to tcp/ip packet
      i_data =  data ;]
      i_opt_off = tcp option size
      i_size = data size 
  RETURN: char table that contains tcp/ip headers + data or NULL
*/
unsigned char* tcpip4_add_data(unsigned char* i_packet,char* i_data,size_t i_opt_off,size_t i_size);

/* Calculate packet checksum
   IN: i_packet = pointer to packet
       i_size  = size of packet witch data
   RETURN: checksum || 0 if error
*/
unsigned short tcp_calculate_chsum(void* i_packet,size_t i_size);

struct tcp_seqack
{
       unsigned long rseq;
       unsigned long rack;     
};

/* Filling tcp_seqack 
   IN: i_rpacket = pointer to memm that contains packet from remote host
       i_nums = pointer to tcp_seqack structure
   RETURN: Void
*/
void tcp_get_seqack(void* i_rpacket,struct tcp_seqack* i_nums);

/* Create TCP SYN Packet 
   IN: pointer to structure thath contains tcp_packet 
   RETURN: void
*/
void tcp_syn(struct tcpip4_packet* i_packet,struct tcp_seqack*,size_t i_optlen,size_t i_dsize);

/* Create TCP SYN/ACK Packet
   IN: pointer to structure that contains tcp_packet ;)
          i_nums = PREV SEQ NUM
          i_dsize = data size || 0
   RETURN:void
*/
void tcp_synack(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize);

/* Create TCP ACK Packet
   IN: pointer to structure that contains tcp_packet
          i_nums = PREV SEQ NUM
          i_dsize = data size || 0
   RETURN: void
*/
void tcp_ack(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize);

/* Create TCP FIN Packet
   IN: pointer to structure that contains tcp_packet
         i_nums = PREV SEQ NUM
         i_dsize = data size || 0
   RETURN: void
*/
void tcp_fin(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize);

/* Create TCP RST Packet
   IN: pointer to structure that contains tcp_packet
       i_nums = PREV SEQ NUM
       i_dsize = data size || 0
   RETURN: void
*/
void tcp_rst(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize);

/* Create TCP/ip Malformed Packet
   IN: pointer to structure that contains tcp_packet
         i_nums = PREV SEQ NUM
         i_dsize = data size || 0
   RETURN: void
*/
void tcp_malformed(struct tcpip4_packet* i_packet,struct tcp_seqack* i_nums,size_t i_optlen,size_t i_dsize);

#endif 
