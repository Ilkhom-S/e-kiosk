/* @file Класс, повышающий привилегии для определенной операции Windows . */

#pragma once

// windows
#include <windows.h>
#include <psapi.h>

//--------------------------------------------------------------------------------
/**
 * @brief Class for elevating privileges for a specific Windows operation.
 *
 * This class handles the temporary elevation of a specific privilege for the
 * current process. It opens a process token, looks up the privilege, and
 * adjusts the token privileges. The privilege is enabled upon construction and
 * the token is closed in the destructor.
 */
class PrivilegeElevator {
    ::HANDLE hToken;
    ::TOKEN_PRIVILEGES tkp;
    int result;

  public:
    /**
     * @brief Constructor that elevates the specified privilege.
     *
     * Attempts to enable the given privilege for the current process.
     * The result of the operation can be checked with OK().
     *
     * @param aPrivilegeName The name of the privilege to elevate (e.g.,
     * SE_DEBUG_NAME).
     */
    PrivilegeElevator(LPCTSTR aPrivilegeName) : result(ERROR_NOT_ALL_ASSIGNED) {
        // Get a token for this process.
        if (::OpenProcessToken(::GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &hToken)) {
            if (::LookupPrivilegeValue(NULL, aPrivilegeName,
                                       &tkp.Privileges[0].Luid)) {
                // Get the LUID for the privilege.
                tkp.PrivilegeCount = 1; // one privilege to set
                tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                result = ::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                                                 (PTOKEN_PRIVILEGES)NULL, 0);
                // Get the privilege for this process.
            }
        }
    }

    /**
     * @brief Destructor that closes the process token handle.
     */
    ~PrivilegeElevator() { ::CloseHandle(hToken); }

    /**
     * @brief Checks if the privilege elevation was successful.
     *
     * @return true if the privilege was successfully enabled, false otherwise.
     */
    bool OK() const { return result == ERROR_SUCCESS; }
};

//--------------------------------------------------------------------------------
