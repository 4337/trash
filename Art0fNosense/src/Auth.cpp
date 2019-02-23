#include <string>
#include <vector>
#include <Windows.h>

#include "Auth.h"

__stdcall
Auth::AuthUserAndPasswd::AuthUserAndPasswd() noexcept {

	UserLists.push_back({ "user1","user1" });
	UserLists.push_back({ "user2","user2" });
	UserLists.push_back({ "test","test1" });

}

__stdcall 
Auth::AuthUserAndPasswd::~AuthUserAndPasswd() noexcept {
}

int __stdcall 
Auth::AuthUserAndPasswd::CheckUserLogin(const char* const sUserLogin) noexcept {

	for (size_t i = 0; i < UserLists.size(); i++) {
		if (UserLists[i].GetUserName() == sUserLogin) return i;
	}
	return -1;

}

bool __stdcall
Auth::AuthUserAndPasswd::CheckUserPassword(const char* const sUserPwd,int userIndex) noexcept {

    if (UserLists[userIndex].GetUserPwd() == sUserPwd) return true;
	return false;

}

int __stdcall 
Auth::AuthUserAndPasswd::DoAuthenticate(Messages& out) noexcept {

		::memset(out.sMsg,'\0',sizeof(out.sMsg));
		int ret = CheckUserLogin(uname.c_str());
		if (ret != -1) {
			if(CheckUserPassword(pwd.c_str(),ret) == false) {
				::snprintf(out.sMsg, uname.length(), "%s", uname.c_str());
				return 0;
			} else {  
				::snprintf(out.sMsg, strlen("Ups this should not happen dude : (") + 5, "Ups this should not happen dude :(\r\n");
			}
		} else { 
			::snprintf(out.sMsg, uname.length(), "%s", uname.c_str());
			return 2;
		}
		return 1;
}