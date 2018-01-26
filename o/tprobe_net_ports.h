#ifndef TPROBE_NET_PORTS_H
#define TPROBE_NET_PORTS_H

#include "univ_packet.h"

/*
  Return port status (open,closed,unknow,...)
  IN: i_packet = structure tcp_packet ;>
  RETURN: status
*/
const char* tprobe_port_status(struct tcp_packet* i_packet);

#endif
