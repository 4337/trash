module;

#include <string>
#include <windows.h>

export module Ipc;

import Helpers;

namespace Nipc {

	export
		class Ipc {

		HANDLE handler;
		const size_t HEADER_SIZE = 4;

		/*
		 * Nie udzielamy dostêpu do handler nikomu poza funkcj¹ process::execve_as_client
		 * które potrzebuje go w trybie odczytu aby uruchomiæ process z tokenem klienta.
		 */
		friend DWORD execve_as_client(const Ipc& ipc_obj);

		public:

			Ipc() : handler(INVALID_HANDLE_VALUE) {
			}

			DWORD client(const std::string&, DWORD, const DWORD timeout = 20000);
			DWORD server(const std::string&, DWORD max_instances = PIPE_UNLIMITED_INSTANCES);
			DWORD impersonation();

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

			size_t bytes_to_read() {
				DWORD to_read = 0;
				PeekNamedPipe(handler, nullptr, 0, nullptr, &to_read, nullptr);
				return static_cast<size_t>(to_read);
			}

			template<typename T>
			binary_string create_public_msg(const T&);

			template <typename T>
			std::string create_pipename(const T&);

			template <typename T>
			T read(size_t);

			template <typename T>
			size_t write(const T&);

			~Ipc() {
				close();
			}

	};

}
