#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "tprobe_signatures.h"
#include "univ_mem.h"
#include "univ_macros.h"
#include "univ_utils.h"

/* from p0f   http://lcamtuf.coredump.cx/p0f3.shtml  */
const struct mtu_db g_mtus[SIG_MTUS_CNT + 1] = {{"Ethernet or modem",{576,1500},2},
                                                 {"DSL",{1452,1454,1492},3},
                                                 {"GIF",{1240,1280},2},
                                                 {"Generic tunnel or VPN",{1300,1400,1420,1440,1450,1460},6},
                                                 {"IPSec or GRE",{1476},1},
                                                 {"IPIP or SIT",{1480},1},
                                                 {"PPTP",{1490},1},
                                                 {"AX.25 radio modem",{256},1},
                                                 {"SLIP",{552},1},
                                                 {"Google Network",{1470},1},
                                                 {"Vlan",{1496},1},
                                                 {"Ericsson HIS modem",{1656},1},
                                                 {"Jumbo Ethernet",{9000},1},
                                                 {"LoopBack",{3924,16384,16436},3},
                                                 {NULL,{0},0}};  


static unsigned int tprobe_sort_fcnt(struct signatures_db* i_sig);

unsigned int tprobe_sort_fcnt(struct signatures_db* i_sig)
{
         if(i_sig != NULL) {
            unsigned int l_safe = 0;
            struct signatures_db* l_iter;
            for(l_iter=i_sig;l_iter!=NULL;l_iter=l_iter->next) {
                if(l_iter->f_cnt < l_safe) l_iter->f_cnt = l_safe;
                else l_safe = l_iter->f_cnt;                                      
            }    
            return l_safe;   
         }
         return 0;        
}

