//////////////////////////////////////////////////////////////
///23/05/2022 20:50
///Modu� udostepnia informacje o systemie operacyjnym,
///aktualnie wykorzystywanym koncie u�ytkownika oraz 
///udost�pnia interfejs do manipulowania podstawowym tokenem 
///dost�powym, a tak�e do dodawania nowy uprawnie� do aktualnie 
///wykorzystywanego konta u�tkownika - je�li dysponujemy prawami 
///administratora. 
/////////////////////////////////////////////////////////////

module;

#include <Windows.h>
#include <vector>
#include <string>

#include "Environment.h"

export module System;

namespace Juche {

	namespace System {

		export {

			enum : unsigned char {
				MAX_ERROR = 4
			};

			/// <summary>
			/// Helpers.
			/// </summary>

			inline SYSTEM_INFO info() noexcept {
				SYSTEM_INFO ret;
				GetSystemInfo(&ret);
				return ret;
			}

			inline bool is_64bit(USHORT in) noexcept {

				return (in == IMAGE_FILE_MACHINE_IA64 || in == IMAGE_FILE_MACHINE_AMD64 || 
					    in == PROCESSOR_ARCHITECTURE_AMD64 || in == PROCESSOR_ARCHITECTURE_IA64) ? true : false;

			}

			/// <summary>
			/// Zwraca uchwyt do tokena podsatwowego kt�ry opisuje uprawnienia i to�samos� u�ytkownika procesu.
			/// Zale�nie od parametru token mo�e by� powi�zany z procesem "lokalnym" lub innymi procesami.
			/// </summary>
			/// <param name="proc">Uchwyt do procesu kt�rego token chcemy otworzy�.</param>
			/// <param name="access">
			/// Prawa dost�pu z jakimi tworzymy uchwyt.
			/// </param>
			/// <returns>Uchwyt lub NULL w przypadku b��du.</returns>
			inline HANDLE primary_token(HANDLE proc, DWORD access) noexcept {
				HANDLE token;
				if (OpenProcessToken(proc, access, &token)) {
					return token;
				}
				return NULL;
			}

			struct Privileges {
				/// <summary>
				/// privs jest/musi by� alokowane przy pomocy operatora new[] i powinno zosta� 
				/// zwolnione przy pomocy wyra�ena delete[] privs.
				/// </summary>
				TOKEN_PRIVILEGES* privs;
				std::vector<string_t> privs_names;
			};

			struct SidEx {
				bool init;
				/// <summary>
				/// Wska�nik do struktury SID, struktura jest alokowana via operator new[] 
				/// i powinna zosta� zwolniona przy pomocy wyra�enia delete[].
				/// </summary>
				SID* sid;
			};

			class Account {

				HANDLE primary_token_; 

				SidEx sid_;

				string_t user_name_; 

				Privileges token_; 
			    
				/// <summary>
				/// Helpers.
				/// </summary>
				
				/// <summary>
				/// Konwersja LUID -> (string_t) nazwy uprawnienia w postaci 
				/// ci�gu znak�w.
				/// </summary>
				/// <returns>
				/// 0 = error
				/// 1 = sukces
				/// -1 = error
				/// </returns>
				int luid_2_privs() noexcept;

				bool set_username() noexcept;

				int set_token() noexcept;

				bool set_sid() noexcept;

			public:

				/// <summary>
				/// (Wymaga uprawnie� Administracyjnych) 
				/// Pzypiuje nowe uprawnienie do konta u�ytkonika wkazywanego przez sk�adow� sid_
				/// (czyli konto powi�zane z procesem). 
				/// </summary>
				/// <param name="priv">(Unicode) nazwa uprawnienia np. L"SeDebugPrivilege".</param>
				/// <returns>
				/// 1  = sukcess
				/// 0  = error
				/// -1 = error
				/// </returns>
				int assign_privilege(const std::wstring& priv) noexcept;

				inline int refresh_privileges() noexcept {

					if (token_.privs != nullptr) {
						delete[] token_.privs;
						token_.privs_names.erase(token_.privs_names.begin(), token_.privs_names.end());
						return set_token();
					}
					return -1;

				}

				inline Account(const Account& copy) noexcept;

				Account& operator=(const Account& copy) noexcept(false);

				inline Account(Account&& other) noexcept(false);
				
				Account& operator=(Account&& other) noexcept(false);

				BOOL is_admin() noexcept;

				/// <summary>
				/// Getters.
				/// </summary>

				inline const HANDLE access_token() const noexcept {
					   return primary_token_;
				}

				inline const Privileges privileges() const noexcept {
					return token_;
				}

				inline const string_t user_name() const noexcept {
					return user_name_;
				}

				/// <summary>
				/// EG.
				/// const Juche::System::SidEx sid = sys.account_sid();
				/// if (sid.init) {
			    ///     LPTSTR sid_string = nullptr;
			    ///     ConvertSidToStringSid((PSID)&sid.sid, &sid_string);
			    ///     Juche::Helpers::Console(TEXT("sid : %s\r\n"), sid_string);
				/// }
				/// </summary>
				/// <returns></returns>
				inline const SidEx sid() const noexcept {
					return sid_;
				}

				/// <summary>
				/// RAII, exception safety
				/// </summary>
				/// <returns></returns>
				Account() noexcept;

				~Account();

			};

			/// <summary>
			/// Klasa System udost�pnia informacje zwi�zane z kontem u�ytkownika 
			/// kt�ry jest powi�zany z aktualnie wykonywanym procesem, nazw� komputera,
			/// itp. Klasa pozwala r�wnie� manipulowa� tokenem i uprawnieniami u�ytkownika.
			/// </summary>
			class System {
	
				 Account* ac_;

				 string_t computer_name_;

				 /// <summary>
				 /// Helpers.
				 /// </summary>
				 int set_computername() noexcept;

			public:

				System(const System& copy) noexcept(false);

				System& operator=(const System& copy) noexcept(false);

				inline System(System&& other) noexcept(false);

				System& operator=(System&& other) noexcept(false);

				//////////

				inline string_t computer_name() const noexcept {
					   return computer_name_;
				}

				/// <summary>
				/// RAII, exception safety 
				/// </summary>
				/// <returns></returns>
				System() noexcept : ac_(new Account()) {

					set_computername();

				}
				
				/// <summary>
				/// Account wrappers  
				/// </summary>

				inline BOOL AC_is_admin() noexcept {
					return ac_->is_admin();
				}

				inline const HANDLE AC_access_token() const noexcept {
					return ac_->access_token();
				}

				inline const Privileges AC_privileges() const noexcept {
					return ac_->privileges();
				}

				inline const string_t AC_username() const noexcept {
					return  ac_->user_name();
				}

				inline const SidEx AC_sid() const noexcept {
					return ac_->sid();
				}

				/// <summary>
				/// Dodaje okre�lone uprawnienie do konta u�ykownika w kontek�cie kt�rego 
				/// uruchomiony zosta� program.
				/// (Wymaga uprawnie� administratora).
				/// </summary>
				/// <param name="priv">(UNICODE) Nazwa uprawnienia np. L"SeChangeNotifyPrivilege".</param>
				/// <returns>
				/// 0 = error
				/// -1 = bardzo error
				/// 1 = sukcess
				/// </returns>
				inline int AC_assign_privilege(const std::wstring& priv) noexcept {
					return ac_->assign_privilege(priv);
				}

				////////////////
				
				~System();

				/// <summary>
				/// W��cza lub wyl�cza uprawnienia w tokenie dost�powym.
				/// </summary>

				int set_token_privilege(const string_t& priv = SE_DEBUG_NAME, DWORD status = SE_PRIVILEGE_ENABLED) noexcept(false);

			};

		};

	}

}

