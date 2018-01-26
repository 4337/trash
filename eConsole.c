#include <windows.h>
#include <tchar.h>
#include <stdarg.h>
#include <shlwapi.h>
#include "eConsole.h"


#ifdef MSVC 
#pragma comment(lib,"shlwapi")
#endif
/*dev-cpp [-lshlwapi]*/
/* 
   Print IT Debug Console 
*/

HANDLE eConsoleInit()
{
       TCHAR tAppType[EENV_LENGTH + sizeof(TCHAR)] = "";
       if(GetEnvironmentVariable(ETARGET_ENV,tAppType,EENV_LENGTH) == 0)
          return NULL;
       else if(lstrcmp(ETARGET_ENVVAL,tAppType) != 0)   /* to jest w sumie nadmiarowe (konsola siê nie alokuje w przypadku appsów konsolowych)*/
               if(AllocConsole() == FALSE) return NULL;
       return GetStdHandle(STD_OUTPUT_HANDLE);   
}

DWORD eConsoleWrite(HANDLE iConsole,const TCHAR* iFmt,...)
{
      DWORD   tRet;
      va_list tList;
      TCHAR   tBuff[EMAX_CONSOLE_LINE + sizeof(TCHAR)] = ""; 
      va_start(tList,iFmt);
      wvnsprintf(tBuff,EMAX_CONSOLE_LINE,iFmt,tList);
      va_end(tList);
      if(WriteConsole(iConsole,tBuff,EMAX_CONSOLE_LINE,&tRet,NULL) == FALSE)
         return 0; 
      return tRet;
}


int main()
{
   eConsoleWrite(eConsoleInit(),TEXT("%d  %c %s"),125,0x42,TEXT("ALAMAKODEK"));
   eConsoleFree(); /* MaCro */
   return 0;   
}
