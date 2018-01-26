#ifndef TPROBE_OSF_H
#define TPROBE_OSF_H

#define TPROBE_SIG_NODEF '*'
#define TPROBE_MIN_SCORE 85.0

struct ip4_packet;
struct tcp_packet;
struct tcpip4_packet;
struct signatures_db;
struct s_multi_ptr_uc;
struct s_multi_uc;
struct s_multi_short;
struct s_multi_ulong;


/***************************************************************************************************/

unsigned char tprobe_cmp_tcp_winsize(struct tcp_packet* i_tcp_pkt,struct s_multi_short* i_tcp_winsize);    /* chck multivalue tcp.winsize */
unsigned char tprobe_cmp_ip_ttl(struct ip4_packet* i_ip4_pkt,struct s_multi_uc* ip_ttl);                     /* chck multivalue ip.ttl */
unsigned char tprobe_cmp_ip_ident(struct ip4_packet* i_ip4_pkt,struct s_multi_short* ip_ident);               /* chck multi ident */
unsigned char tprobe_cmp_tcp_flag(struct tcp_packet* i_tcp_pkt,struct s_multi_uc* sig_tcp_flags);             /* chck multi tcp f */
unsigned char tprobe_cmp_ip_opt(void* i_ip4_opt,struct s_multi_ptr_uc* sig_ip_opt,unsigned int opt_len);     /* chck multi ip opt */
unsigned char tprobe_cmp_tcp_opt(void* i_tcp_opt,struct s_multi_ptr_uc* sig_tcp_opt,unsigned int opt_len); /* chck multi tcp opt */
unsigned char tprobe_cmp_ip_df(struct s_multi_uc* sig_ip_df,struct ip4_packet* i_ip);
unsigned char tprobe_cmp_ip_mf(struct s_multi_uc* sig_ip_mf,struct ip4_packet* i_ip);
unsigned char tprobe_cmp_ip_fro(struct s_multi_uc* sig_ip_fr,struct ip4_packet* i_ip);
unsigned char tprobe_cmp_ip_tos(struct s_multi_uc* sig_ip_tos,struct ip4_packet* i_ip);
unsigned char tprobe_cmp_ip_hlen(struct s_multi_uc* sig_ip_hlen,struct ip4_packet* i_ip);
unsigned char tprobe_cmp_tcp_seq(struct s_multi_ulong* sig_tcp_seq,struct tcp_packet* i_tcp_pkt);
unsigned char tprobe_cmp_tcp_ack(struct s_multi_ulong* sig_tcp_ack,struct tcp_packet* i_tcp_pkt);
unsigned char tprobe_cmp_tcp_prior(struct s_multi_short* sig_tcp_prio,struct tcp_packet* i_tcp_pkt);

/***************************************************************************************************/

const char* tprobe_cmp_mtu(unsigned short i_mss); 

/*
 RETURN : 1=options are equal / 0=options not equal
*/
char tprobe_agresive_tcp_opt_cmp(unsigned char* i_sig_opt,unsigned char* i_p_opt,int i_len);
/*
 ...
*/
char tprobe_cmp_net_options(void* i_opt,void* i_sig_opt,unsigned int i_len);
/*
  ....
*/
float tprobe_cmp_signature(void* i_tcpip,struct signatures_db* i_sig,unsigned int i_ip_opt,unsigned int i_tcp_opt); 
/*
   ....
*/
unsigned char tprobe_sig_calculate_tcp_flags(struct tcp_packet* i_tcp);

/*
   Commpare pattern witch net_data 
   IN: i_pattern = pattern
       i_data = pointer to net_data
   RETURN: info or NULL = ERROR
*/
char* tprobe_sig_cmp_netdata(char* i_pattern,char* i_data);

/* Commpare remote ip header witch sgignatures 
   IN: i_ip = ip header structure
       i_sig = pointer to signature 
   RETURN: 0.0 = ERROR , Procent = SUCCESS
*/
float tprobe_sig_cmp_ip(struct ip4_packet* i_ip,struct signatures_db* i_sig,unsigned int i_opt_len);

/* 
  Commpare remote tcp header witch signatures
  IN:....
  RETURN: yes ;)
*/
float tprobe_sig_cmp_tcp(struct tcp_packet* i_tcp,struct signatures_db* i_sig,unsigned int i_opt_len);



#endif  /*...*/
