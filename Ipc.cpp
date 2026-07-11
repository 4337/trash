module;
#include <windows.h>
#include <string>
#include <regex>

/*
 * Moja wielka pora¿ka – nie uda³o mi siê obczaiæ protoko³u komunikacji
 * za poœrednictwem potoków nazwanych w NordVPN.
 *
 * Us³uga wysy³a nazwê potoku, ale niezale¿nie od tego, w jaki sposób próbowa³em
 * siê z ni¹ po³¹czyæ – jako u¿ytkownik, administrator, SYSTEM, us³uga,
 * podpisana aplikacja czy aplikacja, która odbiera wszystkim poza sob¹
 * mo¿liwoœæ dostêpu do procesu – próba po³¹czenia zawsze zwraca³a
 * kod 0x02 (Path Not Found).
 *
 * Oczywiœcie du¿¹ czêœæ tych us³ug mo¿na zdekompilowaæ, poniewa¿ s¹ napisane
 * w .NET, i w³aœnie dlatego to wielka pora¿ka.
 *
 * Mo¿e kiedyœ Nord mi kiedyœ powie, ¿ebym nie by³ smutny.
 *
 * *****************
 * Synchroniczny klient-serwer, którego potrzebowa³em
 * *****************
 */


module Ipc;

/*
* evil things, because why not
*/
DWORD Nipc::Ipc::impersonation() {

    if (handler == INVALID_HANDLE_VALUE || handler == nullptr) {
        return ERROR_INVALID_HANDLE; 
    }

    if (!ImpersonateNamedPipeClient(handler)) {
        return GetLastError();
    }

    char user_name[512 + 1] = { 0 };
    DWORD username_len = sizeof(user_name);
    if (GetUserNameA(user_name, &username_len)) {
        printf("[!!!]. mpersonation succeeded. Current thread user: %s\r\n", user_name);
    }

    return ERROR_SUCCESS; 
}

bool Nipc::Ipc::server(const std::string& pipe_name, DWORD max_instances) {

    if (handler != INVALID_HANDLE_VALUE) {
        return false;
    }

    if (pipe_name.empty()) {
        return false;
    }

    SECURITY_DESCRIPTOR sd;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        return false;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE)) {
        return false;
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
        1024 * 4,                             
        1024 * 4,                               
        0,                                      
        &sa                                     
    );

    if (handler == INVALID_HANDLE_VALUE) {
        return false;
    }

    BOOL connected = ConnectNamedPipe(handler, nullptr);

    if (!connected) {
        DWORD last_error = GetLastError();
        if (last_error == ERROR_PIPE_CONNECTED) {
            return true;
        }

        CloseHandle(handler);
        handler = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

DWORD Nipc::Ipc::client(const std::string& pipe_name, DWORD access_mode, const DWORD timeout) {

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
            printf("OK pipe %s\r\n", pipe_name.c_str());
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
    printf("DATA size write %d\r\n", data.size());
    WriteFile(handler, data.data(), static_cast<DWORD>(data.size()), &writed_bytes, nullptr);
    return static_cast<size_t>(writed_bytes);
}

template <typename T>
std::string Nipc::Ipc::create_pipename(const T& json) {
    std::string json_str;
    std::string pipe_name;

    if constexpr (std::is_same_v<T, std::string>) {
        json_str = json;
    }
    else if constexpr (std::is_same_v<T, binary_string>) {
        if (!json.empty()) {
            json_str.assign(reinterpret_cast<const char*>(json.data()), json.size());
        }
    }

    if (json_str.empty()) {
        return std::string("");
    }

    std::regex pattern("\\\"(?:Name|PrivatePipeNames)\\\":\\[?\\\"([^\\\"]+)\\\"");
    std::smatch matches;

    if (std::regex_search(json_str, matches, pattern)) {

         std::string result = matches.str(1); 
         result = std::regex_replace(result, std::regex("\\\\{2}"), "\\");

         pipe_name = "\\\\.\\pipe\\";
         pipe_name += result;

    }

    return pipe_name;
}

template size_t Nipc::Ipc::write<std::string>(const std::string&);
template size_t Nipc::Ipc::write<binary_string>(const binary_string&);

template std::string Nipc::Ipc::create_pipename<std::string>(const std::string&);
template std::string Nipc::Ipc::create_pipename<binary_string>(const binary_string&);

template std::string Nipc::Ipc::read(size_t);
template binary_string Nipc::Ipc::read(size_t);