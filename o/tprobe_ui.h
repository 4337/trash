#ifndef TPROBE_UI_H
#define TPROBE_UI_H

#include "univ_mem.h"
#include "univ_macros.h"

#define TPROBE_VERSION 1.1.0

#define TPROBE_BANNER "\r\n"                                                                \
                      "  ******* tprobe  v 1.1.0 by echo && tao *******  \r\n"              \
                      "            vulnerable.zappa@gmail.com\r\n"                          \
                      "             tao.cee.tung@gmail.com\r\n"                             \
                      "                ----------------\r\n"                                \
                      "            use tprobe.exe -h for help\r\n"                          \
                      "       ***************************************   \r\n\r\n"

#define UI_DEF_TIMEOUT 7000

#define TPROBE_SHOW_BANNER() printf(TPROBE_BANNER)
#define TPROBE_OUT(in...) printf(in)

#define NET_SEND_SYN 0
#define NET_SEND_SYNACK 1
#define NET_SEND_ACK 2
#define NET_SEND_MALLFORMED 3

struct ui_options
{
       size_t packet_size;
       size_t data_size;
       size_t time_out;
       char*  data;
       unsigned short s_port;
       unsigned short d_port;
       char target[15 + 1];
       unsigned char target_set;
       unsigned char pof;
       unsigned char packet_opt;  /* offset syn/ack syn ack mallformed */
       unsigned char packet_set;  /* duplicate ?*/ 
       unsigned char tracert;   
       unsigned char uptime;
       unsigned char log;
	   char log_path[MAX_PATH + 1];  /* optional for -l (dir for dump file)*/
       unsigned char interactive;
};

struct ui_options* tprobe_get_cmdline(int i_argc,char** i_argv);

/*inline*/
static univ_inline
void tprobe_ui_free(struct ui_options* i_opt)
{
     univ_free(i_opt);
}

void tprobe_help(void);



#endif


