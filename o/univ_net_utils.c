#include <winsock2.h>
#include <windows.h>
#include "univ_net_utils.h"

unsigned char univ_is_private_ipv4(struct in_addr* i_ip)
{
         if((i_ip->S_un.S_un_b.s_b1 == 192) &&
            (i_ip->S_un.S_un_b.s_b2 == 168)) return 1;
            
         if((i_ip->S_un.S_un_b.s_b1 == 172) &&
            (i_ip->S_un.S_un_b.s_b2 >= 16)  &&
            (i_ip->S_un.S_un_b.s_b2 <= 31)) return 1;
            
         if(i_ip->S_un.S_un_b.s_b1 == 10) return 1;
         if(i_ip->S_un.S_un_b.s_b1 == 127) return 1;
         
         if((i_ip->S_un.S_un_b.s_b1 == 169) &&
            (i_ip->S_un.S_un_b.s_b2 == 254)) return 1;
            
         return 0;
}

char univ_get_selfip(char* i_out,size_t i_iplen)
{ 
     struct in_addr l_ip;
     struct hostent* l_hostname;
     char l_host[NET_HOST_NAME + 1];
     
     if(gethostname(l_host,NET_HOST_NAME) != 0)
        return -1;
        
     l_hostname = gethostbyname(l_host);
     if(l_hostname == NULL) return -1;
     
     l_ip.s_addr = *(unsigned long*)l_hostname->h_addr_list[0];  
     strncpy(i_out,inet_ntoa(l_ip),i_iplen);

     return 0;
}

static PIP_ADAPTER_INFO univ_get_correct_iface_info(PIP_ADAPTER_INFO i_info);

PIP_ADAPTER_INFO univ_get_correct_iface_info(PIP_ADAPTER_INFO i_info)
{
                 CHAR l_sip[NET_IP_ADDR + 1];
                 PIP_ADDR_STRING l_ipaddr;
                 PIP_ADAPTER_INFO  l_ptr = i_info;
                 
                 univ_get_selfip(l_sip,NET_IP_ADDR);
                 do {
                       l_ipaddr = &l_ptr->IpAddressList;
                       do {    
                              if(strcmp(l_sip,l_ipaddr->IpAddress.String) == 0) return l_ptr;
                              l_ipaddr = l_ipaddr->Next;  
                       } while(l_ipaddr != NULL);
                       l_ptr = l_ptr->Next;
                 } while(l_ptr != NULL);
                 return NULL;
}

PIP_ADAPTER_INFO univ_iface_info_init(void)
{  
      PIP_ADAPTER_INFO l_out;
      CHAR l_sip[NET_IP_ADDR + 1];
      unsigned long l_adp_size = sizeof (IP_ADAPTER_INFO);
      
      l_out = (PIP_ADAPTER_INFO)univ_malloc(sizeof(IP_ADAPTER_INFO));
      if(l_out == NULL) return NULL;
      
      univ_get_selfip(l_sip,NET_IP_ADDR);
      
      for(;;) {
         if(GetAdaptersInfo(l_out,&l_adp_size) == ERROR_BUFFER_OVERFLOW) {                                 
            PIP_ADAPTER_INFO l_temp = (PIP_ADAPTER_INFO)univ_realloc(l_out,l_adp_size);
            if(l_temp == NULL) return NULL;
            l_out = l_temp;
            continue;                                       
         }     
         break;    
      }
      return univ_get_correct_iface_info(l_out);
}


