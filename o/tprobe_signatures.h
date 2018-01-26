#ifndef TPROBE_SIGNATURES_H
#define TPROBE_SIGNATURES_H

#define SIG_MULITIVAL_DELIM ","
#define SIG_MULITIVAL_DELIM_CHR ','

#define SIG_IP_OPTION_LEN 16
#define SIG_TCP_OPTION_LEN 16

#define SIG_IP_TTL 50702
#define SIG_IP_TOS 38048
#define SIG_IP_DF 40041
#define SIG_IP_MF 24052
#define SIG_FR_OFF 17472
#define SIG_TCP_WIN_SIZE 60103
#define SIG_TCP_PRIOR 14733
#define SIG_TCP_OPTION 46606
#define SIG_IP_H_LEN 32796
#define SIG_NET_DATA 60493
#define SIG_TCP_FLAGS 1234
#define SIG_TCP_ACK 11296
#define SIG_SIG_NAME 62487
#define SIG_IP_OPTION 56011
#define SIG_IP_IDENT 27940
#define SIG_TCP_SEQ 15331
/* //obsolete\\
#define SIG_IPIDENT_NOZERO 26819
#define SIG_TCPACK_NOZERO 10791
#define SIG_TCPSEQ_NOZERO 27394
*/

#define SIG_FILL 0

#define SIG_MAX_MTUS 7
#define SIG_MTUS_CNT 14

struct mtu_db
{
       char* name;
       unsigned short value[SIG_MAX_MTUS + 1];
       unsigned int cnt;
};


/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  differnces between types forced us to define many difrent functions and structures
  thats make basicly the same thing 
  In C++ we could use template but here we cant 
*/

struct s_multi_ulong
{
	   size_t amount;
	   union _ptr_ulong
	   {
		      unsigned long* ack;                          
              unsigned long* seq;    
			  unsigned long* tunion;
	   }ptr;
};

/*
  ...
*/
int tprobe_set_multivalue_ulong(char* i_buff,struct s_multi_ulong* o_multi_ulong);

struct s_multi_short
{
	   size_t amount;
	   union _ptr_short
	   { 
		     unsigned short* ident;
		     unsigned short* winsize;
			 unsigned short* tcp_prior;
		     unsigned short* tunion;
	   }ptr;
};

/*
   IN i_buff = ptr to rvalue in sig file
      o_multi_short = ptr to t s_multi_short
   OUT : amount || -1 alloc error
*/
int tprobe_set_multivalue_short(char* i_buff,struct s_multi_short* o_multi_short);

struct s_multi_ptr_uc
{
	   size_t amount;
	   union _ptr_ptr_uc
	   {
		     unsigned char** tcp_option;
			 unsigned char** ip_option;
             unsigned char** tunion;
	   }ptr;
};

/*
  IN i_buff = ptr to rvalue in sig file
     o_multi_ptr_uc = ptr 2 s_multi_ptr_uc 
  OUT -1 = error alloc || amount
*/
int tprobe_set_multivalue_ptr_uc(char* i_buff,struct s_multi_ptr_uc* o_multi_ptr_uc);

struct s_multi_uc
{
	   size_t amount;
	   union _ptr_uc{
	         unsigned char* df;
			 unsigned char* mf;
             unsigned char* ip_fr;
			/* unsigned char* ident_nozero; */
			 unsigned char* tcp_flags;
			 unsigned char* ttl;
			 unsigned char* ip_hlen;  
			 unsigned char* ip_tos; 
			 unsigned char* tunion;
			/* unsigned char* ack_nozero; */
			/* unsigned char* seq_nozero; */
	   }ptr;
};

/*
  IN ...
  OUT -1=error || amount
*/
int tprobe_set_multivalue_uc(char* i_buff,struct s_multi_uc* o_multi);

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

struct signatures_db 
{
       unsigned int f_cnt;
       struct signatures_db* next;
       struct signatures_db* prev;
       char* section; 
       size_t current;
       float procent;
       char* name;
	   struct s_multi_uc  ip_ttl;
       struct s_multi_uc  ip_df;             /* 1 || 0 */
       struct s_multi_uc  ip_mf;             /* 1 || 0 */
       struct s_multi_uc  ip_fr_off;         /* 0 || N>0 */
	   struct s_multi_short ip_ident;        /* 0 || N>0 */
       struct s_multi_uc ip_hlen;            /* wielowartoœæ  */
	   struct s_multi_ptr_uc ip_option;      /* opconalne zmienne obs³uga multi i * nie wpe³ni zaimplementowane */
	   struct s_multi_short tcp_winsize;     /* 0 || N>0 wiele wartoœci */
       struct s_multi_uc ip_tos;             /* 0 || 255 */
       struct s_multi_short tcp_prior;      
	   struct s_multi_ptr_uc tcp_option;     /* opcjonalne zmienne obs³uga multi i  * */
	   struct s_multi_uc tcp_flags; 
       struct s_multi_ulong  tcp_ack;        /* 0 || N */      
       struct s_multi_ulong  tcp_seq;        /* 0 || N */       
       char*          net_data;   
      /* struct s_multi_uc  ip_ident_nozero;  */ 
      /* struct s_multi_uc  tcp_ack_nozero;   */
      /* struct s_multi_uc  tcp_seq_nozero;   */       
};

/* Create and init signatures_db
   IN:void
   RETURN: pointer or NULL
*/
struct signatures_db* tprobe_sig_init(void);

/* Parse file that contains tcp/ip fingerprints 
   IN: i_sig = pointer to structure returned by pprobe_sig_init
       i_file_name = path to file
   RETURN: 1 = ON_SUCCESS or 0 = ERROR
*/
unsigned char tprobe_sig_parse_file(struct signatures_db* i_sig,char* i_file_name);

/* Add Section names to signatures_db 
   IN: i_section = Section Name
       i_len = Section Name Length
       i_sig = signatures_db
   Return: NULL || pointer to signatures_db
*/ 
struct signatures_db* tprobe_sig_add_section(char* i_section,size_t i_len,struct signatures_db* i_sig);

/* 
 Free ....... all .......
 IN: ........
 RETURN: void
*/
void  tprobe_sig_free(struct signatures_db* i_handle);

#endif 
