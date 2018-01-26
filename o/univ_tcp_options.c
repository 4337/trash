#include <windows.h>
#include <stdio.h>
#include "univ_tcp_options.h"

int UTCP_GET_LENGTH_BY_KIND(unsigned char* i_kind_ptr)
{
    if(i_kind_ptr == NULL) return -1;
    switch(*i_kind_ptr) {
           case UTCP_OPT_NOP:   return 1;
           case UTCP_OPT_MPTCP: return 1; 
           case UTCP_OPT_UTO:   return 4;
           case UTCP_OPT_MSS:   return 4;
           case UTCP_OPT_WSOPT: return 3;
           case UTCP_OPT_SACKP: return 2;
           case UTCP_OPT_SACK:  return (int)*(i_kind_ptr + 1);
           case UTCP_OPT_ECHO:  return 6;
           case UTCP_OPT_ECHOR: return 6;
           case UTCP_OPT_TS:    return 10;
           case UTCP_OPT_POCP:  return 2;
           case UTCP_OPT_POSP:  return 3;
           case UTCP_OPT_CC:    return (int)*(i_kind_ptr + 1);  
           case UTCP_OPT_CCNEW: return (int)*(i_kind_ptr + 1);                     /* WARN !! */
           case UTCP_OPT_CCECHO:return (int)*(i_kind_ptr + 1);                     /* WARN !! */
           case UTCP_OPT_ACHR:  return 3;
           case UTCP_OPT_ACHSUM: return (int)*(i_kind_ptr + 1);
           case UTCP_OPT_SKEETE: return (int)*(i_kind_ptr + 1);                     /* WARN !! */
           case UTCP_OPT_BUBBA:  return (int)*(i_kind_ptr + 1);                      /*WARN !! */
           case UTCP_OPT_TCHO:   return 3;
           case UTCP_OPT_MD5SIG: return 18;
           case UTCP_OPT_SCPS:   return (int)*(i_kind_ptr + 1);                        /*WARN !! */
           case UTCP_OPT_SNACK:  return (int)*(i_kind_ptr + 1);                         /*WARN !!*/
           case UTCP_OPT_RB:     return (int)*(i_kind_ptr + 1);                             /*WARN !!*/
           case UTCP_OPT_CE:     return (int)*(i_kind_ptr + 1);                            /*WARN !!*/
           case UTCP_OPT_SNAP:   return (int)*(i_kind_ptr + 1);                          /*WARN!!*/;
           case UTCP_OPT_UNASIG: return (int)*(i_kind_ptr + 1);                       /*WARN!!*/
           case UTCP_OPT_TCPCF:  return (int)*(i_kind_ptr + 1);                       /*WARN */
           case UTCP_OPT_QSR:    return 8;
           case UTCP_OPT_AUTH:   return (int)*(i_kind_ptr + 1);                     /*WARN !!*/
           default: return -1;    
    }     
}

