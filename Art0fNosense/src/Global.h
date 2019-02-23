#pragma once

#define DEBUG_TST true

namespace Global {

	static const char* SVC_NAME = "DepHub";

	static int(__stdcall *MSGBOX)(HWND, LPCTSTR, LPCTSTR, UINT) = &MessageBox;

	static char Banner[] = "Use username and password to login !\r\n";

	DWORD __stdcall InstallService() noexcept;

	void WINAPI ServiceMain(DWORD, LPCSTR*);

	DWORD WINAPI SvcHandler(DWORD, DWORD, VOID*, VOID*);

	void WINAPI SvcReportStatus(SERVICE_STATUS_HANDLE, DWORD);

#define DBGOUT(a,b) \
do { \
  ::printf(a,b); \
}while(0)

	static void DepHubEmpire() noexcept {
		return;
	}

	/*
	   @@Testy (file read)
	*/
	void __stdcall UnitTest_IO() throw(...);

	/*
	*/
	void __stdcall UnitTest_InOut(const char* str) noexcept;

	static void(__stdcall *UTC)() = &UnitTest_IO;

};
