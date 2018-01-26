#ifndef XXX_WPCAP_H
#define XXX_WPCAP_H

/*
....
 IN:i_sip=source ip address
    i_timeout=read timeout in mls 
    i_error = pointer to out buffer that contains error information
    i_esize = error buf size
 RETURN: NULL=ERROR || Pointer=NO_ERROR
*/
struct pcap* wpcap_init(char* i_sip,int i_timeout,char* i_error,size_t i_esize,const unsigned char i_auto);

/*... raw recv */

void wpcap_free(struct pcap* i_desc);

#define WPCAP_DEV_LEN 256

/* Geting correct device name 
   IN: i_ip = ip address 
       i_out_out_iface = pointer to char that contains
                         correct device name
   RETURN: 0 = NO_ERROR ||
          -1 = ERROR ||
          Needed outbuf size if is to small
*/
int wpcap_get_dev_name(char* i_ip,char* i_out_name,size_t i_outlen,const unsigned char i_auto);

/*
  ,,,
*/
size_t wpcap_set_filter(pcap_t* i_desc,char* i_str,int i_opt,bpf_u_int32 i_mask);

#endif
