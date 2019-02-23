#include <WinSock2.h>
#include <exception>
#include <Windows.h>

#include "Network.h"

using namespace std;

Network::Socket*  Network::Socket::instance = 0;

__stdcall
Network::Socket::Socket() throw(...)  {

	::memset(WinSock, INVALID_SOCKET, 2);

	WORD WSAVersion = MAKEWORD(2, 2);
	if (::WSAStartup(WSAVersion, &Wsa) != 0)
		throw new NetworkException("WSAStartup error");

	if ((WinSock[GLOBAL_SOCK] = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		throw new NetworkException("socket() error");

}

__stdcall
Network::Socket::~Socket() noexcept {

	WSACleanup();

	if (INVALID_SOCKET != WinSock[GLOBAL_SOCK]) 
		::closesocket(WinSock[GLOBAL_SOCK]);

	if (INVALID_SOCKET != WinSock[ACCEPT_SOCK])
		::closesocket(WinSock[ACCEPT_SOCK]);

}


Network::Socket* __stdcall
Network::Socket::GetInstance() noexcept {

	if (instance == 0) {
		try {
			instance = new Socket();
		} catch (...) {
			   instance = 0;
		}
	}
	return instance;
}

BOOL __stdcall 
Network::Socket::Listen(unsigned short wPort, const char* const sBindAddress) throw(...) {

	sockaddr_in WinSvc;

	WinSvc.sin_family = AF_INET;
#pragma warning(disable : 4996)
 	WinSvc.sin_addr.s_addr = inet_addr(sBindAddress);
	WinSvc.sin_port = htons(wPort);

	if (::bind(WinSock[GLOBAL_SOCK], (SOCKADDR *)&WinSvc, sizeof(WinSvc)) == SOCKET_ERROR) {
		return FALSE;
	}

	if (::listen(WinSock[GLOBAL_SOCK], SOMAXCONN) == SOCKET_ERROR) {
		return FALSE;
	}

	return TRUE;
}

INT __stdcall 
Network::Socket::Recv(const char* vBuff, unsigned int uiLen) throw(...) {

	    return static_cast<INT>(::recv(WinSock[ACCEPT_SOCK], const_cast<char*>(vBuff), static_cast<int>(uiLen), 0));
}


INT __stdcall 
Network::Socket::Send(const char* vBuff, unsigned int uiLen) throw(...) {

	    return static_cast<INT>(::send(WinSock[ACCEPT_SOCK], vBuff, static_cast<int>(uiLen), 0));
}


BOOL __stdcall 
Network::Socket::Accept(sockaddr* SockAddrIn) throw(...) {

	    WinSock[ACCEPT_SOCK] = ::accept(WinSock[GLOBAL_SOCK], SockAddrIn, 0);
		if (WinSock[ACCEPT_SOCK] == INVALID_SOCKET) {
			return FALSE;
		}
		return TRUE;
}

void __stdcall 
Network::Socket::Close(void) throw(...) {

	if (WinSock[ACCEPT_SOCK] != INVALID_SOCKET) {
		::shutdown(WinSock[ACCEPT_SOCK], SD_BOTH);
		::closesocket(WinSock[ACCEPT_SOCK]);
		WinSock[ACCEPT_SOCK] = INVALID_SOCKET;
	}

	if (WinSock[GLOBAL_SOCK] != INVALID_SOCKET) {
		::shutdown(WinSock[GLOBAL_SOCK], SD_BOTH);
		::closesocket(WinSock[GLOBAL_SOCK]);
		WinSock[GLOBAL_SOCK] = INVALID_SOCKET;
	}
}