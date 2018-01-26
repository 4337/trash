#ifndef _MEXTERNAL_H
#define _MEXTERNAL_H

#ifndef _WINDOWS_H
#include <windows.h>
#endif
#ifndef _IPHLPAPI_H
#include <Iphlpapi.h>
#endif

#define M_DEBUG 1
#define M_TIMEOUT 1000
#define M_TRACE "69.99.104.111"               //change if u must
#define MAX_HOSTNAME 255

typedef DWORD( *GETADPINFO )( PIP_ADAPTER_INFO,PULONG );
typedef DWORD( *ICMPSENDECHO )( HANDLE,IPAddr,LPVOID,WORD,PIP_OPTION_INFORMATION,LPVOID,DWORD,DWORD );
typedef HANDLE( *ICMPCREATEFILE )( void );
typedef BOOL( *ICMPCLOSEHANDLE )( HANDLE );
typedef DWORD(*GETIPSTAT)(PMIB_IPSTATS);

#define LOADLIB_ERROR        100001
#define GETPROCESSADDR_ERROR 100002
#define GETADPINFO_ERROR     100003
#define ICMPCREATEFILE_ERROR 100004
#define GETDEFAULTTTL_ERROR  100005
#define GETHOSTNAME_ERROR    100006
#define GETHOSTBYNAME_ERROR  100007
#define INETNTOA_ERROR       100008

typedef struct APIZ
{
       HMODULE         hLib; 
       ICMPSENDECHO    mIcmpSendEcho;
       ICMPCREATEFILE  mIcmpCreateFile;
       ICMPCLOSEHANDLE mIcmpCloseHandle;
       GETADPINFO      mGetAdpInfo;
       GETIPSTAT       mGetDefTtl;
        
}MAPIZ,*PMAPIZ;  

INT   Init( PMAPIZ apiInit );
DWORD GetCorrectTtl( PMAPIZ apiIit );
DWORD GetExternalIp( char* outBuf , DWORD TTTL, PMAPIZ _f );
UINT  IsInNat(char* addr);
VOID  mFree( HMODULE hLib );

#endif
