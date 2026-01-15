/* @file Класс, повышающий привилегии для определенной операции Windows . */

#pragma once

// windows
#include <windows.h>
#include <psapi.h>

//--------------------------------------------------------------------------------
/// Класс для повышения привилегий для определенной операции Windows.
class PrivilegeElevator {
    ::HANDLE hToken;
    ::TOKEN_PRIVILEGES tkp;
    int result;

  public:
    /// Конструктор, повышающий указанную привилегию.
    PrivilegeElevator(LPCTSTR aPrivilegeName) : result(ERROR_NOT_ALL_ASSIGNED) {
        // Get a token for this process.
        if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
            if (::LookupPrivilegeValue(NULL, aPrivilegeName, &tkp.Privileges[0].Luid)) {
                // Get the LUID for the privilege.
                tkp.PrivilegeCount = 1; // one privilege to set
                tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                result = ::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
                // Get the privilege for this process.
            }
        }
    }

    /// Деструктор, закрывающий дескриптор токена процесса.
    ~PrivilegeElevator() {
        ::CloseHandle(hToken);
    }

    /// Проверяет, успешно ли повышение привилегии.
    bool OK() const {
        return result == ERROR_SUCCESS;
    }
};

//--------------------------------------------------------------------------------
