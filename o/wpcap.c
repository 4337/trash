#define HAVE_REMOTE
#define WPCAP

#include <stdio.h>
#include <windows.h>
#include <pcap.h>
#include "univ_mem.h"
#include "wpcap.h"

struct pcap* wpcap_init(char* i_sip,int i_timeout,char* i_error,size_t i_esize,const unsigned char i_auto)
{
        int l_ret;
        char* l_dev;
        pcap_t* l_dev_desc;
        int l_len = WPCAP_DEV_LEN;

        l_dev = (char*)univ_malloc(l_len + 1);
        if(l_dev == NULL) return NULL;
        
        for(;;) {          /* device name */
           l_ret =  wpcap_get_dev_name(i_sip,l_dev,l_len,i_auto);
           if(l_ret != 0) {
              if(l_ret > l_len) { 
                 char* l_tmp = (char*)univ_realloc(l_dev,l_ret + 1);
                 if(l_tmp == NULL) {
                    univ_free(l_dev);
                    return NULL;         
                 } 
                 l_dev = l_tmp;
                 l_len = l_ret;
                 continue;        
              } else if(l_ret == -1) {
                        univ_free(l_dev);
                        return NULL;     
              }    
           }   
           break;
        }
        if((l_dev_desc = pcap_open(l_dev,65536, 
                                   PCAP_OPENFLAG_PROMISCUOUS,
                                   i_timeout,NULL,i_error)) == NULL) {
           univ_free(l_dev);
           return NULL;
        }
        univ_free(l_dev);
        return l_dev_desc;       
}

void wpcap_free(struct pcap* i_desc)
{
     if(i_desc != NULL) pcap_close(i_desc);
}

size_t wpcap_set_filter(pcap_t* i_desc,char* i_str,int i_opt,bpf_u_int32 i_mask)
{ 
       struct bpf_program l_bpf;
       if(pcap_compile(i_desc,&l_bpf,i_str,i_opt,i_mask) < 0) return 0;
       else if(pcap_setfilter(i_desc,&l_bpf) < 0) return 0;
       return 1;  
}

/*
   Get interface by indeks 
   IN: i_pacp = pcap_if pointer from pcap_findalldevs_ex
       i_out  = pointer to outup array (stored dev name)
       i_index = index of device
       i_len   = size of output (i_out) array
   OUT: -1 = error | 0 = ok
*/
static int wpcap_interactive_get_interface(pcap_if_t* i_pcap,char* i_out,unsigned int i_index,unsigned int i_len);

int wpcap_interactive_get_interface(pcap_if_t* i_pcap,char* i_out,unsigned int i_index,unsigned int i_len)
{
     register unsigned int i = 0;
     pcap_if_t* p = i_pcap;
     do {
       if(i == i_index) {  
          strncpy(i_out,p->name,i_len);
          return 0;
       }
       p = p->next;
       ++i;
     } while(p != NULL);
     return -1;
}

int wpcap_get_dev_name(char* i_ip,char* i_out_name,size_t i_outlen,const unsigned char i_auto)
{
     size_t l_nlen;
     pcap_addr_t* l_addr;
     pcap_if_t* l_devices,*l_dev;     
     char l_error[PCAP_ERRBUF_SIZE + 1];
     struct in_addr l_ip;
     unsigned int i = 0;             /* addon 1.0,9 */
     int ret        = -1;            /* ++ 1.0.9 */
     
     if(pcap_findalldevs_ex("rpcap://",NULL,&l_devices,l_error) == -1)
        return -1;
                                                                   
     l_dev = l_devices;

     do {       
         l_nlen = strlen(l_dev->name);        
         if(i_auto > 0) {                  /* dirty addon 1.0.9 */   
            printf(" (%u). Iface: %s\r\n",i,l_dev->name);
         }
         for(l_addr=l_dev->addresses;
             l_addr!=NULL;
             l_addr=l_addr->next) {
             char* l_ascii_ip = NULL; 
             l_ip.s_addr = ((struct sockaddr_in*)l_addr->addr)->sin_addr.s_addr;
             l_ascii_ip =  inet_ntoa(l_ip);
             if(i_auto > 0) {                                                          /* dirty addon 1.0.9 */
                printf("      [+]. ip: %s\r\n",l_ascii_ip);        
             } else if(!strcmp(i_ip,l_ascii_ip)) {
               if(l_nlen > i_outlen) goto __buf2small__;                            
               strncpy(i_out_name,l_dev->name,i_outlen);
               pcap_freealldevs(l_devices); 
               return 0;  
            }   
         }
         l_dev = l_dev->next;
         ++i;
     } while(l_dev != NULL);
     if(i_auto > 0) {                                                                   /* ++ 1.0.9 */
       printf(" [*]. Select interface number from 0 to %u\r\n",i - 1);       
       scanf("%u",&i);
       ret = wpcap_interactive_get_interface(l_devices,i_out_name,i,i_outlen);
    } 
    pcap_freealldevs(l_devices);
    return ret;
      
    __buf2small__: pcap_freealldevs(l_devices); 
    return l_nlen;   
}
