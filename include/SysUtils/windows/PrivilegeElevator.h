/* @file Класс, повышающий привилегии для определенной операции Windows . */

#pragma once

// windows
#include <psapi.h>
#include <windows.h>

//--------------------------------------------------------------------------------
/// Класс для повышения привилегий для определенной операции Windows.
class PrivilegeElevator {
    ::HANDLE m_HandleToken;
    ::TOKEN_PRIVILEGES m_TokenPrivileges;
    int m_Result;

public:
    /// Конструктор, повышающий указанную привилегию.
    PrivilegeElevator(LPCTSTR aPrivilegeName) : m_Result(ERROR_NOT_ALL_ASSIGNED) {
        // Get a token for this process.
        if (::OpenProcessToken(
                ::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &m_HandleToken)) {
            if (::LookupPrivilegeValue(NULL, aPrivilegeName, &m_TokenPrivileges.Privileges[0].Luid)) {
                // Get the LUID for the privilege.
                m_TokenPrivileges.PrivilegeCount = 1; // one privilege to set
                m_TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                m_Result =
                    ::AdjustTokenPrivileges(m_HandleToken, FALSE, &m_TokenPrivileges, 0, (PTOKEN_PRIVILEGES)NULL, 0);
                // Get the privilege for this process.
            }
        }
    }

    /// Деструктор, закрывающий дескриптор токена процесса.
    ~PrivilegeElevator() { ::CloseHandle(m_HandleToken); }

    /// Проверяет, успешно ли повышение привилегии.
    bool OK() const { return m_Result == ERROR_SUCCESS; }
};

//--------------------------------------------------------------------------------
