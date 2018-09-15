#include <windows.h>
#include <stdio.h>
#define  R3_MAX_LOG_SIZE 4096
#define  R3_CONTINUE 1
#define  R3_EXIT 0

/* 
   01.12.2010 echo

TO DO:
Miêdzy innymi 
Ustawienie flagi w wyj±tku na kontynuowalny w razie potrzeby
*/

void   r3_parseException(EXCEPTION_RECORD ex,char* IO);
void   r3_parseException(EXCEPTION_RECORD ex,char* IO)              //Nie ma kontextu bo nie oto chodzi
{
                         sprintf(IO,"[#]. _exception_ (0x%x)\n"
                                    "[+]. Exception Flags: 0x%x\n"
                                    "[+]. Exception Address: 0x%x\n"
                                    "[+]. Exception_Info.rwFlag:  0x%x\n"
                                    "[+]. Exception_Info.EvilVaddr: 0x%x\n"
                                    "[+]. Exception_Info.NtSatus: 0x%x\n",
                                    ex.ExceptionCode,
                                    ex.ExceptionFlags,
                                    ex.ExceptionAddress,
                                    (ex.NumberParameters == 1) ? ex.ExceptionInformation[0] : 0x00,
                                    (ex.NumberParameters == 2) ? ex.ExceptionInformation[1] : 0x00,
                                    (ex.NumberParameters == 3) ? ex.ExceptionInformation[2] : 0x00);
}

size_t r3_onTrueException(size_t interact,size_t deep);
size_t r3_onTrueException(size_t interact,size_t deep)
{
                        if(interact > 0)
                        {
                           fputs("[!!]. Do you want kill debuged process [y/n] ?\r\n",stdout);         
                           char choose = getc(stdin); 
                           if(tolower(choose) == 'y')
                           {
                              fputs("[!!]. Process is death\r\n",stdout);
                              FatalExit(666);
                              return R3_EXIT;       
                           }         
                        }    
                        else if(deep > 0)               //deep > 0 kill everything
                        {
                           fputs("[!!]. Process is death\r\n",stdout);
                           FatalExit(666);
                           return R3_EXIT;        
                        } 
                        return R3_CONTINUE;
}

void r3_myExit(const char* msg,size_t code);
void r3_myExit(const char* msg,size_t code)
{
               fputs(msg,stdout);
               exit(code);    
}

