/////////////////////////////////////////////////////////
///02/06/2022 17:56
///Modu³ pomocniczy.
///Udostêpnia funkcje konwersji, konsole debugowania,
///procedure obs³ugi wyj¹tków, wskaŸniki do funckji oraz 
///funkcje pomocnicze ...
////////////////////////////////////////////////////////

module;

#include <Windows.h>
#include <mutex>
#include <concepts>

#include "Environment.h"

#ifdef __BE_KIND__
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#endif

export module Helpers;

namespace Juche {

	namespace Helpers {

		export {

			/// <summary>
			/// Konwersja ASCII -> UNICODE
			/// </summary>
			/// <param name="a_str">ASCII string</param>
			/// <returns>
			/// WskaŸnik do UNICODE string.
			/// String jest alokowany na stercie, wiêc po tym jak przestanie byæ potrzeby 
			/// nale¿y zwolniæ zasoby przy pomocy delete[].
			/// </returns>
			WCHAR* A2U(const CHAR* a_str) noexcept(false);

			/// <summary>
			/// Konwersja UNICODE -> ASCII
			/// </summary>
			/// <param name="wide_str">UNICODE string</param>
			/// <returns>
			/// WskaŸnik do ASCII string.
			/// String jest alokowany na stercie, wiêc po tym jak przestanie byæ potrzeby 
			/// nale¿y zwolniæ zasoby przy pomocy delete[].
			/// </returns>
			CHAR* U2A(const WCHAR* wide_str) noexcept(false);

#ifdef __J_DEBUG__
			/// <summary>
			/// (Singleton) Konsola debugowania dla aplikacji GUI.
			/// </summary>
			class DebugConsole {

				enum : unsigned short { DEBUG_OUTPUT_SIZE = 512 * 2 };

				std::mutex prot;

				static DebugConsole* instance_;

				/// <summary>
				/// Singleton
				/// </summary>
				DebugConsole() : prot() {};
				DebugConsole(DebugConsole&&) = delete;
				DebugConsole(const DebugConsole&) = delete;
				DebugConsole& operator=(const DebugConsole&) = delete;
				DebugConsole& operator=(DebugConsole&&) = delete;

			public:
				static DebugConsole* instance() noexcept(false);

				DWORD write(const TCHAR* format, ...) noexcept(false);

				~DebugConsole();

			};

			DebugConsole* DebugConsole::instance_ = nullptr;

#endif
			/// <summary>
			/// Bo mi siê czasami nie chce. 
			/// (funkcja zostanie usuniêta w bli¿ej nieokreœlonej przysz³oœci).
			/// </summary>
			/// <typeparam name="...Args">Zmienna lista parametrów szablonu.</typeparam>
			/// <param name="format">Ci¹gi formatuj¹ce</param>
			/// <param name="...args">Zmienna lista parametrów</param>
			/// <returns>0 || iloœæ wys³anych do konsoli bajtów</returns>
			template<typename... Args>
			inline DWORD Console(const TCHAR* format, Args... args) noexcept(false) {
#ifdef __J_DEBUG__
				DebugConsole* dbg__ = DebugConsole::instance();

				return dbg__->write(format, args...);
#else
				return 0;
#endif
			}

#ifdef __BE_KIND__
			enum : size_t { MAX_EXCEPTION_JOURNAL_SIZE = 4096 * 1024 };
			extern const std::string EXCEPTION_JOURNAL("process.log");
#endif
			/// <summary>
			/// Koncepcja szablonu.
			/// Typ musi posiadaæ metodê what(), która zwraca wartoœæ typu const char* lub wartoœæ 
			/// która mo¿e byæ rzutowana na const char*.
			/// </summary>
			template<typename T>
			concept valid_exception_type = requires (T t) {
				{t.what()} -> std::convertible_to<const char*>;
			};

			/// <summary>
			/// Procedura obs³ugi wyj¹tków, 
			/// zale¿nie od tego czy zdefiniowany zosta³ identyfikator praprocesora __BE_KIND__ 
			/// tworzy prosty plik logu lub nie tworzy.
			/// </summary>
			/// <typeparam name="T">T std::exception lub pochodny std::exception.</typeparam>
			/// <param name="exception">Sta³a referencja do std::exception lub typu pochodnego.</param>
			/// <returns>Procedura nie wraca, wywo³uje exit()</returns>
			template<typename T>
			requires valid_exception_type<T>
			[[ noreturn ]]
			void exception_handler(const T& exception) noexcept {

#ifdef __BE_KIND__

				try {

					struct tm timeinfo;
					std::stringstream datestream;

					std::time_t in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					localtime_s(&timeinfo, &in_time_t);
					datestream << std::put_time(&timeinfo, "%d-%m-%Y_%H_%M_%S");

					namespace fs = std::filesystem;

					if (fs::exists(EXCEPTION_JOURNAL)) {
						if (fs::file_size(EXCEPTION_JOURNAL) >= MAX_EXCEPTION_JOURNAL_SIZE) {
							fs::rename(EXCEPTION_JOURNAL, EXCEPTION_JOURNAL + "." + datestream.str());
						}
					}

					std::string message = "[" + datestream.str() + "] " + exception.what() + "\r\n";

					std::ofstream journal(EXCEPTION_JOURNAL, std::ios_base::app);
					if (journal.is_open()) {       /*gorzka prawda o wyj¹tkach*/
						journal << message;
						journal.close();
					}

				}
				catch (...) {
				
				}
#else 
				UNREFERENCED_PARAMETER(excepion);
#endif
				exit(-1);

			}


		}

	}

	/// <summary>
	/// WskaŸniki do funckji i procedur oraz funckja pomocnicza 
	/// do inicjalizacji wskaŸnika.
	/// </summary>
	namespace Api {

		export { 

			/// <summary>
			/// WkaŸniki do funkcji.
			/// </summary>
			using NtCompareTokens = NTSTATUS(NTAPI*)(HANDLE, HANDLE, PBOOLEAN);

			/// <summary>
			/// Inicjalizacja wskaŸnika do funckji.
			/// </summary>
			/// <param name="lib">Nazwa lub œcie¿ka biblioteki dll lub pliku exe</param>
			/// <param name="proc">Nazwa procedury</param>
			/// <param name="present">Modu³ obency w pamiêci procesu lub nie obency</param>
			/// <returns>Adres szukanej funkcji/procedury</returns>
			FARPROC function(const string_t& lib, const string_t& proc, bool present = false) noexcept(false) {

				HMODULE mod = (present == true) ? GetModuleHandle(lib.c_str()) : LoadLibrary(lib.c_str());
				if (mod == nullptr) {
					return nullptr;
				}

#ifndef UNICODE
				std::string ascii_proc = proc;
#else
				char* converted_str = Juche::Helpers::U2A(proc.c_str());
				if (converted_str == nullptr) {
					throw std::runtime_error("Juche::Helpers::Api::function()->U2A FAIL");
				}
				std::string ascii_proc = converted_str;
#endif
				FARPROC ret = GetProcAddress(mod, ascii_proc.c_str());

#ifdef UNICODE
				delete[] converted_str;
#endif

				return ret;

			}

		}

	}

}