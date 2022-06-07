/////////////////////////////////////////////////////////
///07/06/2022 10:56
///Modu³ tworzy listê procesów które sep³niaj¹ okreœlone 
///warunki wstêpne i udostêpnia interfejs do pobierania 
///informacji o poszczególnych procesach.
////////////////////////////////////////////////////////
module;

#include <Windows.h>
#include <vector>
#include <utility>

#include "Environment.h"

export module Processes;

namespace Juche {

	namespace Processes {
		
		export {

			enum class ProcessType : unsigned char {
				 x86,
				 x64,
				 WOW64,
				 ANY,
				 UNKNOW
			};

			struct ProcessArchitecture {
				bool wow64;
				ProcessType arch;
			};

			struct MitigationInfo {
				bool init;
				PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY cig;
				PROCESS_MITIGATION_DYNAMIC_CODE_POLICY acg;
			};

			///
			/// ///////////
			/// 

			class Process {

				HANDLE hnd_;  
				BOOL inherit_handle_;

				DWORD pid_;
				string_t app_path_;

				DWORD session_id_;
				ProcessArchitecture architecture_;  

				/*
				bool is_native;
				*/

				/// <summary>
				/// CIG - Code Integrity Guard
				/// ACG - Arbitrary Code Guard
				/// </summary>
				MitigationInfo mitigations_;

				/// <summary>
				/// Helpers.
				/// </summary>
				
				bool set_mitigation_info() noexcept;
				
				bool set_app_path() noexcept;
				
				void set_arch_info() noexcept; 

				int is_wow64(USHORT* native_machine = NULL) noexcept;

			public:

				Process(const Process& copy) noexcept(false);  
				Process& operator=(const Process& copy) noexcept(false);

				Process(Process&& other) noexcept(false);
				Process& operator=(Process&& other) noexcept(false);

				explicit Process(HANDLE proc = NULL, BOOL inherit = FALSE) noexcept;

				bool open(DWORD ipid, DWORD access = PROCESS_QUERY_INFORMATION, BOOL inherit = FALSE) noexcept(false);

				inline MitigationInfo mitigations() const noexcept {
					return mitigations_;
				}

				inline string_t path() const noexcept {
					return app_path_;
				}

				inline HANDLE handler() const noexcept {
					return hnd_;
				}

				inline ProcessArchitecture arch() const noexcept {
					return architecture_;
				}

				inline DWORD sessionid() const noexcept {
					return session_id_;
				}

				void close() noexcept(false);

				~Process();

			};
			
			/////////////////

			/// <summary>
			/// WskaŸnik do tablicy identyfikatorów PID uruchomionych w danej chwili procesów 
			/// oraz rozmiar tablicy.
			/// </summary>
			struct Pids {
				/// <summary>
				/// ptr jest alokowany za pomoc¹ operatora new[] i powinien 
				/// zostaæ zwolniony za pomoc¹ operatora delete[].
				/// </summary>
				DWORD* ptr;
				DWORD  size;
			};
 
			/// 
			/// //////////
			/// 
			
			class ProcessList {

				enum : size_t {
					   LIST_SIZE = 1024,
					   MAX_ERROR = 5 
				};

				std::vector<Process> list_;

                /// <summary>
                /// DWORD = Suma CRC32 nazwy wykluczonego procesu.
				/// ASCII:
				/// 0xb4e35f10 = firefox.exe 
				/// 0x9c1d0d0e = chrome.exe
				/// 0x79a3aebf = msedge.exe
				/// UNICODE:
				/// 0xXXXXXXXX = msedge.exe
				/// 0xYYYYYYYY = firefox.exe
                /// </summary>
                std::vector<DWORD> excluded_;  

				template<typename T>
				using exclusion_vector = std::vector<T>;


				/// <summary>
				/// Helpers.
				/// </summary>

				int enum_pids(Pids& info) noexcept(false);
				  
			public:

				explicit ProcessList(const std::vector<DWORD>& ex = {}) noexcept(false);

				explicit ProcessList(std::initializer_list<DWORD> ex = {}) noexcept(false);

				int snapshoot(ProcessType proc_type = ProcessType::ANY, bool ignore_cig = false) noexcept(false); 

				/// <summary>
				/// Operator indeksowania bez weryfikacji granic.
				/// </summary>
				/// <param name="index">indeks.</param>
				/// <returns>Process</returns>
				inline Process& operator[](size_t index) noexcept {
					return list_[index];
				}

				~ProcessList();

				inline const size_t size() const noexcept {
					return list_.size();
				}

				inline std::vector<Process> list() const noexcept {
					return list_;
				}

			};


		}

	}


}