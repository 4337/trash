#ifndef UNIV_TCP_OPTIONS_H
#define UNIV_TCP_OPTIONS_H


/* http://www.networksorcery.com/enp/protocol/tcp.htm */

#define UTCP_OPT_QUIT_ON_ERROR 1 /* quit on unknow tcp option u_tcpoptions_get_option() */

#define UTCP_OPT_EOOL  0  /* end of options */
#define UTCP_OPT_NOP   1  /* no opperation */
#define UTCP_OPT_MSS   2  /* maximum segment size */
#define UTCP_OPT_WSOPT 3  /* window scale factor */
#define UTCP_OPT_SACKP 4  /* selective ack permitted */
#define UTCP_OPT_SACK  5  /* selective ack */
#define UTCP_OPT_ECHO  6  /* obslete echo */
#define UTCP_OPT_ECHOR 7  /* obslete echo reply */
#define UTCP_OPT_TS    8  /* timestamp */
#define UTCP_OPT_POCP  9  /* Partial Order Connection permitted. */
#define UTCP_OPT_POSP  10 /* Partial Order service profile.*/
#define UTCP_OPT_CC    11 /* CC, Connection Count. */
#define UTCP_OPT_CCNEW 12 /* CC New */
#define UTCP_OPT_CCECHO 13 /* CC Echo */
#define UTCP_OPT_ACHR   14 /* Alternate checksum request. */
#define UTCP_OPT_ACHSUM 15 /* Alternate checksum data. */
#define UTCP_OPT_SKEETE 16 /* Skeeter */
#define UTCP_OPT_BUBBA  17 /* Bubba */
#define UTCP_OPT_TCHO   18 /* Trailer checksum option */
#define UTCP_OPT_MD5SIG 19 /* Md5 signature option obslete */
#define UTCP_OPT_SCPS   20 /* SCPS Capabilities */
#define UTCP_OPT_SNACK  21 /* Selective Negative Acknowledgements */
#define UTCP_OPT_RB     22 /* Record bounduaries */
#define UTCP_OPT_CE     23 /* Corruption experienced */
#define UTCP_OPT_SNAP   24 /* SNAP */
#define UTCP_OPT_UNASIG 25 
#define UTCP_OPT_TCPCF  26 /* commpression filter */
#define UTCP_OPT_QSR    27 /* Quick-Start Response */
#define UTCP_OPT_UTO    28 /* User time out */
#define UTCP_OPT_AUTH   29 /* TCP Authentication Option */
#define UTCP_OPT_MPTCP  30 /* draft-ietf-mptcp-multiaddressed-07 */
/*
  31-254 - reserved/\experiment
*/

struct u_tcp_timestamp
{
       unsigned short _lt;
       unsigned int _val;
       unsigned int _reply;    
};

struct u_tcp_option
{
       unsigned int _kind;
       unsigned int _val;    
       unsigned int _length;
};

size_t u_tcpoptions_raw_size(struct u_tcp_option* i_options,unsigned int i_opt_cnt);
size_t u_tcpoptions_align_size(struct u_tcp_option* i_options,unsigned int i_opt_cnt);

#define UTCP_ACK_SIZE(x) (x + 8)
#define UTCP_FILL_SIZE(a,b) ((b & 0xffffff00) | a) /*a=length , b=trash */

int UTCP_GET_LENGTH_BY_KIND(unsigned char* i_kind_ptr);
/*
 IN: i_options = pointer to memmory that store tcp_options
     i_opt=option kind
     i_opt_len=options length
 RETURN: NULL=ERROR / pointer to option
*/
unsigned char* u_tcpoptions_get_option(unsigned char* i_options,unsigned int i_opt,int i_opt_len);

/* 
 IN: i_place_in_pkt = pointer to place for options in tcp_packet
     i_options = pointer to array of u_tcp_option structure
     i_opt_cnt = i_options array size
 RETURN : void
*/
void u_tcpoptions_set_option(unsigned char* i_place_in_pkt,struct u_tcp_option* i_options,unsigned int i_opt_cnt,size_t i_aligned_size);

#endif