unsigned char tprobe_sig_parse_file(struct signatures_db* i_sig,char* i_file_name)
{                    
              DWORD l_fsize;
              HANDLE l_hfile;
              int l_a,l_c;
              char* l_file_buf;
              char* l_file_buf_ptr;
              unsigned short l_crc16;
              unsigned char l_return = 0;
              size_t l_cnt = 0,x_cnt = 0;
              
              l_hfile = CreateFile(i_file_name,GENERIC_READ,0,NULL,
                                   OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | 
								   FILE_ATTRIBUTE_READONLY,
                                   NULL);
              if(l_hfile == INVALID_HANDLE_VALUE)  return 0;
              
              l_fsize = GetFileSize(l_hfile,NULL);
              CloseHandle(l_hfile);
              if((l_fsize == INVALID_FILE_SIZE) || (l_fsize == 0)) return 0;            
              
              l_file_buf = (char*)univ_malloc(l_fsize);
              if(l_file_buf == NULL) return 0;
              
              l_file_buf_ptr = l_file_buf;
              
              if(GetPrivateProfileSectionNames(l_file_buf_ptr,
				                               l_fsize,
											   i_file_name) == 0) goto error;
              
              for(;;) {     
                  if(*l_file_buf_ptr == '\0') {
                     i_sig = tprobe_sig_add_section((++l_file_buf_ptr) - l_cnt - 1,
						                             l_cnt,i_sig);
                     if(i_sig == NULL) goto error;
                     else if(*l_file_buf_ptr == '\0') break;
                     l_cnt = 0;               
                  }
                  l_file_buf_ptr++;
                  l_cnt++;      
              }       
                                                                       
              l_cnt = 0;
              memset(l_file_buf,'\0',l_fsize); 
              l_file_buf_ptr = l_file_buf;
              
              for(;;) {                                                           
                 if(i_sig->section == NULL)  goto error;                   /*bad file*/
                 if(GetPrivateProfileSection(i_sig->section,l_file_buf_ptr,
                                             l_fsize,i_file_name) == 0) goto error;
                 x_cnt = 0;
                  
                  for(;;) { 
                      if(*l_file_buf_ptr == '=') {                               /*l_value*/
                         l_crc16 = univ_crc16(l_file_buf_ptr - l_cnt,l_cnt - 1);                         
                         switch(l_crc16) {                                       /*r_value*/
                                 case SIG_IP_IDENT:
									  if(tprobe_set_multivalue_short(l_file_buf_ptr + 1,
										                             &i_sig->ip_ident) == -1) goto error;
                                      x_cnt++;
                                 break;                                                 
                                 case SIG_IP_TTL:
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,
										                          &i_sig->ip_ttl) == -1) goto error;
                                      x_cnt++;
                                 break;          
                                 case SIG_IP_TOS:    
                                      /*i_sig->ip_tos = atoi(l_file_buf_ptr + 1);*/
									 if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->ip_tos) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_IP_DF:
                                      /* i_sig->ip_df = atoi(l_file_buf_ptr + 1); */
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->ip_df) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_IP_MF:
                                     /* i_sig->ip_mf = atoi(l_file_buf_ptr + 1);*/
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->ip_mf) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_FR_OFF:
                                    /*  i_sig->ip_fr_off = atoi(l_file_buf_ptr + 1); */
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->ip_fr_off) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_TCP_WIN_SIZE:
									  if(tprobe_set_multivalue_short(l_file_buf_ptr + 1,
										                             &i_sig->tcp_winsize) == -1) goto error;
									  x_cnt++;
                                 break;
                                 case SIG_TCP_PRIOR:
									  if(tprobe_set_multivalue_short(l_file_buf_ptr + 1,&i_sig->tcp_prior) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_IP_H_LEN:
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->ip_hlen) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_IP_OPTION:
									   if(tprobe_set_multivalue_ptr_uc(l_file_buf_ptr + 1,
										                               &i_sig->ip_option) == -1) goto error;
                                       x_cnt++;
                                 break;
                                 case SIG_TCP_OPTION:
									  if(tprobe_set_multivalue_ptr_uc(l_file_buf_ptr + 1,
										                              &i_sig->tcp_option) == -1) goto error;
                                      x_cnt++;
                                 break;
                                 case SIG_SIG_NAME:
                                      l_c = strlen(l_file_buf_ptr + 1);
                                      i_sig->name = (char*)univ_malloc(l_c + 1);
                                      if(i_sig->name == NULL) break;
                                      memset(i_sig->name,'\0',l_c + 1);
                                      strncpy(i_sig->name,l_file_buf_ptr + 1,l_c);
                                 break;
                                 case SIG_NET_DATA:
                                      i_sig->net_data = (char*)univ_malloc(strlen(l_file_buf_ptr + 1) + 1);
                                      if(i_sig->net_data == NULL) break;
                                      strncpy(i_sig->net_data,l_file_buf_ptr + 1,strlen(l_file_buf_ptr + 1));
                                      x_cnt++;
                                 break;
                                 case SIG_TCP_FLAGS:
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,
										                          &i_sig->tcp_flags) == -1) goto error;
                                      x_cnt++;
                                 break; 
                                 case SIG_TCP_ACK:                               
									  if(tprobe_set_multivalue_ulong(l_file_buf_ptr + 1,&i_sig->tcp_ack) == -1)
										 goto error;
                                      x_cnt++;
                                 break; 
                                 case SIG_TCP_SEQ:
									  if(tprobe_set_multivalue_ulong(l_file_buf_ptr + 1,& i_sig->tcp_seq) == -1) 
										 goto error;
                                      x_cnt++;
                                 break;
							   /*
                                 case SIG_IPIDENT_NOZERO:
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,
										                          &i_sig->ip_ident_nozero) == -1) goto error; 
                                      x_cnt++;
                                 break;
							   */
							   /*
                                 case SIG_TCPACK_NOZERO:
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->tcp_ack_nozero) == -1) goto error;
                                      x_cnt++;
                                 break; 
							  */
                              /*   case SIG_TCPSEQ_NOZERO:
									  if(tprobe_set_multivalue_uc(l_file_buf_ptr + 1,&i_sig->tcp_seq_nozero) == -1) goto error;
                                      x_cnt++;
                                 break; 
						     */
                          }
                          l_cnt = 0; 
                          while(*(++l_file_buf_ptr) != '\0'); 
                          ++l_file_buf_ptr;
                     } else if(*l_file_buf_ptr == '\0') {
                               if(*(l_file_buf_ptr + 1) == '\0') {         
                                  l_cnt = 0;
                                  break;                                     /*bad file*/
                               }
                               l_cnt = 0;                                     /*bad label*/  
                     }
                     l_file_buf_ptr++;  
                     l_cnt++;
                  } 
                  i_sig->f_cnt = x_cnt;
                  if(i_sig->prev == NULL) break;
                  i_sig = i_sig->prev;         
              }
              tprobe_sort_fcnt(i_sig);
              l_return = 1;
              error: univ_free(l_file_buf);
              return l_return;
}

