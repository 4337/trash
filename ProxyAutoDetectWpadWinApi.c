#include <windows.h>
#include <stdio.h>
#include <Wininet.h>

#define PAC_NAME "J:\\_0001PROXY_GET_INFO_\\downwpad.pac"

typedef BOOL (CALLBACK *pfnInternetInitializeAutoProxyDll)(
              DWORD dwVersion,
              LPSTR lpszDownloadedTempFile,
              LPSTR lpszMime,
              AutoProxyHelperFunctions* lpAutoProxyCallbacks,
              LPAUTO_PROXY_SCRIPT_BUFFER lpAutoProxyScriptBuffer);
typedef BOOL (CALLBACK *pfnInternetDeInitializeAutoProxyDll)(LPSTR lpszMime,DWORD dwReserved);
typedef BOOL (CALLBACK *pfnInternetGetProxyInfo)(  
	          LPCSTR lpszUrl,
              DWORD dwUrlLength,
              LPSTR lpszUrlHostName,
              DWORD dwUrlHostNameLength,
             LPSTR *lplpszProxyHostName,
             LPDWORD lpdwProxyHostNameLength);

int main(int argc,char** argv)
{
	int r = -1;
	if(argc < 2) {
	   printf("Use .exe ip/hostname\r\n");
	   exit(-1);
	} else {
		 WSADATA wsza;
		 HMODULE jsdll;
         if(WSAStartup(MAKEWORD(2,2),&wsza) == 0) {
			struct hostent* host;
            struct in_addr iAddr; 
			iAddr.s_addr = inet_addr(argv[1]);
			if(iAddr.s_addr == INADDR_NONE) {
			   host = gethostbyname(argv[1]);
			   if(host == NULL) {
				  printf("Bad target %s\r\n",argv[1]);
				  goto clear;
			   }
			} else if((host = gethostbyaddr((char*)&iAddr,4,AF_INET)) == NULL) {
                 printf("Bad target %s\r\n",argv[1]);
				 goto clear;
			} else {
			  printf("Target %s\r\n",argv[1]);
			}
			jsdll = (HMODULE)LoadLibrary("jsproxy.dll");
			if(jsdll != NULL)
			{
				pfnInternetInitializeAutoProxyDll   _pInternetInitializeAutoProxyDll  = (pfnInternetInitializeAutoProxyDll)GetProcAddress(jsdll,"InternetInitializeAutoProxyDll");
                pfnInternetDeInitializeAutoProxyDll _pInternetDeInitializeAutoProxyDll = (pfnInternetDeInitializeAutoProxyDll)GetProcAddress(jsdll,"InternetDeInitializeAutoProxyDll");
                pfnInternetGetProxyInfo             _pInternetGetProxyInfo             = (pfnInternetGetProxyInfo)GetProcAddress(jsdll,"InternetGetProxyInfo");
				if( _pInternetInitializeAutoProxyDll == 0 || _pInternetDeInitializeAutoProxyDll == 0 ||
					_pInternetGetProxyInfo  == 0) {
				   printf("GetProcAddrr fail !\r\n");
                   r = -1;
				} else {
				  char WpadUri[256 * 2 + 1] = {0};
				  if(!DetectAutoProxyUrl(WpadUri,256 * 2,PROXY_AUTO_DETECT_TYPE_DHCP | PROXY_AUTO_DETECT_TYPE_DNS_A)) {
					 printf("AutoProxyUrl Detect fail !\r\n");
					 r = -1;
				  } else {
					  AutoProxyHelperFunctions HelpFun;
					  URLDownloadToFile(NULL,WpadUri,PAC_NAME,0,NULL);
					  if(!_pInternetInitializeAutoProxyDll(0,PAC_NAME,NULL,&HelpFun,NULL)) {  /* add helper function */
						  printf("_pInternetInitializeAutoProxyDll fail %d %x!\r\n",GetLastError(),GetLastError());
						  r = -1;
					  } else {
						  char turl[256 * 3 + 1] = {0};
						  char thost[256 * 2 + 1] = {0};
						  char proxy[256 * 4 + 1] = {0};
						  char* pproxy = proxy;
						  DWORD proxyLen = 256 * 4;
						  sprintf(turl,"http://%s/u/bot.php",host->h_name);
						  sprintf(thost,"http://%s",host->h_name);
						  printf("DstHostName %s\r\n",thost);
						  if(!_pInternetGetProxyInfo((LPSTR)turl,(DWORD)strlen(turl),
							                        (LPSTR)thost,(DWORD)strlen(thost),
													&pproxy,&proxyLen)) {
							printf("_pInternetGetProxyInfo fail \r\n");
							r = -1;
						  } else {
							printf("Dst Res   %s\r\n"
								   "Argv[1]    %s\r\n"
								   "Via Proxy %s\r\n",turl,argv[1],pproxy);
							r = 1;
						  }
						  _pInternetDeInitializeAutoProxyDll(NULL,0);
					  }
					  DeleteFile(PAC_NAME); 
				  }
				}
				FreeLibrary(jsdll);
			}
		  } 
		clear: WSACleanup();
	}
	return r;
}