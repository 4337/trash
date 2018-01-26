/*
*  bla bla bla
*  2oo10
*/
#include "mexternal.h"

VOID  mFree( HMODULE hLib )
{
            FreeLibrary( hLib );     
}      

INT Init( PMAPIZ apiInit )
{
         apiInit->hLib = LoadLibrary( "Iphlpapi.dll" );
         if( apiInit->hLib == NULL ) apiInit->hLib = LoadLibrary( "Icmp.dll" );
         if( apiInit->hLib == NULL ) return LOADLIB_ERROR;
         apiInit->mIcmpCloseHandle  = ( ICMPCLOSEHANDLE )GetProcAddress( apiInit->hLib,"IcmpCloseHandle" );
         if( apiInit->mIcmpCloseHandle == NULL ) return GETPROCESSADDR_ERROR;
         apiInit->mIcmpCreateFile   = ( ICMPCREATEFILE )GetProcAddress( apiInit->hLib,"IcmpCreateFile" );
         if( apiInit->mIcmpCreateFile == NULL ) return GETPROCESSADDR_ERROR;
         apiInit->mIcmpSendEcho     = ( ICMPSENDECHO )GetProcAddress( apiInit->hLib,"IcmpSendEcho" ); 
         if( apiInit->mIcmpSendEcho == NULL ) return GETPROCESSADDR_ERROR;
         apiInit->mGetAdpInfo       = ( GETADPINFO )GetProcAddress( apiInit->hLib,"GetAdaptersInfo" );
         if( apiInit->mGetAdpInfo == NULL ) return GETPROCESSADDR_ERROR;
         apiInit->mGetDefTtl        = ( GETIPSTAT )GetProcAddress( apiInit->hLib,"GetIpStatistics" );
         if( apiInit->mGetDefTtl == NULL ) return GETPROCESSADDR_ERROR;
         return 0;   
}    
    
DWORD GetCorrectTtl( PMAPIZ apiInit )
{
                    DWORD retTtl;
                    IP_ADAPTER_INFO adapterInfo;
                    ULONG infoSize = sizeof( IP_ADAPTER_INFO );
                    if( apiInit->mGetAdpInfo( &adapterInfo,&infoSize ) != NO_ERROR )
                        return GETADPINFO_ERROR;
                        
                    const char* const tGateWay = adapterInfo.GatewayList.IpAddress.String;    
                    HANDLE hIcmp = apiInit->mIcmpCreateFile( );   
                    if( hIcmp == INVALID_HANDLE_VALUE ) return ICMPCREATEFILE_ERROR;  
                    
                    char  sendData[32] =  "ECHO";
                    ULONG uIpAddr      =  inet_addr( tGateWay ); 
                    DWORD repSize      =  sizeof( ICMP_ECHO_REPLY ) + sizeof( sendData );   
                    char  repBuf[ repSize ];
                    PICMP_ECHO_REPLY      repEcho;
                    DWORD icmpStat     =  apiInit->mIcmpSendEcho( hIcmp,uIpAddr,sendData,sizeof(sendData),
                                                                  NULL,&repBuf,sizeof( repBuf ),M_TIMEOUT );
                    if( icmpStat != 0 )
                    { 
                        repEcho = ( PICMP_ECHO_REPLY )repBuf; 
                        retTtl  =  repEcho->Options.Ttl;
                        #if M_DEBUG == 1
                        printf("Debug [GateWay TTL] is %d \r\n",retTtl); 
                        #endif
                    }    
                    else 
                    {
                        MIB_IPSTATS mibIpStat;   
                        DWORD mibRet = apiInit->mGetDefTtl( &mibIpStat );
                        if( mibRet != NO_ERROR ) return GETDEFAULTTTL_ERROR; 
                        retTtl = mibIpStat.dwDefaultTTL;
                    }     
                    apiInit->mIcmpCloseHandle( hIcmp );
                    return retTtl; 
}            

