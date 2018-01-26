#include <stdio.h>
#include "tprobe_net_ports.h"

const char* tprobe_port_status(struct tcp_packet* i_packet)
{
             if((i_packet->syn == 1) && (i_packet->ack == 1)) return "open";   
             else if((i_packet->rst) && (i_packet->ack_number == 0)) return "unfiltered";
             else if((i_packet->rst == 1) && (i_packet->ack == 1)) return "closed"; 
             else if(i_packet->ack == 1) return "open";
             return "unknow";
}
