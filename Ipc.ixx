module;

#include <Windows.h>
#include <aclapi.h>
#include <sddl.h>

module Process;
import Ipc; // Importujemy pełny moduł Ipc, aby widzieć definicję klasy i pole 'handler'

// Dodano brakujący modyfikator 'const' w sygnaturze
DWORD Nipc::execve_as_client(const Nipc::Ipc& ipc_obj) {
    // Przykład dostępu do prywatnego pola dzięki zaprzyjaźnieniu:
    HANDLE h = ipc_obj.handler;

    return -1;
}

bool Nipc::LockProcess() {
    HANDLE Process = GetCurrentProcess();

    /*
     DENY Admin
     DENY System
     ALLOW Owner
    */
    PCSTR Sddl = "D:(D;OICI;GA;;;BA)(D;OICI;GA;;;SY)(A;OICI;GA;;;OW)";

    PSECURITY_DESCRIPTOR PSD = NULL;

    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
        Sddl, SDDL_REVISION_1, &PSD, NULL)) {
        return FALSE;
    }

    BOOL DaclPresent = FALSE;
    BOOL DaclDefaulted = FALSE;
    PACL DDacl = NULL;

    if (!GetSecurityDescriptorDacl(PSD, &DaclPresent, &DDacl, &DaclDefaulted)) {
        LocalFree(PSD);
        return FALSE;
    }

    DWORD result = SetSecurityInfo(
        Process,
        SE_KERNEL_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL,
        NULL,
        DDacl,
        NULL
    );

    LocalFree(PSD);

    return (result == ERROR_SUCCESS);
}
