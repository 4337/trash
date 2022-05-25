//////////////////////////////////////////////////////////////
///23/05/2022 20:50
///Modu³ udostepnia informacje o systemie operacyjnym,
///aktualnie wykorzystywanym koncie u¿ytkownika oraz 
///udostêpnia interfejs do manipulowania podstawowym tokenem 
///dostêpowym, a tak¿e do dodawania nowy uprawnieñ do aktualnie 
///wykorzystywanego konta u¿tkownika - jeœli dysponujemy prawami 
///administratora. 
/////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <Windows.h>
#include <ntsecapi.h>
#include <lsalookup.h>

#include "Environment.h"

import Helpers;
import System;

/// <summary>
/// Konstrutor klasy Account.
/// </summary>
/// <returns>none</returns>
Juche::System::Account::Account() noexcept : user_name_(TEXT("")), primary_token_(NULL), 
                                             sid_{ false, 0, nullptr }, token_{ nullptr, {} } {

    /// <summary>
    /// RAII bez rzucania wyj¹tków.
    /// Trzeba tylko sprawdzaæ zwracane wartoœci.
    /// EG.
    /// Account ac;
    /// Privileges prv = ac.privileges();
    /// if(prv.privs_names.size() == 0 || prv.privs == nullptr) {
    ///    return -1337;
    /// }
    /// </summary>
    /// <returns></returns>
    set_username();
    set_token();
    set_sid();  

}

/// <summary>
/// Destruktor klasy Account.
/// </summary>
Juche::System::Account::~Account() {

    if (token_.privs != nullptr) {
        delete[] token_.privs;
    }

    if (sid_.sid != nullptr) {
        delete[] sid_.sid;
    }

    if (primary_token_ != NULL && primary_token_ != INVALID_HANDLE_VALUE) { 
        CloseHandle(primary_token_);
    }

}

/// <summary>
/// (private) Inicjalizuje sk³adow¹ Account::user_name_, która 
/// reprezentuje mazwê konta u¿ytkownika systemu powi¹zan¹ z aktualnym procesem/w¹tkiem.
/// </summary>
/// <returns>
/// true = sukces.
/// false = error.
/// </returns>
int 
Juche::System::Account::set_username() noexcept {

    TCHAR* buff;
    int ret = 0;
    DWORD size = 128;
    unsigned char i = 0;

    do {

        buff = new TCHAR[size];
        if (buff == nullptr) {
            return -1;
        }

        if (GetUserName(buff, &size) == FALSE) {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                ret = -1;
            }
            ++i;
        } else {
            user_name_ = buff;
            ret = 1;
        }

        delete[] buff;

    } while (ret == 0 && i < MAX_ERROR);

    return ret;

}

/// <summary>
/// Konwersja LUID -> do nazw uprawnieñ w formacie std::string | std::wstring.
/// </summary>
/// <returns>
/// -1 = error
/// 0 = error
/// 1 = sukces
/// </returns>
int 
Juche::System::Account::luid_2_privs() noexcept { //TODO: TEST

    if (token_.privs == nullptr) {
        return -1;
    }
    
    int ret = 1;
    DWORD size = 128;
    TCHAR* buff = nullptr;
    unsigned char error = 0;
    
    for (DWORD i = 0;
        i < token_.privs->PrivilegeCount && ret == 1; ) {

        buff = new TCHAR[size];
        if (buff == nullptr) {
            return -1;
        }

        if (LookupPrivilegeName(NULL, &token_.privs->Privileges[i].Luid, buff, &size) == 0) {

            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                ret = -1;
            } else {
                ++error;
            }

        } else {
          token_.privs_names.push_back(buff);
          error = 0;
          ++i;
        }

        delete[] buff;

        if (error >= MAX_ERROR) {
            return 0;
        }

    } 

    return ret;

}

/// <summary>
/// Inicjalizuje sk³adow¹ token_
/// </summary>
/// <returns>
/// -1 = error
///  0 = error mniejszy
///  1 = sukces
/// </returns>
int 
Juche::System::Account::set_token() noexcept {

    /// <summary>
    /// Powinniœmy zrobiæ DuplicateTokenEx, ale w sumie nie musimy.
    /// </summary>
    primary_token_ = primary_token(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_DUPLICATE | TOKEN_QUERY);
    if (primary_token_ == NULL) {
        return -1;
    }

    DWORD size;
    if (GetTokenInformation(primary_token_, TokenPrivileges, NULL, 0, &size) == 0) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return -1;
        }
    }

    token_.privs = new TOKEN_PRIVILEGES[size];

    if (token_.privs == nullptr) {
        return -1;
    }
    
    if (GetTokenInformation(primary_token_, TokenPrivileges, token_.privs, size, &size) == 0) {
        return -1;
    }

    int ret = luid_2_privs();
    if (ret != 1) {
        return ret;
    }

    return 1;

}

