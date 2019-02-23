#pragma once

#define AUTH_MAX_DATA 1024

namespace Auth {

	struct Messages {

		  char  sMsg[AUTH_MAX_DATA];
		  const char* sMessage[4];
		  void*(*CB)(void);
		  unsigned short wErrorCnt;
		  
		  Messages() { 
			  ::memset(&sMsg[0], '\0', AUTH_MAX_DATA);
			  sMessage[0] = "Access denied";
			  sMessage[1] = "Access granted";
			  sMessage[2] = "Invalid user";
			  sMessage[3] = 0;
			  CB = 0;
			  wErrorCnt = 3;
		  }

		  void __stdcall SetMsgCb(void* fPtr) {
			  CB = (void*(*)(void))fPtr;
		  }

	};

	class AuthDB {
		 
		std::string username;
		std::string passwd;

	    public:
			__stdcall AuthDB(std::string login, std::string pwd) : username(login), passwd(pwd) {		      
			}

			std::string __stdcall GetUserName() noexcept {
				return username;
			}

			std::string __stdcall GetUserPwd() noexcept {
				return passwd;
			}

	};

	class AuthUserAndPasswd {

		  std::string pwd;
		  std::string uname;

		  std::vector<AuthDB> UserLists;

	      protected:

		  int __stdcall CheckUserLogin(const char* const) noexcept;

		  bool __stdcall CheckUserPassword(const char* const,int) noexcept;

	      public:

	      void __stdcall SetUserName(const char* const login) noexcept {
			  
				  (::strlen(login) > 0) ? uname.assign(login) : uname.assign("");

		  }

		  void __stdcall SetUserPassword(const char* const pass) noexcept {

			  (::strlen(pass) > 0) ? pwd.assign(pass) : pwd.assign("");

		  }

		  __stdcall AuthUserAndPasswd() noexcept;

		  __stdcall ~AuthUserAndPasswd() noexcept;

		  int __stdcall DoAuthenticate(Messages&) noexcept;

	};

};