struct signatures_db* tprobe_sig_init(void)
{
       struct signatures_db* l_sig;
       l_sig = (struct signatures_db*)univ_malloc(sizeof(struct signatures_db));
       if(l_sig == NULL) return NULL;
       memset(l_sig,SIG_FILL,sizeof(struct signatures_db));
       return l_sig;
}

struct signatures_db* tprobe_sig_add_section(char* i_section,size_t i_len,struct signatures_db* i_sig)
{
                     if(i_sig->current == 0) {                  
                         i_sig->section = (char*)univ_malloc(i_len + 1);
                         if(i_sig->section == NULL) return NULL;
                         memset(i_sig->section,'\0',i_len + 1);
                         strncpy(i_sig->section,i_section,i_len);
                         i_sig->current++;
                         return i_sig;                
                      } else { 
                          struct signatures_db* l_npsig;
                          l_npsig = (struct signature_db*)univ_malloc(sizeof(struct signatures_db));
                          if(l_npsig == NULL) return NULL;

                          memset(l_npsig,SIG_FILL,sizeof(struct signatures_db));
                          
                          l_npsig->prev = i_sig;  
                          l_npsig->current = i_sig->current + 1;
                          l_npsig->section = (char*)univ_malloc(i_len + 1);
                          
                          if(l_npsig->section == NULL) {
                             univ_free(l_npsig);
                             return NULL;                    
                          }
      
                          memset(l_npsig->section,'\0',i_len + 1);
                          strncpy(l_npsig->section,i_section,i_len); 
                           
                          return (i_sig->next = l_npsig);
                      }     
}

void  tprobe_sig_free(struct signatures_db* i_handle)
{
      if(i_handle != NULL) {
         struct signatures_db* l_iter;       
      
         for(l_iter=i_handle;
             l_iter->next!=NULL;
             l_iter=l_iter->next);                 /*musimy to zrobiæ od ty³u ;]  */      
       
         for(;l_iter!=NULL;l_iter = l_iter->prev) {
             univ_free(l_iter->section);
             univ_free(l_iter->net_data);
             univ_free(l_iter->name);
             univ_free(l_iter->next); 
             l_iter->next = NULL;  
         }   
         univ_free(l_iter);      
      }
}

/*
  differnces between types forced us to define many difrent functions and structures
  thats make basicly the same thing 
  In C++ we could use template but here we cant so we use some ... tricks ;=
*/

int tprobe_set_multivalue_ulong(char* i_buff,struct s_multi_ulong* o_multi_ulong)
{
	int r = -1;
	unsigned long* tmp = NULL;
	char* str_dup = (char*)strdup(i_buff);
	if(str_dup != NULL) {
       char* part = (char*)strtok(str_dup,(const char*)SIG_MULITIVAL_DELIM);
	   o_multi_ulong->amount = 0;
	   o_multi_ulong->ptr.tunion = NULL;
	   while(part != NULL) {
		     if(((o_multi_ulong->amount + 1) * sizeof(unsigned long)) <= o_multi_ulong->amount) goto __jmp__;
			 tmp = (unsigned long*)univ_realloc(o_multi_ulong->ptr.tunion,(o_multi_ulong->amount + 1) * sizeof(unsigned long));
			 if(tmp == NULL) goto __jmp__;
			 o_multi_ulong->ptr.tunion = tmp;
			 o_multi_ulong->ptr.tunion[o_multi_ulong->amount] = (unsigned long)atoi(part);
			 ++o_multi_ulong->amount;
			 part = strtok(NULL,(const char*)SIG_MULITIVAL_DELIM);
	   }
       r = o_multi_ulong->amount + 1;
	   __jmp__: univ_free(str_dup);
	}
	return r;
}