/// <summary>
/// Sprawdza czy u¿ykownik nale¿y do grupy lokalnych administratorów.
/// Ÿród³o: msdn
/// </summary>
/// <returns>
/// TRUE = nale¿y do grupy administraorów
/// FALSE = nie nale¿y
/// </returns>
BOOL 
Juche::System::Account::is_admin() noexcept {

    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    PSID sid;

    BOOL ret = AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                        DOMAIN_ALIAS_RID_ADMINS,
                                        0, 0, 0, 0, 0, 0, &sid);

    if (ret == TRUE) {

        if (!CheckTokenMembership(NULL, sid, &ret)) {
            ret = FALSE;
        }

        FreeSid(sid);
    }

    return ret;
 
}

/// <summary>
/// Inicjalizuje sk³adow¹ sid_.
/// Pobiera identyfikator SID u¿ytkownika definiowanego sk³adow¹ user_name_ 
/// (u¿ytkownik który jest w³aœcielem tego procesu).
/// </summary>
/// <returns>
/// true = sukces
/// false = error
/// </returns>
bool 
Juche::System::Account::set_sid() noexcept {

    SID_NAME_USE sid_use;
    DWORD sid_size, ad_size;

    if (LookupAccountName(nullptr, user_name_.c_str(), nullptr, &sid_size, 0, 
                          &ad_size, &sid_use) == 0) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return false;
        }
    }

    std::unique_ptr<TCHAR[]> domain(new TCHAR[ad_size]);

    if (!domain) {
        return false;
    }

    sid_.sid = new SID[sid_size];
    if (sid_.sid == nullptr) {
        return false;
    }

    if (LookupAccountName(0, user_name_.c_str(), sid_.sid, &sid_size,
                          domain.get(), 
                          &ad_size, &sid_use) == 0) {   
        return false;
    }

    sid_.init = true;
    sid_.sid_size = sid_size;

    return true;

}

/// <summary>
/// Przypisanie przenosz¹ce klasy Account.
/// </summary>
/// <param name="other">Account&&</param>
/// <returns>Account&</returns>
Juche::System::Account& 
Juche::System::Account::operator=(Account&& other) noexcept(false) { 

    if (this != &other) {

        primary_token_ = other.primary_token_;
        other.primary_token_ = NULL;

        if (other.sid_.sid != nullptr) {

            sid_.sid = new SID[other.sid_.sid_size];
            if (sid_.sid == nullptr) {
                throw std::bad_alloc();
            }
            CopySid(other.sid_.sid_size, sid_.sid, other.sid_.sid);

        } else {
          sid_.sid = nullptr;
        }

        sid_.init = other.sid_.init;
        sid_.sid_size = other.sid_.sid_size;
        user_name_ = other.user_name_;

        if (other.token_.privs != nullptr) {

            token_.privs = new TOKEN_PRIVILEGES[other.token_.privs->PrivilegeCount];
            if (token_.privs == nullptr) {
                throw std::bad_alloc();
            }
            memcpy(token_.privs, other.token_.privs, 
                   other.token_.privs->PrivilegeCount * sizeof(TOKEN_PRIVILEGES)); 

        }

        std::swap(token_.privs_names, other.token_.privs_names);

    }

    return *this;

}

/// <summary>
/// Konstruktor przenosz¹cy klasy Account.
/// </summary>
/// <param name="other">Account&&</param>
/// <returns></returns>
Juche::System::Account::Account(Account&& other) noexcept(false) {

    *this = std::move(other);

}

/// <summary>
/// Konstruktor kopiuj¹cy klasy Account.
/// </summary>
/// <param name="copy">Kopia/Ÿród³o</param>
/// <returns>none</returns>
Juche::System::Account::Account(const Account& copy) noexcept(false) {

    *this = copy;

}

/// <summary>
/// Przypisanie kopiuj¹ce klasy Account.
/// </summary>
/// <param name="copy">Kopia/Ÿród³o</param>
/// <returns>Obiekt Account utworzony na podstawie kopi.</returns>
Juche::System::Account&
Juche::System::Account::operator=(const Account& copy) noexcept(false) {   //TODO: trzeba to przetestowaæ bardzo 

    Juche::Helpers::Console(TEXT("Juche::System::Account::operator=\r\n"));

    if (this != &copy) {

        sid_.sid = new SID[copy.sid_.sid_size]; 
        if (sid_.sid == nullptr) {
            throw std::bad_alloc();
        }

        CopySid(copy.sid_.sid_size, sid_.sid, copy.sid_.sid);

        sid_.init = copy.sid_.init;
        sid_.sid_size = copy.sid_.sid_size;

        token_.privs = new TOKEN_PRIVILEGES[copy.token_.privs->PrivilegeCount];
        if (token_.privs == nullptr) {
            throw std::bad_alloc();
        }

        memcpy(token_.privs, copy.token_.privs, copy.token_.privs->PrivilegeCount * sizeof(TOKEN_PRIVILEGES));

        user_name_ = copy.user_name_;
        token_.privs_names = copy.token_.privs_names;

        DuplicateTokenEx(copy.primary_token_, TOKEN_ADJUST_PRIVILEGES | TOKEN_DUPLICATE | TOKEN_QUERY,
                         NULL, SecurityImpersonation, TokenPrimary, &primary_token_);
       
    }

    return *this;

}