UINT IsInNat(char* addr)
{
     #define size 2
     int pRange[size][3] = {{10,255,255},
                            {172,16,31}};      
     struct in_addr sAddr;
     sAddr.S_un.S_addr = inet_addr( addr );   
     #if M_DEBUG == 1
     printf("Debug [First two octets] %d %d\r\n",sAddr.S_un.S_un_b.s_b1,sAddr.S_un.S_un_b.s_b2); 
     #endif
     UINT i;
     if(( sAddr.S_un.S_un_b.s_b1 == 192 ) && ( sAddr.S_un.S_un_b.s_b2 == 168 )) return 1;
     for(i=0; i<size; i++)
     {
         if( sAddr.S_un.S_un_b.s_b1 == pRange[i][0] )
         {
             if(( sAddr.S_un.S_un_b.s_b2 >= pRange[i][1] ) && 
                (  sAddr.S_un.S_un_b.s_b2 <= pRange[i][2] ))
                return 1;
         }              
     }                    
     return 0;
}     

DWORD GetExternalIp( char* outBuf , DWORD TTTL, PMAPIZ apiInit )
{
                     char* retIpAddr = NULL;
                     struct in_addr replyAddr;
                     char  compName[ MAX_HOSTNAME + 2 ];
                     if(gethostname( compName,MAX_HOSTNAME ) == SOCKET_ERROR )
                       return GETHOSTNAME_ERROR;
                     
                     struct hostent* hostName = gethostbyname( compName );
                     if( hostName == NULL ) return GETHOSTBYNAME_ERROR;        
                      
                     retIpAddr = inet_ntoa( *(struct in_addr*) *hostName->h_addr_list );
                     if( retIpAddr == NULL ) return INETNTOA_ERROR;  
                   
                     if( IsInNat( retIpAddr ) == 1 )
                     {
                         DWORD  icmpStat;
                         HANDLE hIcmp = apiInit->mIcmpCreateFile( );
                         if( hIcmp == INVALID_HANDLE_VALUE )  return  ICMPCREATEFILE_ERROR;
                         ULONG uIpAddr      =  inet_addr( M_TRACE );    
                         char  sendData[32] = "ECHO";
                         DWORD repSize      = sizeof( ICMP_ECHO_REPLY ) + sizeof( sendData );               
                         IP_OPTION_INFORMATION IP_INFO32 = {0};
                         PICMP_ECHO_REPLY      repEcho;
                         UCHAR i;
                         for( i=1; i<255; i++ )
                         {
                              IP_INFO32.Ttl = i;
                              char repBuf[ repSize ];   
                              icmpStat     = apiInit->mIcmpSendEcho( hIcmp,uIpAddr,sendData,sizeof( sendData ),
                                                                     &IP_INFO32,repBuf,sizeof( repBuf ),M_TIMEOUT );               
                              if( icmpStat != 0 )
                              {
                                  repEcho = ( PICMP_ECHO_REPLY )repBuf;
                                  replyAddr.S_un.S_addr = repEcho->Address;
                                  if( IsInNat( inet_ntoa( replyAddr ) ) == 0) break;    
                              }  
                         }
                         UINT ix;
                         DWORD rTtl;
                         for(ix=2; ix<253; ix++)
                         {
                             char repBuf[ repSize ];
                             replyAddr.S_un.S_un_b.s_b4 = ix;  
                             uIpAddr  = inet_addr( inet_ntoa( replyAddr ) );
                             icmpStat =  apiInit->mIcmpSendEcho( hIcmp,uIpAddr,sendData,sizeof( sendData ),
                                                                 NULL,repBuf,sizeof( repBuf ),M_TIMEOUT );              
                             if( icmpStat != 0 )
                             {
                                 repEcho = ( PICMP_ECHO_REPLY )repBuf; 
                                 rTtl    = repEcho->Options.Ttl;
                                 #if M_DEBUG == 1
                                 printf("Debug : ping reply form %s rTTl %d \r\n",inet_ntoa(  replyAddr ),
                                                                                  repEcho->Options.Ttl );
                                 #endif
                                 if( rTtl == TTTL )
                                 {
                                          retIpAddr = inet_ntoa( replyAddr );
                                          #if DEBUG == 1
                                          printf("Debug: We got it!! Your external ip address is %s \r\n", retIpAddr );
                                          #endif
                                          break;
                                 }      
                             }    
                         }  
                       apiInit->mIcmpCloseHandle( hIcmp );          
                     }  
        strcpy(outBuf,retIpAddr);
      return 0; 
}      
