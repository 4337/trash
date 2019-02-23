#pragma once

#include <vector>
#include <string>
#include <thread>
#include <Windows.h>

#include "Global.h"

namespace Network {

	class NetworkException : public std::exception {
		  
	      public:
		  
			  NetworkException(const char* const) : std::exception() {
			  }

	};

	class Socket {

		enum {
			GLOBAL_SOCK = 0,
			ACCEPT_SOCK = 1
		};

		WSAData Wsa;

		SOCKET WinSock[2];

		__stdcall Socket(const Socket&);

		const Socket& __stdcall operator=(const Socket&) {
		}

		__stdcall Socket() throw(...);

		static Socket* instance;

	    public :

			   __stdcall ~Socket() noexcept;

			   static Socket* __stdcall GetInstance() noexcept;

			   BOOL __stdcall Listen(unsigned short, const char* const) throw(...);

			   BOOL __stdcall Accept(sockaddr* = 0) throw(...);

			   INT __stdcall Recv(const char*, unsigned int) throw(...);

			   INT __stdcall Send(const char*, unsigned int) throw(...);

			   void __stdcall Close(void) throw(...);

			   void __stdcall CloseConnection(void) noexcept {

				   if (WinSock[ACCEPT_SOCK] != INVALID_SOCKET) {

					   ::shutdown(WinSock[ACCEPT_SOCK], SD_BOTH);
					   ::closesocket(WinSock[ACCEPT_SOCK]);
					   WinSock[ACCEPT_SOCK] = INVALID_SOCKET;

				   }

			   }
    };

	class NetworkEvent {
		 
		  HANDLE Event;
		  std::string Data;
		  CRITICAL_SECTION WriteSync;

	      public:

			  __stdcall ~NetworkEvent() {
				  ::DeleteCriticalSection(&WriteSync);
			  }

			  __stdcall NetworkEvent() {
				           
				  Event = ::CreateEvent(0, FALSE, FALSE, 0);
				  InitializeCriticalSection(&WriteSync);

			  }

			  void __stdcall Set(const char* const buff) {

				      ::EnterCriticalSection(&WriteSync);
					  Data.assign(buff).append("\r\n");
					  ::SetEvent(Event);
					  ::LeaveCriticalSection(&WriteSync);

			  }

			  std::string __stdcall Get() const {
				 
				  if (WaitForSingleObject(Event, INFINITE) == WAIT_OBJECT_0) {
					  ::ResetEvent(Event);
				  }
				  return Data;

			  }
         
	};

	template<typename IOBuff>
	class TCPServer {
		 
		  IOBuff* IO;

		  Socket* Sock; 

	      public:  

	      TCPServer() throw(...)  {
			       IO = new IOBuff();
				   Sock = Socket::GetInstance();
				   if (Sock == 0)  throw new NetworkException("TCPServer Socket::GetInstance exception");
		  }

		  ~TCPServer() noexcept {

			  if (IO != 0) {
				  delete IO;
			  }
			  if (Sock != 0) { 
				  delete Sock;
			  }
		      
		  }

		  IOBuff* GetIo() const {
			  return IO;
		  }

		  std::thread __stdcall MultiThread(const char* const sIp, unsigned short wPort) {  
			    
			  return std::thread(&TCPServer::SingleThread, this, sIp, wPort);
		  }

		  BOOL __stdcall SingleThread(const char* const sIp, unsigned short wPort) throw(...) {
			      
			      register BOOL ret = FALSE;
				  if (Sock->Listen(wPort,sIp)) {
					  BOOL Exit = FALSE;  
					  BOOL BreakLoop = FALSE;
					  while (!Exit) {
						  BreakLoop = FALSE;
						  if (Sock->Accept() != TRUE) {
							  break;
						  } else {
							  int WSError = 0;
							  UINT RecvLen = 0, j = 0;
							  const UINT BuffSize = 1024;
							  Sock->Send(Global::Banner, strlen(Global::Banner));
							  while (!BreakLoop)
							  {
								  do {
									  const char* RecvBuff = (const char*)new char[(BuffSize * ++j) * sizeof(char)];
									  if (RecvBuff != 0) {
										  ::memset((void*)&RecvBuff[0], '\0', (BuffSize * j) * sizeof(char));
										  RecvLen = Sock->Recv(&RecvBuff[0], BuffSize * j);
										  WSError = WSAGetLastError();
										  if (SOCKET_ERROR == RecvLen) {
											  if ((WSError == WSAENOTCONN) || (WSError == WSAECONNRESET)) {
												   Sock->CloseConnection();
												   BreakLoop = TRUE;
												   break;
											  }
											  delete[] RecvBuff;
											  continue;
										  }
										  IO->Set(RecvBuff);
										  delete[] RecvBuff;
									  } else {
										  throw std::exception("OOM in TCPServer SingleThread !");
									  }
								  } while (WSAEMSGSIZE == WSError);
								  if (WSError == 0) {
									  std::string tmp = IO->Get();
									  const char* data = tmp.c_str();
									  if (Sock->Send(data, strlen(data)) == SOCKET_ERROR) {
										  WSError = WSAGetLastError();
										  if ((WSError == WSAENOTCONN) || (WSError == WSAECONNRESET)) {
											  Sock->CloseConnection();
											  BreakLoop = TRUE;
											  break;
										  }
									  }
								  }
							  }
						  }
					  }
				  }

				  return ret;
		  }

	};

};