/// <summary>
/// Destruktor klasy System.
/// </summary>
Juche::System::System::~System() {
      
    if (ac_ != nullptr) {
        delete ac_;
    }

}

/// <summary>
/// Konstruktor kopiuj¹cy klasy System.
/// </summary>
/// <param name="copy">System&</param>
/// <returns>none</returns>
Juche::System::System::System(const System& copy) noexcept(false) {

    *this = copy;

}

/// <summary>
/// Przypisanie kopiuj¹ce klasy System.
/// </summary>
/// <param name="copy">System&</param>
/// <returns>System&</returns>
Juche::System::System& 
Juche::System::System::operator=(const System& copy) noexcept(false) {


    if (this != &copy) {

        ac_ = new Account(*(copy.ac_));  
        if (ac_ == nullptr) {
            throw std::bad_alloc();
        }

        computer_name_ = copy.computer_name_;

    }

    return *this;

}

/// <summary>
/// Pobiera nazwê lokalnego komputera.
/// </summary>
/// <returns>
/// 0 = error
/// 1 = sukcess
/// -1 = error
/// </returns>
int 
Juche::System::System::set_computername() noexcept {

    DWORD size = 128;
    int i = 1, ret = 0;
    TCHAR* buff = nullptr;

    do {

        buff = new TCHAR[(size * i) + 1];
        if (buff == nullptr) {
            return -1;
        }

        if (GetComputerName(buff, &size) == FALSE) {
            
            if (GetLastError() != ERROR_BUFFER_OVERFLOW) {
                ret = -1;
            }

        } else {
            computer_name_ = buff;
            ret = 1;
        }

        delete[] buff;

        if (++i >= MAX_ERROR) {
            return 0;
        }

    } while (ret == 0);

    return ret;

}

/// <summary>
/// W³¹cza lub wy³¹cza uprawnienia przypisane do tokena dostêpu.
/// </summary>
/// <param name="priv">Nazwa uprawnienia np. "SeUndockPrivilege".</param>
/// <returns>
/// 0 = error
/// 1 = sukces
/// -1 = error
/// </returns>
int 
Juche::System::System::set_token_privilege(const string_t& priv, DWORD status) noexcept(false) {

    LUID luid;
    if (LookupPrivilegeValue(NULL, priv.c_str(), &luid) == FALSE) {
        return -1;
    }

    TOKEN_PRIVILEGES privs;
    privs.PrivilegeCount = 1;
    privs.Privileges[0].Luid = luid;
    privs.Privileges[0].Attributes = status;

    if (!AdjustTokenPrivileges(ac_->access_token(), FALSE, &privs, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        return -1;
    }

    DWORD err = GetLastError();
    if (err != ERROR_SUCCESS) {
        return -1;
    } 
    
    if (ac_->refresh_privileges() != 1) {
        return -1;
    }

    return 1;

}

/// <summary>
/// (Wymaga uprawnieñ Administracyjnych) 
/// Pzypiuje nowe uprawnienie do konta u¿ytkonika wkazywanego przez sk³adow¹ sid_
/// (czyli konto powi¹zane z procesem). 
/// </summary>
/// <param name="priv">(Unicode) nazwa uprawnienia np. L"SeDebugPrivilege".</param>
/// <returns>
/// 1  = sukcess
/// 0  = error
/// -1 = error
/// </returns>
int 
Juche::System::Account::assign_privilege(const std::wstring& priv) noexcept {

    int ret = 1;
    LSA_HANDLE lsa_hnd;
    LSA_OBJECT_ATTRIBUTES lsa_attrib = { 0 };
    if (!NT_SUCCESS(LsaOpenPolicy(NULL, &lsa_attrib, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &lsa_hnd))) {
        return -1;
    }

    size_t priv_len = priv.length();
    if (priv_len > 0x7ffe) {
        return -1;
    }

    LSA_UNICODE_STRING lsa_priv;

    lsa_priv.Buffer = const_cast<PWSTR>(priv.c_str());
    lsa_priv.Length = static_cast<USHORT>(priv_len * sizeof(WCHAR));
    lsa_priv.MaximumLength = static_cast<USHORT>((priv_len + 1) * sizeof(WCHAR));

    if (!NT_SUCCESS(LsaAddAccountRights(lsa_hnd, &sid_, &lsa_priv, 1))) {
        ret = -1;
    }

    LsaClose(lsa_hnd);

    return ret;

}