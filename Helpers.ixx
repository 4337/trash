/////////////////////////////////////////////////////////
///04/06/2022 10:13
///Modu³ pomocniczy.
///Udostêpnia funkcje konwersji, konsole debugowania,
///procedure obs³ugi wyj¹tków, wskaŸniki do funckji oraz 
///funkcje pomocnicze ...
////////////////////////////////////////////////////////

module;

#include <Windows.h>
#include <mutex>
#include <concepts>
#include <unordered_map>

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
			/// Koncpecja szablonu.
			/// (trzeba poprawiæ, ale mi siê nie chce).
			/// </summary>
			template< class T >
			concept valid_range = requires(T & t) {
				t.begin(); 
				t.end();
			};

			/// <summary>
			/// Sprawdza czy wartoœæ typu V 
			/// znajduje siê w kolekcji  typu T.
			/// </summary>
			/// <typeparam name="T">Typ kolekcji/kontenera.</typeparam>
			/// <typeparam name="V">Typ wartoœci.</typeparam>
			/// <param name="collection">Referencja do przeszukiwanego kontenera.</param>
			/// <param name="val">Szukana wartoœæ.</param>
			/// <returns>
			/// true = wartoœæ znajduje siê w kolekcji.
			/// false = wartoœæ nie znajduje siê w kolekcji.
			/// </returns>
			template
			<typename T, typename V>
			requires valid_range<T>
			bool in_collection(const T& collection, V val) noexcept {  

				for (const auto& item : collection) {
					if (item == val) {
						return true;
					}
				 }
				return false;

			}
  
			/// <summary>
			/// Wersja dla tablic asocjacyjnych.
			/// (W trakcie konkretyzacji liczba parametrów mo¿e siê niez gadzaæ eg:
			/// -------
			/// std::map ma 4 parametry (2 domyœlne)
			/// template<class Key,class T,
		    /// class Compare = std::less<Key>,
		    /// class Allocator = std::allocator<std::pair<const Key, T> >
            /// > class ma
			/// -------
			/// Wy zy wi¹zku z czym mo¿e zaitnieæ potrzeba definiowania aliasów eg:
			/// template<typename key, typename value> using mymap = map<key, value>;
			/// ...
			/// if(in_collection<mymap,string, HANDLE>(libs, "stefan.dll"))
			/// ...
			/// lub uskuteczniania jakieœ innej gimnastyki.
			/// </summary>
			/// <typeparam name="TK">Typ klucza tablicy.</typeparam>
			/// <typeparam name="TV">Typ wartoœci powi¹zanej z kluczem.</typeparam>
			/// <param name="collection">Tablica asocjacyjna któr¹ przesukujemy.</param>
			/// <param name="key">Klucz którego szukamy w tablicy.</param>
			/// <returns>
			/// true = wartoœæ znajduje siê w kolekcji.
			/// false = wartoœæ nie znajduje siê w kolekcji.
			/// </returns>
			template
			< 
			 template<typename, typename> class ASSOC_ARRAY, 
			 typename TK, 
			 typename TV 
			>
			requires valid_range<ASSOC_ARRAY>
			bool in_collection(const ASSOC_ARRAY<TK, TV>& collection, TK key) noexcept {
				 
				for (const auto& item : collection) {
					if (item.first == key) {
						return true;
					}
				}
				return false;

			}

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

				/// 
				/// //////////
				/// 

				class Api { 

					static unsigned char ref_counter;
					  
					static std::unordered_map<string_t, HMODULE> libs;
					static std::unordered_map<std::string, FARPROC> procs;

					Api(Api&&) = delete;
					Api(const Api&) = delete;
					Api& operator=(const Api&) = delete;
					Api& operator=(Api&&) = delete;

					/// <summary>
					/// Helpers.
					/// </summary>
					
					static void release_libs() noexcept;

					static HMODULE in_libs(const string_t& lib) noexcept;
					static FARPROC in_procs(const std::string& proc) noexcept;

					static inline 
				    void inc_ref() noexcept {
						++Api::ref_counter;
					}

					static inline 
					void dec_ref() noexcept {
						--Api::ref_counter;
					}

				public:

					Api() {
						Api::inc_ref();
					}

					static void free() noexcept {
						if (Api::ref_counter == 1) {
							Api::release_libs();
							Api::procs.erase(Api::procs.begin(), Api::procs.end());
						}
					}

					/// <summary>
					/// NtCompareTokens NtCmpTokens = reinterpret_cast<Juche::Helpers::Api::NtCompareTokens>
					///                               (Api::function("ntdll.dll", "NtCompareTokens"));
					/// </summary>
					/// <param name="lib">Nazwa biblioteki.</param>
					/// <param name="proc">Nazwa procedury/funkcji.</param>
					/// <returns>Adres procedury/funkcji lub nullptr.</returns>
					static FARPROC function(const std::string& lib, const std::string& proc) noexcept;

					~Api() {
						Api::free();
						Api::dec_ref();
					}

				};

				unsigned char Api::ref_counter = 0;
				std::unordered_map<std::string, HMODULE> Api::libs = {};
				std::unordered_map<std::string, FARPROC> Api::procs = {};

				/// <summary>
				/// Globalny, statyczny obiekt.
				/// Wywo³uje destruktor podczas koñczenia programu.
				/// Dziêki temu nie musimy jawnie wywo³ywaæ metody zwalniaj¹cej zasoby ("free()").
				/// </summary>
				Api API_OBJECT_CLEANING;

			}

		}

	}

}