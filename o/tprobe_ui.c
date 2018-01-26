#include <stdio.h>
#include <windows.h>
#include <getopt.h>
#include "tprobe_ui.h"
#include "tprobe_runtime.h"

void tprobe_help(void)
{
     TPROBE_OUT("Use tprobe.exe options\r\n"
                "eg: tprobe.exe -A -p -S 53 -D 80 -T 192.168.0.1\r\n"
                "    tprobe.exe -s -S 53 -D 80 -T 192.168.0.1\r\n"
                "    tprone.exe -a -S 66 -D 80 -T 192.168.0.201\r\n"
                "Options:\r\n"
                "-i : interactive (select interface)\r\n"
                "-s : send syn\r\n"
                "-A : send syn/ack\r\n"
                "-a : send ack\r\n"
                "-m : send mallformed\r\n"
                "-S : source port\r\n"
                "-D : dest port\r\n"
                "-T : type target (hostname or Ipv4)\r\n"
                "-h : help me\r\n"
                "-f : append data from file\r\n"
                "-c : append data from command line\r\n"
                "-p : compare os fingerprints\r\n"
                "-r : traceroute\r\n"
                "-u : get remote host uptime (noisly option)\r\n"
                "-l : log packets to file \r\n"
				"-e : edit sigantures\r\n");
     
}

#define TPROBE_UI_ERR(var,val)                                          \
do{                                                                     \
           if((var) != (val)) {                                         \
              TPROBE_OUT(" [!] Duplicate tcp packet option !\r\n\r\n"); \
              goto __free_error__;                                      \
           }                                                            \
}while(0)                                                    

struct ui_options* tprobe_get_cmdline(int i_argc,char** i_argv)
{
                   int l_l;
                   HANDLE l_f;
                   DWORD  l_dsize,l_bytes;
                   struct ui_options* l_opt = (struct ui_options*)univ_malloc(sizeof(struct ui_options));
                   
                   if(l_opt == NULL) return NULL;
                   memset(l_opt,0,sizeof(struct ui_options));
                   l_opt->time_out = UI_DEF_TIMEOUT;
       
                   while((l_l = getopt(i_argc,i_argv,"lseuAamS:D:T:hrf:c:l:pi")) != -1) {
                          switch(l_l) {
                                 case 's':      /* syn send */  
                                     TPROBE_UI_ERR(l_opt->packet_set,0);
                                     l_opt->packet_opt = NET_SEND_SYN;
                                     l_opt->packet_set = 1;                                
                                 break; 
                                 case 'l':
                                      TPROBE_UI_ERR(l_opt->log,0);
                                      l_opt->log = 1;           
                                 break;
                                 case 'u':
                                      TPROBE_UI_ERR(l_opt->uptime,0);
                                      l_opt->uptime = 1;              
                                 break;
                                 case 'A':
                                      TPROBE_UI_ERR(l_opt->packet_set,0);
                                      l_opt->packet_opt = NET_SEND_SYNACK;
                                      l_opt->packet_set = 1;         
                                 break;        
                                 case 'a':
                                      TPROBE_UI_ERR(l_opt->packet_set,0);
                                      l_opt->packet_opt = NET_SEND_ACK;
                                      l_opt->packet_set = 1;
                                 break;
                                 case 'm':
                                      TPROBE_UI_ERR(l_opt->packet_set,0);
                                      l_opt->packet_opt = NET_SEND_MALLFORMED;
                                      l_opt->packet_set = 1;
                                 break;
                                 case 'p':  /* poffffff */
                                      TPROBE_UI_ERR(l_opt->pof,0);
                                      l_opt->pof = 1;
                                 break;
                                 case 'f': /* data form file */
                                      TPROBE_UI_ERR(l_opt->data_size,0);
                                      l_f = CreateFile(optarg,GENERIC_READ,0,NULL,
                                                       OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,
                                                       NULL);
                                      if(l_f == INVALID_HANDLE_VALUE) {
                                         TPROBE_OUT(" [!].Bad %%f%% Argument %s dosent exists ?!\r\r\n",optarg);
                                         goto __free_error__;      
                                      }
                                      l_dsize = GetFileSize(l_f,NULL);
                                      if((l_dsize == INVALID_FILE_SIZE) || (l_dsize == 0)) {
                                          CloseHandle(l_f);
                                          TPROBE_OUT(" [!]. %%f%% Invalid file size %s!\r\n\r\n",optarg);
                                          goto __free_error__;        
                                      }
                                      l_opt->data = (char*)univ_malloc(l_dsize + 1);
                                      if(l_opt->data == NULL) {
                                         CloseHandle(l_f);
                                         TPROBE_OUT(" [!]. %%f%% I can't read data form file [alloc_error] :(\r\n\r\n");
                                         goto __free_error__;
                                      } else if(!ReadFile(l_f,l_opt->data,l_dsize,&l_bytes,0)) {
                                              CloseHandle(l_f);
                                              TPROBE_OUT(" [!]. %%f%% I can't read data from file [readFile error] :(\r\n\r\n");
                                              goto __free_error__;     
                                      }
                                      l_opt->data_size = l_bytes;
                                      CloseHandle(l_f);
                                 break;
                                 case 'c':
                                      TPROBE_UI_ERR(l_opt->data_size,0);
                                      
                                      l_dsize = strlen(optarg);
                                      l_opt->data = (char*)univ_malloc(l_dsize + 1);
                                      
                                      if(l_opt->data == NULL) {
                                         TPROBE_OUT(" [!]. %%c%% I can't copy data from cl [alloc error] :(\r\n\r\n");
                                         goto __free_error__;               
                                      }
                                      strncpy(l_opt->data,optarg,l_dsize);
                                      l_opt->data_size = l_dsize;
                                 break;
                                 case 'T':
                                      TPROBE_UI_ERR(l_opt->target_set,0);
                                      strncpy((char*)l_opt->target,optarg,15);
                                      l_opt->target_set = 1;
                                 break;
                                 case 'S':
                                      TPROBE_UI_ERR(l_opt->s_port,0);
                                      l_opt->s_port = (unsigned short)atoi(optarg);                
                                 break;
                                 case 'D':
                                      TPROBE_UI_ERR(l_opt->d_port,0);
                                      l_opt->d_port = (unsigned short)atoi(optarg);                
                                 break;
                                 case 'h': /*Ha jak Lucjan */
                                     tprobe_help();
                                     goto __free_error__;
                                 break;
                                 case 'r': /*Tracerouteee */
                                      TPROBE_UI_ERR(l_opt->tracert,0);              
                                      l_opt->tracert = 1;
                                 break;    
                                 case 'i':
                                      TPROBE_UI_ERR(l_opt->interactive,0);
                                      l_opt->interactive = 1;
                                 break;  
								 case 'e':
									   system("notepad.exe "TPROBE_SIG_FILENAME);
									   goto __free_error__;
								 break;
                                 case '?': goto __free_error__;
                                 default : goto __free_error__;
                          }     
                   }
                   return l_opt;
                   
                   __free_error__: univ_free(l_opt);
                   
                   return NULL;
}