unsigned char* u_tcpoptions_get_option(unsigned char* i_options,unsigned int i_opt,int i_opt_len)
{
    if(i_options != NULL) {
       while(((i_opt_len > 0) || (i_opt_len == 0)) && (*i_options != UTCP_OPT_EOOL)) {
             if(*i_options == (unsigned char)i_opt)
                return i_options;
             switch(*i_options) {
                    case UTCP_OPT_NOP: 
                         ++i_options;
                         --i_opt_len;
                    break;
                    case UTCP_OPT_MSS:
                         i_options += 4;
                         i_opt_len -= 4;
                    break;
                    case UTCP_OPT_WSOPT:
                         i_options += 3;
                         i_opt_len -= 3;
                    break;
                    case UTCP_OPT_SACKP:
                         i_options += 2;
                         i_opt_len -= 2;
                    break;
                    case UTCP_OPT_SACK:
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_ECHO:
                         i_options += 6;
                         i_opt_len -= 6;
                    break;
                    case UTCP_OPT_ECHOR:
                         i_options += 6;
                         i_opt_len -= 6;
                    break;
                    case UTCP_OPT_TS:
                         i_options += 10;
                         i_opt_len -= 10;
                    break;
                    case UTCP_OPT_POCP:
                         i_options += 2;
                         i_opt_len -= 2;
                    break;
                    case UTCP_OPT_POSP:
                         i_options += 3;
                         i_opt_len -= 3;
                    break;
                    case UTCP_OPT_CC:                          /* WARN !! */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_CCNEW:                       /* WARN !! */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1; 
                    break;
                    case UTCP_OPT_CCECHO:                      /* WARN !! */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_ACHR:
                         i_options += 3;
                         i_opt_len -= 3;
                    break;
                    case UTCP_OPT_ACHSUM:
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_SKEETE:                      /* WARN !! */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_BUBBA:                       /*WARN !! */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_TCHO:
                         i_options += 3;
                         i_opt_len -= 3;
                    break;
                    case UTCP_OPT_MD5SIG:
                         i_options += 18;
                         i_opt_len -= 18;
                    break;
                    case UTCP_OPT_SCPS:                           /*WARN !! */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_SNACK:                          /*WARN !!*/
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_RB:                             /*WARN !!*/
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_CE:                             /*WARN !!*/
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_SNAP:                           /*WARN!!*/
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_UNASIG:                        /*WARN!!*/
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_TCPCF:                        /*WARN */
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_QSR:
                         i_options += 8;
                         i_opt_len -= 8;
                    break;
                    case UTCP_OPT_UTO:
                         i_options += 4;
                         i_opt_len -= 4;
                    break;
                    case UTCP_OPT_AUTH:                       /*WARN !!*/
                         i_options += *i_options + 1;
                         i_opt_len -= *i_options + 1;
                    break;
                    case UTCP_OPT_MPTCP:
                         ++i_options;
                         --i_opt_len;
                    break;
                    default:         /* Unknow option ! What now ? */
                           #if UTCP_OPT_QUIT_ON_ERROR == 1
                            return NULL;
                           #else
                            ++i_options;
                            --i_opt_len;
                           #endif
                    break;
             }               
       }         
    }
    return NULL;
}

size_t u_tcpoptions_raw_size(struct u_tcp_option* i_options,unsigned int i_opt_cnt)
{
     if((i_options != NULL) && (i_opt_cnt != 0)) { 
         unsigned int l_a,l_opt_size = 0;
         for(l_a=0;l_a<i_opt_cnt;l_a++) l_opt_size += i_options[l_a]._length;
         return l_opt_size;
     }
     return 0; 
}

size_t u_tcpoptions_align_size(struct u_tcp_option* i_options,unsigned int i_opt_cnt)
{
       if((i_options != NULL) && (i_opt_cnt != 0)) {
          unsigned int l_opt_size = u_tcpoptions_raw_size(i_options,i_opt_cnt);
          while((l_opt_size % 4) != 0) ++l_opt_size;
          return l_opt_size;               
       }
       return  0;
}


void u_tcpoptions_set_option(unsigned char* i_place_in_pkt,struct u_tcp_option* i_options,unsigned int i_opt_cnt,size_t i_aligned_size)
{
     int l_a,l_b,l_c = 0;
     
     for(l_a=0;l_a<i_opt_cnt;l_a++) {
         if(i_options[l_a]._kind == UTCP_OPT_EOOL) continue;  /* skip eol for aligment */
         for(l_b=0;l_b<i_options[l_a]._length ;l_b++) {
             if(l_b == 0) i_place_in_pkt[l_c] = i_options[l_a]._kind;
             if(l_b > sizeof(unsigned int)) i_place_in_pkt[++l_c] = 0x00;
             else i_place_in_pkt[++l_c] = *((unsigned char*)&i_options[l_a]._val + l_b);                           
         }    
     }

     while(l_c < (i_aligned_size - 1)) {
           i_place_in_pkt[l_c] = UTCP_OPT_NOP;
           ++l_c;
     }
     i_place_in_pkt[l_c++] = UTCP_OPT_EOOL;
}
