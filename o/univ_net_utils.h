#ifndef UNIV_NET_UTILS_H
#define UNIV_NET_UTILS

#include "univ_mem.h"
#include "univ_macros.h"

#define NET_HOST_NAME 256
#define NET_IP_ADDR   16  /* 45 TODO Ipv6 support */

#include <iphlpapi.h>
/* 
   Geting own ip address 
   Return: 0 = NO_ERROR 
           -1 = ERROR
   In: i_out = Pointer to char table that contains ip;
       i_iplen = size of char table (must got space for null byte!)
                 if i_iplen = 16 then sizeof table must be 16 + 1
*/
char univ_get_selfip(char* i_out,size_t i_iplen);

/*
   Geting own mac address 
   IN: void
   Return: NULL = ERROR
           Pointer = no_error
*/
PIP_ADAPTER_INFO  univ_iface_info_init(void);


/* Free adapter info */
/*inline*/
static univ_inline
void univ_iface_info_free(PIP_ADAPTER_INFO i_info)
{
     univ_free(i_info);
}

/*
   Chck if ip is provate
   IN: i_ip = pointer to ipv4 string
   OUT: 0=false || 1=true
*/
unsigned char univ_is_private_ipv4(struct in_addr* i_ip);

#endif
