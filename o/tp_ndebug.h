#ifndef TP_NDEBUG_H
#define TP_NDEBUG_H


#define _DUMP_ETH(packet)                                                        \
do                                                                            \
{                                                                             \
       printf(                                                                   \
              "     + ETH packet\r\n"                                                   \
              "       Type: 0x%04x  \r\n"                                                 \
              "       SrcMac: %02x-%02x-%02x-%02x-%02x-%02x\r\n"                    \
              "       DstMac: %02x-%02x-%02x-%02x-%02x-%02x\r\n\r\n",                \
              ntohs(((struct tcpip4_packet*)packet)->eth.type),                   \
              ((struct tcpip4_packet*)packet)->eth.s_eth[0],                     \
              ((struct tcpip4_packet*)packet)->eth.s_eth[1],                     \
              ((struct tcpip4_packet*)packet)->eth.s_eth[2],                     \
              ((struct tcpip4_packet*)packet)->eth.s_eth[3],                     \
              ((struct tcpip4_packet*)packet)->eth.s_eth[4],                     \
              ((struct tcpip4_packet*)packet)->eth.s_eth[5],                    \
              \
              ((struct tcpip4_packet*)packet)->eth.d_eth[0],                     \
              ((struct tcpip4_packet*)packet)->eth.d_eth[1],                     \
              ((struct tcpip4_packet*)packet)->eth.d_eth[2],                     \
              ((struct tcpip4_packet*)packet)->eth.d_eth[3],                     \
              ((struct tcpip4_packet*)packet)->eth.d_eth[4],                     \
              ((struct tcpip4_packet*)packet)->eth.d_eth[5]                      \
             );                                                                  \
}while(0)                                                                  \

#define _DUMP_IPV4(packet)\
do\
{\
    printf(\
           "     + IPv4 packet\r\n"\
           "       HdrLen: %d\r\n" \
           "       Ver:    %d\r\n" \
           "       Tos:    %d\r\n" \
           "       TotLen: %d\r\n" \
           "       IDent:  %d\r\n" \
           "       Frag:   %d\r\n" \
           "       Mf:     %d\r\n" \
           "       Df:     %d\r\n" \
           "       Resv:   %d\r\n" \
           "       Frag1:  %d\r\n" \
           "       Ttl:    %d\r\n" \
           "       Proto:  %d\r\n" \
           "       ChSum:  %d hex(0x%04x) RORD(0x%04x)\r\n"\
           "       SrcIP:  %d\r\n"\
           "       DstIP:  %d\r\n\r\n",\
           ((struct tcpip4_packet*)packet)->ip.h_len,\
           ((struct tcpip4_packet*)packet)->ip.version,\
           ((struct tcpip4_packet*)packet)->ip.tos,\
           ntohs(((struct tcpip4_packet*)packet)->ip.total_len),\
           ntohs(((struct tcpip4_packet*)packet)->ip.ident),\
           ((struct tcpip4_packet*)packet)->ip.frag,\
           ((struct tcpip4_packet*)packet)->ip.mf,\
           ((struct tcpip4_packet*)packet)->ip.df,\
           ((struct tcpip4_packet*)packet)->ip.reserved,\
           ((struct tcpip4_packet*)packet)->ip.frag1,\
           ((struct tcpip4_packet*)packet)->ip.ttl,\
           ((struct tcpip4_packet*)packet)->ip.proto,\
           ((struct tcpip4_packet*)packet)->ip.ch_sum,\
           ((struct tcpip4_packet*)packet)->ip.ch_sum,\
           ntohs( ((struct tcpip4_packet*)packet)->ip.ch_sum),\
           ((struct tcpip4_packet*)packet)->ip.source_ip,\
           ((struct tcpip4_packet*)packet)->ip.dest_ip\
          );\
}while(0)
     

/*dokoncz*/
#define _RAW_CHSUM_DUMP(packet)\
do\
{\
}while(0)

/* dokoncz*/
#define _DUMP_TCP(packet)\
do\
{\
       printf("     + TCP packet\r\n"\
              "       SrcPort: %d\r\n"\
              "       DstPort: %d\r\n"\
              "       SeqNum:  %d\r\n"\
              "       AckNum:  %d\r\n"\
              "       Ns:      %d\r\n"\
              "       Resv1:   %d\r\n"\
              "       HdrLen:  %d\r\n"\
              "       Flags:  \r\n"\
              "        + Fin: %d\r\n"\
              "        + Syn: %d\r\n"\
              "        + Rst: %d\r\n"\
              "        + Psh: %d\r\n"\
              "        + Ack: %d\r\n"\
              "        + Urg: %d\r\n"\
              "        + Ecn: %d\r\n"\
              "        + Cwr: %d\r\n"\
              "       WinSize: %d\r\n"\
              "       ChSum: %d hex(0x%04x) RORD(0x%04x)\r\n"\
              "       Prioity: %d\r\n\r\n",\
              ntohs(((struct tcpip4_packet*)packet)->tcp.src_port),\
              ntohs(((struct tcpip4_packet*)packet)->tcp.dst_port),\
              ntohl(((struct tcpip4_packet*)packet)->tcp.seq_number),\
              ntohl(((struct tcpip4_packet*)packet)->tcp.ack_number),\
              ((struct tcpip4_packet*)packet)->tcp.ns,\
              ((struct tcpip4_packet*)packet)->tcp.reserved1,\
              ((struct tcpip4_packet*)packet)->tcp.hdr_len,\
              ((struct tcpip4_packet*)packet)->tcp.fin,\
              ((struct tcpip4_packet*)packet)->tcp.syn,\
              ((struct tcpip4_packet*)packet)->tcp.rst,\
              ((struct tcpip4_packet*)packet)->tcp.psh,\
              ((struct tcpip4_packet*)packet)->tcp.ack,\
              ((struct tcpip4_packet*)packet)->tcp.urg,\
              ((struct tcpip4_packet*)packet)->tcp.ecn,\
              ((struct tcpip4_packet*)packet)->tcp.cwr,\
              ntohs(((struct tcpip4_packet*)packet)->tcp.win_size),\
              ((struct tcpip4_packet*)packet)->tcp.ch_sum,\
              ((struct tcpip4_packet*)packet)->tcp.ch_sum,\
              ntohs(((struct tcpip4_packet*)packet)->tcp.ch_sum),\
              ntohs(((struct tcpip4_packet*)packet)->tcp.prior)\
              );\
}while(0)
                               

 #define _DUMP_PKT(packet)\
 do\
     {\
        printf(" [!!]. Debug \r\n\r\n");\
        _DUMP_ETH(packet);\
        _DUMP_IPV4(packet);\
        _DUMP_TCP(packet); \
        _RAW_CHSUM_DUMP(packet);\
 }while(0)

#endif
