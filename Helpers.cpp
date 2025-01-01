/////////////////////////////////////////////////////////
///23/04/2022 10:35
///Modu³ pomocniczy.
///Udostêpnia funkcje konwersji, konsole debugowania,
///procedure obs³ugi wyj¹tków ...
////////////////////////////////////////////////////////

#include <Windows.h>
#include <tchar.h>

#include <string>
#include <stdexcept>
#include <mutex>

#include "Environment.h"

#ifdef __J_DEBUG__
#include <shlwapi.h>
#include <cstdarg>
#endif

import Helpers;

using namespace Juche::Helpers;

/// <summary>
/// Konwersja ASCII -> UNICODE
/// </summary>
/// <param name="a_str">ASCII string</param>
/// <returns>
/// WskaŸnik do UNICODE string.
/// String jest alokowany na stercie, wiêc po tym jak przestanie byæ potrzeby 
/// nale¿y zwolniæ zasoby przy pomocy delete[].
/// </returns>
WCHAR* Juche::Helpers::A2U(const CHAR* a_str) noexcept(false) {

	int need_size = MultiByteToWideChar(CP_ACP, 0, a_str, -1, nullptr, 0);
	if (need_size <= 0) {
		return nullptr;
	}

	WCHAR* wide_str = new WCHAR[(need_size + 1)]; 
	if (wide_str == nullptr) {
		return wide_str;
	}

	ZeroMemory(wide_str, (need_size + 1) * sizeof(WCHAR));

	int res = MultiByteToWideChar(CP_ACP, 0, a_str, -1, wide_str, need_size);
	if (res == 0) {
		delete[] wide_str;
		return nullptr;
	}

	return wide_str;

}

/// <summary>
/// Konwersja UNICODE -> ASCII
/// </summary>
/// <param name="wide_str">UNICODE string</param>
/// <returns>
/// WskaŸnik do ASCII string.
/// String jest alokowany na stercie, wiêc po tym jak przestanie byæ potrzeby 
/// nale¿y zwolniæ zasoby przy pomocy delete[].
/// </returns>
CHAR* Juche::Helpers::U2A(const WCHAR* wide_str) noexcept(false) {

	  int need_size = WideCharToMultiByte(CP_ACP, 0, wide_str, -1, nullptr, 0, nullptr, nullptr); 
	  if (need_size <= 0) {
		  return nullptr;
	  }

	  CHAR* a_str = new CHAR[(need_size + 1)];
	  if (a_str == nullptr) {
		  return nullptr;
	  }

	  ZeroMemory(a_str, (need_size + 1) * sizeof(CHAR));

	  int res = WideCharToMultiByte(CP_ACP, 0, wide_str, -1, a_str, need_size, nullptr, nullptr);
	  if (res == 0) {
		  delete[] a_str;
		  return nullptr;
	  }

	  return a_str;

}

#ifdef __J_DEBUG__

/// <summary>
/// Tworzy instacje obiektu DebugConsole (Singleton).
/// </summary>
/// <returns>WskaŸnik do DebugConsole lub rzuca wyj¹tek.</returns>
DebugConsole* Juche::Helpers::DebugConsole::instance() noexcept(false) {
	  
	if (DebugConsole::instance_ == nullptr) {

		AllocConsole();
		if (AttachConsole(GetCurrentProcessId()) == FALSE) {
			DWORD err_code = GetLastError();
			if (err_code == ERROR_INVALID_HANDLE || err_code == ERROR_INVALID_PARAMETER) {
				throw std::invalid_argument("Juche::Helpers::DebugConsole::instance() AttachConsole error : " + std::to_string(err_code));
			}
		}
		DebugConsole::instance_ = new DebugConsole();

	}

	return DebugConsole::instance_;

}

/// <summary>
/// Wyœwietla wskazane wartoœci w oknie konsoli.
/// </summary>
/// <param name="format">Ci¹g formatuj¹cy.</param>
/// <param name="">Zmienna lista parametrów które maj¹ byæ wyœwietlone na ekranie konsoli.</param>
/// <returns>Iloœæ wys³anych do okna konsoli bajtów.</returns>
DWORD Juche::Helpers::DebugConsole::write(const TCHAR* format, ...) noexcept(false) {

	DWORD ret = -1;

	va_list args = nullptr;

	va_start(args, format);

	TCHAR buff[DEBUG_OUTPUT_SIZE + 1] = { 0 };

	if (wvnsprintf(buff, DEBUG_OUTPUT_SIZE, format, args) > 0) {

		std::lock_guard<std::mutex> guard(prot);

		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buff, static_cast<DWORD>(_tcslen(buff)), &ret, NULL);

	}

	va_end(args);

	return ret;

}

/// <summary>
/// Destruktor
/// </summary>
Juche::Helpers::DebugConsole::~DebugConsole() {

	if (DebugConsole::instance_ != nullptr) {
		delete DebugConsole::instance_;
	}

	FreeConsole();

}

#endif

void
Juche::Helpers::Api::Api::release_libs() noexcept {

	for (const auto& item : Juche::Helpers::Api::Api::libs) {
		if (item.second != nullptr) {
			FreeLibrary(item.second);
			Juche::Helpers::Api::Api::libs.erase(item.first);
		}
	}

}


HMODULE 
Juche::Helpers::Api::Api::module(const string_t& lib) noexcept {

	auto iter = Juche::Helpers::Api::Api::libs.find(lib);
	if (iter != Juche::Helpers::Api::Api::libs.end()) {
		return iter->second;
	}

	HMODULE mod = LoadLibrary(lib.c_str());
	if (mod == nullptr) {
		return nullptr;
	}

	Juche::Helpers::Api::Api::libs.insert(std::make_pair(lib, mod));
	return mod;

}

FARPROC 
Juche::Helpers::Api::Api::in_procs(const std::string& proc) noexcept {

	auto iter = Juche::Helpers::Api::Api::procs.find(proc);
	if (iter != Juche::Helpers::Api::Api::procs.end()) {
		return iter->second;
	}

	return nullptr;

}

FARPROC 
Juche::Helpers::Api::Api::function(const string_t& lib, const std::string& proc) noexcept {

	HMODULE mod = Juche::Helpers::Api::Api::module(lib);
	if (mod == nullptr) {
		return nullptr;
	}

	FARPROC addr = Juche::Helpers::Api::Api::in_procs(proc);
	if (addr != nullptr) {
		return addr;
	}

	addr = GetProcAddress(mod, proc.c_str());
	if (addr == nullptr) {
		return nullptr;
	}

	Juche::Helpers::Api::Api::procs.insert(std::make_pair(proc, addr));
	return addr;

}