int main(int argc,char* argv[])
{
         // -pid int -i -d 0||1
         if(argc < 3) r3_myExit("[!].Exception monitor \r\n"
                                "[!].Use:              \r\n"
                                "[!].r3.exe -p (process id) [optional] -i [optional] -d (0 or 1) [optional] -l log_file_path\r\n"
                                "[!] where:\r\n"
                                "[!] -p xxx   <- process id\r\n"
                                "[!] -i       <- interactive (When an exception occurs, \r\n"
                                "                             this option will allow you to decide\r\n"
                                "                             whether the application is complete or\r\n"
                                "                             continue their action)\r\n"
                                "                             r3 kill application by default when hard execption occurs\r\n"
                                "[!] -d 0||1  <- This option allow you to choose the level(0 or 1)\r\n"
                                "                of response to an exception Level 0 stops the application\r\n"
                                "                for exception like Access Violation etc. Level 1 stops the application\r\n"
                                "                always when the exception occurs\r\n"
                                "[!] -l       <- This option allow you to define log file path :)\r\n",-1);
         int          r3Options;
         extern char* optarg; 
         char  r3LogPath[MAX_PATH + 2] = {'\0'};
         size_t       r3Pid,r3Deep = 0,r3Logger = 0,r3Interactive = 0; 
         while((r3Options = getopt(argc,argv,"p:d:l:i")) != -1)
         {
                switch(r3Options)
                {
                       case 'p':
                             r3Pid = abs(atoi(optarg));
                             if(r3Pid == 0) r3_myExit("Bad -p option !\r\n",-1);
                       break;
                       case 'i':
                             r3Interactive = 1;
                       break;
                       case 'd':
                             if(r3Interactive > 0) r3_myExit("Invalid options ! You can not use -d with -i !\r\n",-1);
                             r3Deep = (abs(atoi(optarg)) >= 1) ? 1 : 0;
                       break;     
                       case 'l': 
                             if(optarg == NULL) r3_myExit("Bad -l option !\r\n",-1);
                             memcpy(r3LogPath,optarg,MAX_PATH);
                             r3Logger = 1;
                       break;                
                }           
         }      
         printf("[+]. Infos:\r\n"
                "[!]. Killing deep : %d \r\n"
                "[!]. Interactive  : %d \r\n"
                "[!]. Log file     : %s \r\n"
                "[!]. PID          : %d \r\n"
                "[!]. Logger       : %s \r\n",
                 r3Deep,
                 r3Interactive,(*r3LogPath != 0) ? r3LogPath : "No logging",
                 r3Pid,
                 (r3Logger == 0) ? "No" : "Yes");    
         HANDLE r3LogHandler = NULL;
         if(r3Logger > 0)
         {
            r3LogHandler = CreateFile(r3LogPath,GENERIC_WRITE,
                                      0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);  
            if((r3LogHandler == NULL) || (r3LogHandler == INVALID_HANDLE_VALUE))
                r3_myExit("CreateFile (logFile) Fail !\r\n",-1);
         }    
         char  r3LogData[R3_MAX_LOG_SIZE + 2] = {'\0'};                    
         BOOL  r3DebugHandler                 = DebugActiveProcess(r3Pid);  
         if(r3DebugHandler == FALSE) r3_myExit("DebugActiveProcess (r3Pid) fail!\r\n",-1);
         DWORD       r3Wbytes;
         DWORD       r3DebugStatus;
         DEBUG_EVENT r3DebugEvent; 
         while(TRUE)
         {
               r3DebugStatus = DBG_CONTINUE;    
               WaitForDebugEvent(&r3DebugEvent,INFINITE);      
               switch(r3DebugEvent.dwDebugEventCode) 
               { 
                      case CREATE_PROCESS_DEBUG_EVENT:                 
                           snprintf(r3LogData,R3_MAX_LOG_SIZE,
                                              "[#]. _debug_event_ (CREATE_PROCESS)\n"
                                              "[+]. lpImageName : %s\n",((r3DebugEvent.u.LoadDll.lpImageName != NULL) &&
                                                                          (r3DebugEvent.u.LoadDll.fUnicode == 0)) ? 
                                                                           r3DebugEvent.u.LoadDll.lpImageName : "EMPTY_CHAIN");
                           printf("%s",r3LogData);
                           if(r3Logger > 0) 
                           {
                              if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                 (r3Wbytes < strlen(r3LogData))) 
                                  fputs("[error]. Write log fail (CREATE_PROCESS_EVENT)! \r\n",stdout);             
                           }
                           memset(r3LogData,'\0',strlen(r3LogData));
                      break;    
                      case CREATE_THREAD_DEBUG_EVENT:
                           //...
                      break;    
                      case EXIT_PROCESS_DEBUG_EVENT:
                           printf("[+]. Process PID(%d) send EXIT_PROCESS_DEBUG_EVENT\r\n",r3Pid);
                           if(r3Logger > 0) CloseHandle(r3LogHandler);
                           return 0;
                      break;
                      case EXIT_THREAD_DEBUG_EVENT:
                           //,,,,
                      break;
                      case LOAD_DLL_DEBUG_EVENT:
                           snprintf(r3LogData,R3_MAX_LOG_SIZE,
                                              "[#]. _debug_event_ (LOAD_DLL_DEBUG_EVENT)\n"
                                              "[+]. lpImageName : %s\n",((r3DebugEvent.u.LoadDll.lpImageName != NULL) && 
                                                                          (r3DebugEvent.u.LoadDll.fUnicode == 0)) ? 
                                                                           r3DebugEvent.u.LoadDll.lpImageName : "EMPTY_CHAIN");
                           printf("%s",r3LogData);
                           if(r3Logger > 0)
                           {
                              if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                 (r3Wbytes < strlen(r3LogData))) 
                                  fputs("[error]. Write log fail (LOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                           }
                           memset(r3LogData,'\0',strlen(r3LogData));
                      break;
                      case OUTPUT_DEBUG_STRING_EVENT:
                           ///.....
                      break;
                      case RIP_EVENT:                        //debuger fuckup
                        //   DebugActiveProcessStop(r3Pid);      //.. ? .. ! .. ?
                           if(r3Logger > 0) CloseHandle(r3LogHandler);
                           r3_myExit("[error]. Debuger fuckup (RiP)\r\n",-1); 
                      break; 
                      case UNLOAD_DLL_DEBUG_EVENT:
                           snprintf(r3LogData,R3_MAX_LOG_SIZE,
                                              "[#]. _debug_event_ (UNLOAD_DLL_DEBUG_EVENT)\n"
                                              "[+]. %x\n",r3DebugEvent.u.UnloadDll.lpBaseOfDll);
                           printf("%s",r3LogData);
                           if(r3Logger > 0)
                           {
                              if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                 (r3Wbytes < strlen(r3LogData))) 
                                  fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                           }
                           memset(r3LogData,'\0',strlen(r3LogData));
                      break;
                      case EXCEPTION_DEBUG_EVENT:                                         //powitajmy go brawami
                           switch(r3DebugEvent.u.Exception.ExceptionRecord.ExceptionCode)
                           {  
                                  case EXCEPTION_STACK_OVERFLOW:    //Continue or Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       if(r3_onTrueException(r3Interactive,r3Deep) !=  R3_CONTINUE)
                                       {
                                          if(r3Logger > 0) CloseHandle(r3LogHandler);
                                          return 0;                                    
                                       }
                                       r3DebugStatus = DBG_EXCEPTION_NOT_HANDLED;
                                       memset(r3LogData,'\0',strlen(r3LogData));
                                  break; 
                                  case EXCEPTION_SINGLE_STEP:                //deep = 0
                                  break; 
                                  case EXCEPTION_PRIV_INSTRUCTION:     //Continue or Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       if(r3_onTrueException(r3Interactive,r3Deep) !=  R3_CONTINUE)
                                       {
                                          if(r3Logger > 0) CloseHandle(r3LogHandler);
                                          return 0;                                    
                                       }
                                       r3DebugStatus = DBG_EXCEPTION_NOT_HANDLED;
                                       memset(r3LogData,'\0',strlen(r3LogData));
                                  break; 
                                  case EXCEPTION_NONCONTINUABLE_EXCEPTION: //Kill By Default
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       r3_onTrueException(r3Interactive,1);
                                       if(r3Logger > 0) CloseHandle(r3LogHandler);
                                       return 0;                                    
                                  break;  
                                  case EXCEPTION_INVALID_DISPOSITION:   //Continue or Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       if(r3_onTrueException(r3Interactive,r3Deep) !=  R3_CONTINUE)
                                       {
                                          if(r3Logger > 0) CloseHandle(r3LogHandler);
                                          return 0;                                    
                                       }
                                       r3DebugStatus = DBG_EXCEPTION_NOT_HANDLED;
                                       memset(r3LogData,'\0',strlen(r3LogData));
                                  break; 
                                  case EXCEPTION_INT_OVERFLOW:        //Kill By Deafult
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       r3_onTrueException(r3Interactive,1);
                                       if(r3Logger > 0) CloseHandle(r3LogHandler);
                                       return 0;       
                                  break;     
                                  case EXCEPTION_INT_DIVIDE_BY_ZERO:  //Continue or Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       if(r3_onTrueException(r3Interactive,r3Deep) !=  R3_CONTINUE)
                                       {
                                          if(r3Logger > 0) CloseHandle(r3LogHandler);
                                          return 0;                                    
                                       }
                                       r3DebugStatus = DBG_EXCEPTION_NOT_HANDLED;  
                                       memset(r3LogData,'\0',strlen(r3LogData));
                                  break;                                      
                                  case EXCEPTION_IN_PAGE_ERROR:   //Continue or Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       if(r3_onTrueException(r3Interactive,r3Deep) !=  R3_CONTINUE)
                                       {
                                          if(r3Logger > 0) CloseHandle(r3LogHandler);
                                          return 0;                                    
                                       }
                                       r3DebugStatus = DBG_EXCEPTION_NOT_HANDLED;
                                       memset(r3LogData,'\0',strlen(r3LogData));
                                  break;  
                                  case EXCEPTION_ILLEGAL_INSTRUCTION:   //Always Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       r3_onTrueException(r3Interactive,1);
                                       if(r3Logger > 0) CloseHandle(r3LogHandler);
                                       return 0;    
                                  break;  
                                  case EXCEPTION_FLT_UNDERFLOW:
                                  break;
                                  case EXCEPTION_FLT_STACK_CHECK:
                                  break;
                                  case EXCEPTION_FLT_OVERFLOW:
                                  break;
                                  case EXCEPTION_FLT_INVALID_OPERATION:
                                  break;
                                  case EXCEPTION_FLT_INEXACT_RESULT:
                                  break;
                                  case EXCEPTION_FLT_DIVIDE_BY_ZERO:
                                  break;
                                  case EXCEPTION_FLT_DENORMAL_OPERAND:
                                  break;
                                  case EXCEPTION_DATATYPE_MISALIGNMENT:   //Continue or Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       if(r3_onTrueException(r3Interactive,r3Deep) !=  R3_CONTINUE)
                                       {
                                          if(r3Logger > 0) CloseHandle(r3LogHandler);
                                          return 0;                                    
                                       }
                                       r3DebugStatus = DBG_EXCEPTION_NOT_HANDLED;
                                       memset(r3LogData,'\0',strlen(r3LogData));
                                  break;
                                  case EXCEPTION_BREAKPOINT:
                                  break;
                                  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   //Always Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       r3_onTrueException(r3Interactive,1);
                                       if(r3Logger > 0) CloseHandle(r3LogHandler);
                                       return 0;
                                  break;
                                  case EXCEPTION_ACCESS_VIOLATION:   //Always Kill
                                       r3_parseException(r3DebugEvent.u.Exception.ExceptionRecord,r3LogData);                                  
                                       printf("%s",r3LogData);
                                       if(r3Logger > 0)
                                       {
                                          if((!WriteFile(r3LogHandler,r3LogData,strlen(r3LogData),&r3Wbytes,0)) || 
                                             (r3Wbytes < strlen(r3LogData))) 
                                              fputs("[error]. Write log fail (UNLOAD_DLL_DEBUG_EVENT)! \r\n",stdout);                    
                                       }
                                       r3_onTrueException(r3Interactive,1);
                                       if(r3Logger > 0) CloseHandle(r3LogHandler);
                                       return 0;
                                  break;
                           }
                      break;
               }    
               ContinueDebugEvent(r3DebugEvent.dwProcessId,r3DebugEvent.dwThreadId,r3DebugStatus);
         }   
         return 0;   
}
