module;

#include <windows.h> 
#include <string>
#include <regex>
#include <userenv.h>

/*
 * Moja wielka porażka – nie udało mi się obczaić protokołu komunikacji
 * za pośrednictwem potoków nazwanych w NordVPN.
 *
 * Usługa wysyła nazwę potoku, ale niezależnie od tego, w jaki sposób próbowałem
 * się z nią połączyć – jako użytkownik, administrator, SYSTEM, usługa,
 * podpisana aplikacja czy aplikacja, która odbiera wszystkim poza sobą
 * możliwość dostępu do procesu – próba połączenia zawsze zwracała
 * kod 0x02 (Path Not Found).
 *
 * Oczywiście dużą część tych usług można zdekompilować, ponieważ są napisane
 * w .NET, i właśnie dlatego to wielka porażka.
 *
 * Może kiedyś Nord mi powie, żebym nie był smutny.
 *
 * *****************
 * Synchroniczny klient-serwer
 * *****************
 */



module Ipc;

/*
* do evil things 
*/
DWORD Nipc::Ipc::impersonation() {

    if (handler == INVALID_HANDLE_VALUE || handler == nullptr) {
        return ERROR_INVALID_HANDLE; 
    }

    if (!ImpersonateNamedPipeClient(handler)) {
        return GetLastError();
    }

    DWORD username_len = 512;
    char user_name[512 + 1] = { 0 };
    if (GetUserNameA(user_name, &username_len)) {
        printf("[!!!]. Impersonation succeeded. Current thread user: %s\r\n", user_name);
    }

    return ERROR_SUCCESS; 
}

DWORD Nipc::Ipc::server(const std::string& pipe_name, DWORD max_instances) {

    if (handler != INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    if (pipe_name.empty()) {
        return 0x43374337;
    }

    SECURITY_DESCRIPTOR sd;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        return GetLastError();
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE)) {
        return GetLastError();
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    handler = CreateNamedPipe(
        pipe_name.c_str(),                      
        PIPE_ACCESS_DUPLEX,                   
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 
        max_instances,                   
        1024 << 2,                             
        1024 << 2,                               
        0,                                      
        &sa                                     
    );

    if (handler == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    BOOL connected = ConnectNamedPipe(handler, nullptr);

    if (!connected) {
        DWORD last_error = GetLastError();
        if (last_error == ERROR_PIPE_CONNECTED) {
            return NO_ERROR;
        }

        CloseHandle(handler);
        handler = INVALID_HANDLE_VALUE;
        return last_error;
    }

    return NO_ERROR;
}

DWORD Nipc::Ipc::client(const std::string& pipe_name, DWORD access_mode, const DWORD timeout) {

    if (pipe_name.empty()) {
        return 0x43374337;
    }

    if (handler != INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    while (true) {

        handler = CreateFile(pipe_name.c_str(), 
                             access_mode, 
                             0,
                             nullptr, 
                             OPEN_EXISTING,
                             0, 
                             nullptr);

        if (handler != INVALID_HANDLE_VALUE) {
            break;
        }

        if (GetLastError() != ERROR_PIPE_BUSY) {
            return GetLastError(); 
        }

        if (!WaitNamedPipe(pipe_name.c_str(), timeout)) {
            return GetLastError(); 
        }
    }

    return ERROR_SUCCESS; 

}

/*
*
*/

template <typename T>
T Nipc::Ipc::read(size_t read_size) {

    if (read_size == 0) {
        return T(); 
    }

    T buffer;
    buffer.resize(read_size);

    DWORD readed_bytes = 0;

    if (!ReadFile(handler, buffer.data(), 
                  static_cast<DWORD>(read_size), 
                  &readed_bytes, nullptr)) {
        return T(); 
    }

    buffer.resize(static_cast<size_t>(readed_bytes));

    return buffer;

}

template <typename T>
size_t Nipc::Ipc::write(const T& data) {
       
    DWORD writed_bytes = 0;
    WriteFile(handler, data.data(), static_cast<DWORD>(data.size()), &writed_bytes, nullptr);
    return static_cast<size_t>(writed_bytes);

}

/*
* msg = string||u8string||binary_string
* format (4bajty)[ROZMIAR](string)STRING
*        0x00,0x00,0x00,0x06,'"','u','b','c','"'
* //nie wiem jeszcze czy potrzebujemy innych typ�w ni� binary_string
*/
template<typename T>
binary_string Nipc::Ipc::create_public_msg(const T& msg) {

    size_t msg_len = msg.size();
    binary_string message(msg_len + HEADER_SIZE, 0);

    message[0] = static_cast<unsigned char>((msg_len >> 24) & 0xFF);
    message[1] = static_cast<unsigned char>((msg_len >> 16) & 0xFF);
    message[2] = static_cast<unsigned char>((msg_len >> 8) & 0xFF);
    message[3] = static_cast<unsigned char>(msg_len & 0xFF);

    if (msg_len > 0) {
        std::memcpy(message.data() + HEADER_SIZE, msg.data(), msg_len);
    }

    //yyyy eee a ?
    printf("HALABANCHA %zu %s\r\n", msg_len, reinterpret_cast<const char*>(message.data() + HEADER_SIZE));

    return message;
}

/*
*
*/
template <typename T>
std::string Nipc::Ipc::create_pipename(const T& msg) {

    std::string_view msg_str;
    std::string pipe_name;

    if (msg.size() > HEADER_SIZE) {
        msg_str = std::string_view(reinterpret_cast<const char*>(msg.data()) + HEADER_SIZE, 
                                   msg.size() - HEADER_SIZE);
    }

    if (msg_str.empty()) {
        return std::string("");
    }

    std::regex pattern("\\\"(?:Name|PrivatePipeNames)\\\":\\[?\\\"([^\\\"]+)\\\"");
    std::cmatch matches;

    if (std::regex_search(msg_str.data(), msg_str.data() + msg_str.size(), matches, pattern)) {

        std::string result = matches.str(1);
        result = std::regex_replace(result, std::regex("\\\\{2}"), "\\");

        pipe_name = "\\\\.\\pipe\\";
        pipe_name += result;
    }

    return pipe_name;

}

/*
* wymagana bez inline konkretyzacja : fix
*/
template binary_string Nipc::Ipc::create_public_msg(const binary_string&);
template binary_string Nipc::Ipc::create_public_msg(const std::string&);

template binary_string Nipc::Ipc::create_public_msg(const std::u8string&);

template size_t Nipc::Ipc::write<binary_string>(const binary_string&);
template size_t Nipc::Ipc::write<std::string>(const std::string&);


template size_t Nipc::Ipc::write<std::u8string>(const std::u8string&);

template std::string Nipc::Ipc::create_pipename<std::string>(const std::string&);
template std::string Nipc::Ipc::create_pipename<binary_string>(const binary_string&);

template std::string Nipc::Ipc::read(size_t);
template binary_string Nipc::Ipc::read(size_t);