int tprobe_set_multivalue_uc(char* i_buff,struct s_multi_uc* o_multi)
{
	int r = -1;
    unsigned char* tmp = NULL;
	char* str_dup = (char*)strdup(i_buff);
	if(str_dup != NULL) {
	   char* part = (char*)strtok(str_dup,(const char*)SIG_MULITIVAL_DELIM);
	   o_multi->amount = 0;
	   o_multi->ptr.tunion = NULL;
	   while(part != NULL) {
		     if(((o_multi->amount + 1) * sizeof(unsigned char)) <= o_multi->amount) goto __jmp__;
			 tmp = (unsigned char*)univ_realloc(o_multi->ptr.tunion,(o_multi->amount + 1) * sizeof(unsigned char));
			 if(tmp == NULL) goto __jmp__;
			 o_multi->ptr.tunion = tmp;
			 o_multi->ptr.tunion[o_multi->amount] = (unsigned char)atoi(part);
			 ++o_multi->amount;
			 part = strtok(NULL,(const char*)SIG_MULITIVAL_DELIM);
	   }
	    r = o_multi->amount + 1;
	   __jmp__: univ_free(str_dup);
	}
    return r;
}

int tprobe_set_multivalue_short(char* i_buff,struct s_multi_short* o_multi_short)
{
    int r = -1;
	unsigned short* tmp = NULL;
	char* str_dup = (char*)strdup(i_buff);
	if(str_dup != NULL) {
	   char* part = strtok(str_dup,(const char*)SIG_MULITIVAL_DELIM);
	   o_multi_short->amount = 0;
	   o_multi_short->ptr.tunion = NULL;
       while(part != NULL) {
		     if(((o_multi_short->amount + 1) * sizeof(unsigned short)) <= o_multi_short->amount) goto __jmp__;
		     tmp = (unsigned short*)univ_realloc(o_multi_short->ptr.tunion,(o_multi_short->amount + 1) * sizeof(unsigned short));
		     if(tmp == NULL) goto __jmp__;
		     o_multi_short->ptr.tunion = tmp;
		     o_multi_short->ptr.tunion[o_multi_short->amount] = (unsigned short)atoi(part);
		     ++o_multi_short->amount;
		     part = strtok(NULL,(const char*)SIG_MULITIVAL_DELIM);
	   }
	   r = o_multi_short->amount + 1;
	   __jmp__: univ_free(str_dup);
	}
	return r;
}

static size_t tprobe_delims_count(char* i_buff);

static size_t tprobe_delims_count(char* i_buff)
{
	   size_t r = 0;
	   char* tmp = i_buff;
	   while(*tmp++ != '\0') {
		     if(*tmp == SIG_MULITIVAL_DELIM_CHR) ++r;
	   }
	   return r;
}

int tprobe_set_multivalue_ptr_uc(char* i_buff,struct s_multi_ptr_uc* o_multi_ptr_uc)
{
	int r = -1;
	unsigned char** tmp = NULL;
	char* str_dup = (char*)strdup(i_buff);
	if(str_dup != NULL) {
	   size_t nbr = tprobe_delims_count(i_buff);
	   if(nbr + 1 <= nbr) goto __jmp__;
	   tmp = (unsigned char**)univ_multi_alloc(nbr + 1,(SIG_IP_OPTION_LEN * 2) + 1);
	   if(tmp != NULL) {
		  size_t i;
		  size_t len = 0;
          char* part = strtok(str_dup,(const char*)SIG_MULITIVAL_DELIM);
		  o_multi_ptr_uc->amount = 0;
		  while(part != NULL) {
		        o_multi_ptr_uc->ptr.tunion = tmp;
		        len = (size_t)strlen(part);
                for(i=0;i<len;i+=2) {    
                    if((i / 2) == SIG_IP_OPTION_LEN) break;       
                    sscanf(&(part)[i],"%2x",&o_multi_ptr_uc->ptr.tunion[o_multi_ptr_uc->amount][i / 2]);
		        }
		        ++o_multi_ptr_uc->amount;
			    part = strtok(NULL,(const char*)SIG_MULITIVAL_DELIM);
		  }
	   }
	   r = o_multi_ptr_uc->amount + 1;
	   __jmp__: univ_free(str_dup);
	}
	return r;
}


