#include <WinSock2.h>
#include <Windows.h>
#include <winsvc.h>


#include "Global.h"
#include "Network.h"
#include "Auth.h"

using namespace std;
using namespace Network;

DWORD WINAPI 
Global::SvcHandler(DWORD ctrl, DWORD type, VOID* data, VOID* context) {
	switch (ctrl) {
	    case SERVICE_CONTROL_SHUTDOWN:	 
		break;
		case SERVICE_CONTROL_STOP:
	    break;
		default:
		break;
	}
	return NO_ERROR;
}

void WINAPI 
Global::SvcReportStatus(SERVICE_STATUS_HANDLE SvcStatHnd, DWORD status) {
	   
	SERVICE_STATUS SvcStatus = {
		  SERVICE_WIN32_OWN_PROCESS,
		  status,
		  (status == SERVICE_START_PENDING) ? (DWORD)0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
		  NO_ERROR,
		  (DWORD)0,
		  (DWORD)0,
		  (DWORD)0
	};

	SetServiceStatus(SvcStatHnd, &SvcStatus);

}

void WINAPI 
Global::ServiceMain(DWORD argc, LPCSTR* argv) {

	 SERVICE_STATUS_HANDLE sSvcHndStatus = RegisterServiceCtrlHandlerEx(Global::SVC_NAME, &Global::SvcHandler, 0);
	 SvcReportStatus(sSvcHndStatus, SERVICE_START_PENDING);
	 HANDLE Stop = CreateEvent(0, TRUE, FALSE, 0);
	 SvcReportStatus(sSvcHndStatus, SERVICE_RUNNING);

	 TCPServer<NetworkEvent>* TcpSvc = new TCPServer<NetworkEvent>();
	 NetworkEvent* Event = TcpSvc->GetIo();

	 while (WaitForSingleObject(Stop, 1000) != WAIT_OBJECT_0) {
	
		 if (Event != 0) {
			 int i = 0;
			 std::string Data;
			 Auth::Messages AuthMsg;
			 Auth::AuthUserAndPasswd Authenticate;
			 std::thread th = TcpSvc->MultiThread("0.0.0.0", 4337);
			 for (;;) {
				 Data = Event->Get();
				 if (Data.length() > 0) {
					 if (i == 0) {
						 Authenticate.SetUserName(Data.erase(Data.length() - 3).c_str());
						 Event->Set("PASSWORD PLEASE");
					 }
					 if (i == 1) {
						 Authenticate.SetUserPassword(Data.erase(Data.length() - 3).c_str());
						 int AuthResult = Authenticate.DoAuthenticate(AuthMsg);
						 if (AuthMsg.CB != 0) {
							 AuthMsg.CB();
						 }
						 Event->Set(AuthMsg.sMessage[AuthResult]);
						 if (AuthResult != 1) {
							 i = 0;
							 continue;
						 }
						 if (AuthMsg.CB != 0) {
							 AuthMsg.CB();
						 }
					 }
					 //auth ok
					 Global::DepHubEmpire();
					 ++i;
				 }
			 }
			 th.join();
		 }
	 }

	 SvcReportStatus(sSvcHndStatus, SERVICE_STOP_PENDING);
     CloseHandle(Stop);
	 SvcReportStatus(sSvcHndStatus, SERVICE_STOPPED);

#if DEBUG_TST == true
	 Global::UnitTest_IO();
	 Global::UnitTest_InOut("TestString");
#endif

}

void __stdcall
Global::UnitTest_IO() throw(...) {

	Network::Socket* s = Network::Socket::GetInstance();
	HANDLE h = CreateFile("file", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h != INVALID_HANDLE_VALUE) {
		DWORD size = GetFileSize(h, 0);
		if ((size != INVALID_FILE_SIZE) && (size > 0)) {
			char* buff = (char*)VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (buff != 0) {
				DWORD readed;
				if (ReadFile(h, &buff[0],size,&readed,0 )) {
					s->Send(buff, size);
				}
				VirtualFree(buff, 0, MEM_RELEASE);
			}
		}
		CloseHandle(h);
	}

};

DWORD __stdcall 
Global::InstallService() noexcept {
      
	DWORD ret = 0;

	SC_HANDLE hSCMan = 0;
	SC_HANDLE hSvc   = 0;

	char sPath[MAX_PATH] = { 0 };

	if (!::GetModuleFileName(0, sPath, MAX_PATH)) {
		return -1;
	}

	hSCMan = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (hSCMan == 0) {
		return ::GetLastError( );
	}

	hSvc = ::CreateService(hSCMan, Global::SVC_NAME, Global::SVC_NAME, 
		                   SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, 
		                   SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		                   sPath, 0, 0, 0, 0, 0);
	ret = ::GetLastError();

	if (hSvc != 0) {

		SC_ACTION SCAction[1];
		SERVICE_FAILURE_ACTIONS Fail;

		SCAction[0].Delay = 3000;
		SCAction[0].Type = SC_ACTION_RESTART;
		SCAction[1].Delay = 3000;
		SCAction[1].Type = SC_ACTION_RESTART;

		Fail.lpCommand = &sPath[0];
		Fail.cActions = 1;
		Fail.lpsaActions = &SCAction[0];

		if (::ChangeServiceConfig2(hSvc, SERVICE_CONFIG_FAILURE_ACTIONS, &Fail) == FALSE) {
			ret = ::GetLastError();
		}

		if (::StartService(hSvc, 0, 0) == FALSE) {
			ret = ::GetLastError();
		}

		::CloseServiceHandle(hSvc);
	} 

	::CloseServiceHandle(hSCMan);

	return ret;
}

void __stdcall 
Global::UnitTest_InOut(const char* str) noexcept {

	MSGBOX(0, "Hey", str, 0);
    //...
}


