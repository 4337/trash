#ifndef ECONSOLE_H
#define ECONSOLE_H

#define EMAX_CONSOLE_LINE 72 * sizeof(TCHAR)
#define eConsoleFree() \
        return  FreeConsole();
#define ETARGET_ENVVAL TEXT("Console")
#define ETARGET_ENV    TEXT("CLIENTNAME")
#define EENV_LENGTH    8 * sizeof(TCHAR)
          
HANDLE eConsoleInit();
DWORD  eConsoleWrite(HANDLE iConsole,const TCHAR* iFmt,...);



#endif