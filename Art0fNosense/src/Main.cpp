#include <WinSock2.h>
#include <exception>
#include <Windows.h>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>

#include "Global.h"

int __cdecl main(int argc,char** argv)
{
	    
	    DWORD RunResult = Global::InstallService();

		if ((RunResult != ERROR_SUCCESS) && (RunResult != ERROR_SERVICE_EXISTS)) {
			DBGOUT("(!). Error in Global::IntallService() [GetLastError] : hex(0x%x)\r\n", RunResult);
			return -1;
		}

		SERVICE_TABLE_ENTRY SVCTable[] = {
			                              {
				                           const_cast<LPSTR>(Global::SVC_NAME),
										   reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(&Global::ServiceMain)
			                              },
		                                  {0,0}
		};

		StartServiceCtrlDispatcher(SVCTable);

		return 0;
}
