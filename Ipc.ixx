module;

#include <string>
#include <windows.h>

export module Ipc;

import Helpers;

namespace Nipc {

	export 
	class Ipc {

		HANDLE handler;

	public:

		Ipc() : handler(INVALID_HANDLE_VALUE) {
		}

		/*
		*  pipe_name
		*  access_mode
		*  timeout 
		*  -------
		*  GetLastError
		*/
		DWORD client(const std::string&, DWORD, const DWORD timeout = 20000); //do while

		/*
		* pipe_name
		* max_instances 
		*/
		DWORD server(const std::string&, DWORD max_instances = PIPE_UNLIMITED_INSTANCES);

		/*
		* evil things
		*/
		DWORD impersonation();

		/*
		  bool: true  - connection ok
		        false - connection is broken
		*/
		bool is_alive() {
			DWORD PipeState = 0;
			return GetNamedPipeHandleState(handler, &PipeState, nullptr, nullptr, nullptr, nullptr, 0) != FALSE;
		}

		void close() {
			if (handler != nullptr && handler != INVALID_HANDLE_VALUE) {
				CloseHandle(handler);
				handler = INVALID_HANDLE_VALUE;
			}
		}

		/*
		*
		*/
		size_t bytes_to_read() {

			DWORD to_read = 0;
			PeekNamedPipe(handler, nullptr, 0, nullptr, &to_read, nullptr);
			return static_cast<size_t>(to_read);

		}

		/*
		*
		*/
		template <typename T>
		std::string create_pipename(const T& json);

		/*
		*
		*/
		template <typename T>
		T read(size_t);

		/*
		*
		*/
		template <typename T>
		size_t write(const T&);

		~Ipc() {
			close();
		}

	};

}