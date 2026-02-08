/**********************************************************************
 *
 * StackWalker.cpp
 *
 *
 * History:
 *  2005-07-27   v1    - First public release on http://www.codeproject.com/
 *                       http://www.codeproject.com/threads/StackWalker.asp
 *  2005-07-28   v2    - Changed the params of the constructor and ShowCallstack
 *                       (to simplify the usage)
 *  2005-08-01   v3    - Changed to use 'CONTEXT_FULL' instead of CONTEXT_ALL
 *                       (should also be enough)
 *                     - Changed to compile correctly with the PSDK of VC7.0
 *                       (GetFileVersionInfoSizeA and GetFileVersionInfoA is
 *wrongly defined: it uses LPSTR instead of LPCSTR as first paremeter)
 *                     - Added declarations to support VC5/6 without using
 *'dbghelp.h'
 *                     - Added a 'pUserData' member to the ShowCallstack
 *function and the PReadProcessMemoryRoutine declaration (to pass some
 *user-defined data, which can be used in the readMemoryFunction-callback)
 *  2005-08-02   v4    - OnSym_Init now also outputs the OS-Version by default
 *                     - Added example for doing an exception-callstack-walking
 *in main.cpp (thanks to owillebo:
 *http://www.codeproject.com/script/profile/whos_who.asp?id=536268) 2005-08-05
 *v5    - Removed most Lint (http://www.gimpel.com/) errors... thanks to Okko
 *Willeboordse!
 *
 **********************************************************************/

#define NOMINMAX // HACK for QDateTime Qt 5.0.0
#include <cstdlib>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#pragma comment(lib, "version.lib") // for "VerQueryValue"

#include "StackWalker.h"

#pragma warning(disable : 4127 4740)

// If VC7 and later, then use the shipped 'dbghelp.h'-file
#if _MSC_VER >= 1300
#include <dbghelp.h>
#else
// inline the important dbghelp.h-declarations...
typedef enum {
    Sym_None = 0,
    Sym_Coff,
    Sym_Cv,
    Sym_Pdb,
    Sym_Export,
    Sym_Deferred,
    Sym_Sym,
    Sym_Dia,
    Sym_Virtual,
    Num_Sym_Types
} SYM_TYPE;
typedef struct _IMAGEHLP_LINE64 {
    DWORD SizeOfStruct; // set to sizeof(IMAGEHLP_LINE64)
    PVOID Key;          // internal
    DWORD LineNumber;   // line number in file
    PCHAR FileName;     // full filename
    DWORD64 Address;    // first instruction of line
} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;
typedef struct _IMAGEHLP_MODULE64 {
    DWORD SizeOfStruct;        // set to sizeof(IMAGEHLP_MODULE64)
    DWORD64 BaseOfImage;       // base load address of module
    DWORD ImageSize;           // virtual size of the loaded module
    DWORD TimeDateStamp;       // date/time stamp from pe header
    DWORD CheckSum;            // checksum from the pe header
    DWORD Num_Syms;            // number of symbols in the symbol table
    SYM_TYPE Sym_Type;         // type of symbols loaded
    CHAR ModuleName[32];       // module name
    CHAR ImageName[256];       // image name
    CHAR LoadedImageName[256]; // symbol file name
} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64;
typedef struct _IMAGEHLP_SYMBOL64 {
    DWORD SizeOfStruct;  // set to sizeof(IMAGEHLP_SYMBOL64)
    DWORD64 Address;     // virtual address including dll base address
    DWORD Size;          // estimated size of symbol, can be zero
    DWORD Flags;         // info about the symbols, see the SYMF defines
    DWORD MaxNameLength; // maximum size of symbol name in 'Name'
    CHAR Name[1];        // symbol name (null terminated string)
} IMAGEHLP_SYMBOL64, *PIMAGEHLP_SYMBOL64;
typedef enum { AddrMode1616, AddrMode1632, AddrModeReal, AddrModeFlat } ADDRESS_MODE;
typedef struct _tagADDRESS64 {
    DWORD64 Offset;
    WORD Segment;
    ADDRESS_MODE Mode;
} ADDRESS64, *LPADDRESS64;
typedef struct _KDHELP64 {
    DWORD64 Thread;
    DWORD ThCallbackStack;
    DWORD ThCallbackBStore;
    DWORD NextCallback;
    DWORD FramePointer;
    DWORD64 KiCallUserMode;
    DWORD64 KeUserCallbackDispatcher;
    DWORD64 System_RangeStart;
    DWORD64 Reserved[8];
} KDHELP64, *PKDHELP64;
typedef struct _tagSTACKFRAME64 {
    ADDRESS64 AddrPC;     // program counter
    ADDRESS64 AddrReturn; // return address
    ADDRESS64 AddrFrame;  // frame pointer
    ADDRESS64 AddrStack;  // stack pointer
    ADDRESS64 AddrBStore; // backing store pointer
    PVOID FuncTableEntry; // pointer to pdata/fpo or NULL
    DWORD64 Params[4];    // possible arguments to the function
    BOOL Far;             // WOW far call
    BOOL Virtual;         // is this a virtual frame?
    DWORD64 Reserved[3];
    KDHELP64 KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;
typedef BOOL(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE64)(HANDLE hProcess,
                                                        DWORD64 qwBaseAddress,
                                                        PVOID lpBuffer,
                                                        DWORD nSize,
                                                        LPDWORD lpNumberOfBytesRead);
typedef PVOID(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE64)(HANDLE hProcess, DWORD64 AddrBase);
typedef DWORD64(__stdcall *PGET_MODULE_BASE_ROUTINE64)(HANDLE hProcess, DWORD64 Address);
typedef DWORD64(__stdcall *PTRANSLATE_ADDRESS_ROUTINE64)(HANDLE hProcess,
                                                         HANDLE hThread,
                                                         LPADDRESS64 lpaddr);
#define SYMOPT_CASE_INSENSITIVE 0x00000001
#define SYMOPT_UNDNAME 0x00000002
#define SYMOPT_DEFERRED_LOADS 0x00000004
#define SYMOPT_NO_CPP 0x00000008
#define SYMOPT_LOAD_LINES 0x00000010
#define SYMOPT_OMAP_FIND_NEAREST 0x00000020
#define SYMOPT_LOAD_ANYTHING 0x00000040
#define SYMOPT_IGNORE_CVREC 0x00000080
#define SYMOPT_NO_UNQUALIFIED_LOADS 0x00000100
#define SYMOPT_FAIL_CRITICAL_ERRORS 0x00000200
#define SYMOPT_EXACT_SYMBOLS 0x00000400
#define SYMOPT_ALLOW_ABSOLUTE_SYMBOLS 0x00000800
#define SYMOPT_IGNORE_NT_SYMPATH 0x00001000
#define SYMOPT_INCLUDE_32BIT_MODULES 0x00002000
#define SYMOPT_PUBLICS_ONLY 0x00004000
#define SYMOPT_NO_PUBLICS 0x00008000
#define SYMOPT_AUTO_PUBLICS 0x00010000
#define SYMOPT_NO_IMAGE_SEARCH 0x00020000
#define SYMOPT_SECURE 0x00040000
#define SYMOPT_DEBUG 0x80000000
#define UNDNAME_COMPLETE (0x0000)  // Enable full undecoration
#define UNDNAME_NAME_ONLY (0x1000) // Crack only the name for primary declaration;
#endif                             // _MSC_VER < 1300

// Some missing defines (for VC5/6):
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif

// secure-CRT_functions are only available starting with VC8
#if _MSC_VER < 1400
#define strcpy_s strcpy
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#define _tcscat_s _tcscat
#endif

// Normally it should be enough to use 'CONTEXT_FULL' (better would be
// 'CONTEXT_ALL')
#define USED_CONTEXT_FLAGS CONTEXT_FULL

class StackWalkerInternal {
public:
    StackWalkerInternal(StackWalker *parent, HANDLE hProcess) {
        m_Parent = parent;
        m_HDbhHelp = NULL;
        pSC = NULL;
        m_HProcess = hProcess;
        m_SzSym_Path = NULL;
        pSFTA = NULL;
        pSGLFA = NULL;
        pSGMB = NULL;
        pSGMI = NULL;
        pSGO = NULL;
        pSGSFA = NULL;
        pSI = NULL;
        pSLM = NULL;
        pSSO = NULL;
        pSW = NULL;
        pUDSN = NULL;
        pSGSP = NULL;
    }
    ~StackWalkerInternal() {
        if (pSC != NULL)
            pSC(m_HProcess); // Sym_Cleanup
        if (m_HDbhHelp != NULL)
            FreeLibrary(m_HDbhHelp);
        m_HDbhHelp = NULL;
        m_Parent = NULL;
        if (m_SzSym_Path != NULL)
            free(m_SzSym_Path);
        m_SzSym_Path = NULL;
    }
    BOOL Init(LPCSTR szSym_Path) {
        if (m_Parent == NULL)
            return FALSE;
        // Dynamically load the Entry-Points for dbghelp.dll:
        // First try to load the newsest one from
        TCHAR szTemp[4096];
        // But before wqe do this, we first check if the ".local" file exists
        if (GetModuleFileName(NULL, szTemp, 4096) > 0) {
            _tcscat_s(szTemp, _T(".local"));
            if (GetFileAttributes(szTemp) == INVALID_FILE_ATTRIBUTES) {
                // ".local" file does not exist, so we can try to load the dbghelp.dll
                // from the "Debugging Tools for Windows"
                if (GetEnvironmentVariable(_T("Program_Files"), szTemp, 4096) > 0) {
                    _tcscat_s(szTemp, _T("\\Debugging Tools for Windows\\dbghelp.dll"));
                    // now check if the file exists:
                    if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES) {
                        m_HDbhHelp = LoadLibrary(szTemp);
                    }
                }
                // Still not found? Then try to load the 64-Bit version:
                if ((m_hDbhHelp == NULL) &&
                    (GetEnvironmentVariable(_T("Program_Files"), szTemp, 4096) > 0)) {
                    _tcscat_s(szTemp, _T("\\Debugging Tools for Windows 64-Bit\\dbghelp.dll"));
                    if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES) {
                        m_HDbhHelp = LoadLibrary(szTemp);
                    }
                }
            }
        }
        if (m_HDbhHelp == NULL) // if not already loaded, try to load a default-one
            m_HDbhHelp = LoadLibrary(_T("dbghelp.dll"));
        if (m_hDbhHelp == NULL)
            return FALSE;
        pSI = (tSI)GetProcAddress(m_hDbhHelp, "Sym_Initialize");
        pSC = (tSC)GetProcAddress(m_hDbhHelp, "Sym_Cleanup");

        pSW = (tSW)GetProcAddress(m_hDbhHelp, "StackWalk64");
        pSGO = (tSGO)GetProcAddress(m_hDbhHelp, "Sym_GetOptions");
        pSSO = (tSSO)GetProcAddress(m_hDbhHelp, "Sym_SetOptions");

        pSFTA = (tSFTA)GetProcAddress(m_hDbhHelp, "Sym_FunctionTableAccess64");
        pSGLFA = (tSGLFA)GetProcAddress(m_hDbhHelp, "Sym_GetLineFrom_Addr64");
        pSGMB = (tSGMB)GetProcAddress(m_hDbhHelp, "Sym_GetModuleBase64");
        pSGMI = (tSGMI)GetProcAddress(m_hDbhHelp, "Sym_GetModuleInfo64");
        // pSGMI_V3 = (tSGMI_V3) GetProcAddress(m_hDbhHelp, "Sym_GetModuleInfo64" );
        pSGSFA = (tSGSFA)GetProcAddress(m_hDbhHelp, "Sym_GetSym_From_Addr64");
        pUDSN = (tUDSN)GetProcAddress(m_hDbhHelp, "UnDecorateSymbolName");
        pSLM = (tSLM)GetProcAddress(m_hDbhHelp, "Sym_LoadModule64");
        pSGSP = (tSGSP)GetProcAddress(m_hDbhHelp, "Sym_GetSearchPath");

        if (pSC == NULL || pSFTA == NULL || pSGMB == NULL || pSGMI == NULL || pSGO == NULL ||
            pSGSFA == NULL || pSI == NULL || pSSO == NULL || pSW == NULL || pUDSN == NULL ||
            pSLM == NULL) {
            FreeLibrary(m_HDbhHelp);
            m_HDbhHelp = NULL;
            pSC = NULL;
            return FALSE;
        }

        // Sym_Initialize
        if (szSym_Path != NULL)
            m_SzSym_Path = _strdup(szSym_Path);
        if (this->pSI(m_HProcess, m_SzSym_Path, FALSE) == FALSE)
            this->m_parent->OnDbgHelpErr("Sym_Initialize", GetLastError(), 0);

        DWORD sym_Options = this->pSGO(); // Sym_GetOptions
        sym_Options |= SYMOPT_LOAD_LINES;
        sym_Options |= SYMOPT_FAIL_CRITICAL_ERRORS;
        // sym_Options |= SYMOPT_NO_PROMPTS;
        //  Sym_SetOptions
        sym_Options = this->pSSO(sym_Options);

        char buf[StackWalker::STACKWALK_MAX_NAMELEN] = {0};
        if (this->pSGSP != NULL) {
            if (this->pSGSP(m_HProcess, buf, StackWalker::STACKWALK_MAX_NAMELEN) == FALSE)
                this->m_parent->OnDbgHelpErr("Sym_GetSearchPath", GetLastError(), 0);
        }
        char szUserName[1024] = {0};
        DWORD dwSize = 1024;
        GetUserNameA(szUserName, &dwSize);
        this->m_parent->OnSym_Init(buf, sym_Options, szUserName);

        return TRUE;
    }

    StackWalker *m_parent;

    HMODULE m_hDbhHelp;
    HANDLE m_hProcess;
    LPSTR m_szSym_Path;

    /*typedef struct IMAGEHLP_MODULE64_V3 {
            DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
            DWORD64  BaseOfImage;            // base load address of module
            DWORD    ImageSize;              // virtual size of the loaded module
            DWORD    TimeDateStamp;          // date/time stamp from pe header
            DWORD    CheckSum;               // checksum from the pe header
            DWORD    Num_Syms;                // number of symbols in the symbol
    table SYM_TYPE Sym_Type;                // type of symbols loaded CHAR
    ModuleName[32];         // module name CHAR     ImageName[256];         //
    image name
            // new elements: 07-Jun-2002
            CHAR     LoadedImageName[256];   // symbol file name
            CHAR     LoadedPdbName[256];     // pdb file name
            DWORD    CVSig;                  // Signature of the CV record in the
    debug directories CHAR         CVData[MAX_PATH * 3];   // Contents of the CV
    record DWORD    PdbSig;                 // Signature of PDB GUID     PdbSig70;
    // Signature of PDB (VC 7 and up) DWORD    PdbAge;                 // DBI age
    of pdb BOOL     PdbUnmatched;           // loaded an unmatched pdb BOOL
    DbgUnmatched;           // loaded an unmatched dbg BOOL     LineNumbers; // we
    have line number information BOOL     GlobalSymbols;          // we have
    internal symbol information BOOL     TypeInfo;               // we have type
    information
            // new elements: 17-Dec-2003
            BOOL     SourceIndexed;          // pdb supports source server
            BOOL     Publics;                // contains public symbols
    };
    */
    struct IMAGEHLP_MODULE64_V2 {
        DWORD SizeOfStruct;        // set to sizeof(IMAGEHLP_MODULE64)
        DWORD64 BaseOfImage;       // base load address of module
        DWORD ImageSize;           // virtual size of the loaded module
        DWORD TimeDateStamp;       // date/time stamp from pe header
        DWORD CheckSum;            // checksum from the pe header
        DWORD Num_Syms;            // number of symbols in the symbol table
        SYM_TYPE Sym_Type;         // type of symbols loaded
        CHAR ModuleName[32];       // module name
        CHAR ImageName[256];       // image name
        CHAR LoadedImageName[256]; // symbol file name
    };

    // Sym_Cleanup()
    typedef BOOL(__stdcall *tSC)(IN HANDLE hProcess);
    tSC pSC;

    // Sym_FunctionTableAccess64()
    typedef PVOID(__stdcall *tSFTA)(HANDLE hProcess, DWORD64 AddrBase);
    tSFTA pSFTA;

    // Sym_GetLineFrom_Addr64()
    typedef BOOL(__stdcall *tSGLFA)(IN HANDLE hProcess,
                                    IN DWORD64 dwAddr,
                                    OUT PDWORD pdwDisplacement,
                                    OUT PIMAGEHLP_LINE64 Line);
    tSGLFA pSGLFA;

    // Sym_GetModuleBase64()
    typedef DWORD64(__stdcall *tSGMB)(IN HANDLE hProcess, IN DWORD64 dwAddr);
    tSGMB pSGMB;

    // Sym_GetModuleInfo64()
    typedef BOOL(__stdcall *tSGMI)(IN HANDLE hProcess,
                                   IN DWORD64 dwAddr,
                                   OUT IMAGEHLP_MODULE64_V2 *ModuleInfo);
    tSGMI pSGMI;

    //  // Sym_GetModuleInfo64()
    //  typedef BOOL (__stdcall *tSGMI_V3)( IN HANDLE hProcess, IN DWORD64 dwAddr,
    //  OUT IMAGEHLP_MODULE64_V3 *ModuleInfo
    //  ); tSGMI_V3 pSGMI_V3;

    // Sym_GetOptions()
    typedef DWORD(__stdcall *tSGO)(VOID);
    tSGO pSGO;

    // Sym_GetSym_From_Addr64()
    typedef BOOL(__stdcall *tSGSFA)(IN HANDLE hProcess,
                                    IN DWORD64 dwAddr,
                                    OUT PDWORD64 pdwDisplacement,
                                    OUT PIMAGEHLP_SYMBOL64 Symbol);
    tSGSFA pSGSFA;

    // Sym_Initialize()
    typedef BOOL(__stdcall *tSI)(IN HANDLE hProcess,
                                 IN PSTR UserSearchPath,
                                 IN BOOL fInvadeProcess);
    tSI pSI;

    // Sym_LoadModule64()
    typedef DWORD64(__stdcall *tSLM)(IN HANDLE hProcess,
                                     IN HANDLE hFile,
                                     IN PSTR ImageName,
                                     IN PSTR ModuleName,
                                     IN DWORD64 BaseOfDll,
                                     IN DWORD SizeOfDll);
    tSLM pSLM;

    // Sym_SetOptions()
    typedef DWORD(__stdcall *tSSO)(IN DWORD Sym_Options);
    tSSO pSSO;

    // StackWalk64()
    typedef BOOL(__stdcall *tSW)(DWORD MachineType,
                                 HANDLE hProcess,
                                 HANDLE hThread,
                                 LPSTACKFRAME64 StackFrame,
                                 PVOID ContextRecord,
                                 PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                                 PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                                 PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                                 PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
    tSW pSW;

    // UnDecorateSymbolName()
    typedef DWORD(__stdcall WINAPI *tUDSN)(PCSTR DecoratedName,
                                           PSTR UnDecoratedName,
                                           DWORD UndecoratedLength,
                                           DWORD Flags);
    tUDSN pUDSN;

    typedef BOOL(__stdcall WINAPI *tSGSP)(HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength);
    tSGSP pSGSP;

private:
// **************************************** ToolHelp32 ************************
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE 0x00000008
#pragma pack(push, 8)
    typedef struct tagMODULEENTRY32 {
        DWORD dwSize;
        DWORD th32ModuleID;  // This module
        DWORD th32ProcessID; // owning process
        DWORD GlblcntUsage;  // Global usage count on the module
        DWORD ProccntUsage;  // Module usage count in th32ProcessID's context
        BYTE *modBaseAddr;   // Base address of module in th32ProcessID's context
        DWORD modBaseSize;   // Size in bytes of module starting at modBaseAddr
        HMODULE hModule;     // The hModule of this module in th32ProcessID's context
        char szModule[MAX_MODULE_NAME32 + 1];
        char szExePath[MAX_PATH];
    } MODULEENTRY32;
    typedef MODULEENTRY32 *PMODULEENTRY32;
    typedef MODULEENTRY32 *LPMODULEENTRY32;
#pragma pack(pop)

    BOOL GetModuleListTH32(HANDLE hProcess, DWORD pid) {
        // CreateToolhelp32Snapshot()
        typedef HANDLE(__stdcall * tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
        // Module32First()
        typedef BOOL(__stdcall * tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
        // Module32Next()
        typedef BOOL(__stdcall * tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

        // try both dlls...
        const TCHAR *dllname[] = {_T("kernel32.dll"), _T("tlhelp32.dll")};
        HINSTANCE hToolhelp = NULL;
        tCT32S pCT32S = NULL;
        tM32F pM32F = NULL;
        tM32N pM32N = NULL;

        HANDLE hSnap;
        MODULEENTRY32 me;
        me.dwSize = sizeof(me);
        BOOL keepGoing;
        size_t i;

        for (i = 0; i < (sizeof(dllname) / sizeof(dllname[0])); i++) {
            hToolhelp = LoadLibrary(dllname[i]);
            if (hToolhelp == NULL)
                continue;
            pCT32S = (tCT32S)GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
            pM32F = (tM32F)GetProcAddress(hToolhelp, "Module32First");
            pM32N = (tM32N)GetProcAddress(hToolhelp, "Module32Next");
            if ((pCT32S != NULL) && (pM32F != NULL) && (pM32N != NULL))
                break; // found the functions!
            FreeLibrary(hToolhelp);
            hToolhelp = NULL;
        }

        if (hToolhelp == NULL)
            return FALSE;

        hSnap = pCT32S(TH32CS_SNAPMODULE, pid);
        if (hSnap == (HANDLE)-1)
            return FALSE;

        keepGoing = !!pM32F(hSnap, &me);
        int cnt = 0;
        while (keepGoing) {
            this->LoadModule(
                hProcess, me.szExePath, me.szModule, (DWORD64)me.modBaseAddr, me.modBaseSize);
            cnt++;
            keepGoing = !!pM32N(hSnap, &me);
        }
        CloseHandle(hSnap);
        FreeLibrary(hToolhelp);
        if (cnt <= 0)
            return FALSE;
        return TRUE;
    } // GetModuleListTH32

    // **************************************** PSAPI ************************
    typedef struct _MODULEINFO {
        LPVOID lpBaseOfDll;
        DWORD SizeOfImage;
        LPVOID EntryPoint;
    } MODULEINFO, *LPMODULEINFO;

    BOOL GetModuleListPSAPI(HANDLE hProcess) {
        // Enum_ProcessModules()
        typedef BOOL(__stdcall *
                     tEPM)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
        // GetModuleFileNameEx()
        typedef DWORD(__stdcall *
                      tGMFNE)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
        // GetModuleBaseName()
        typedef DWORD(__stdcall *
                      tGMBN)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
        // GetModuleInformation()
        typedef BOOL(__stdcall *
                     tGMI)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize);

        HINSTANCE hPsapi;
        tEPM pEPM;
        tGMFNE pGMFNE;
        tGMBN pGMBN;
        tGMI pGMI;

        DWORD i;
        // ModuleEntry e;
        DWORD cbNeeded;
        MODULEINFO mi;
        HMODULE *hMods = 0;
        char *tt = NULL;
        char *tt2 = NULL;
        const SIZE_T TTBUFLEN = 8096;
        int cnt = 0;

        hPsapi = LoadLibrary(_T("psapi.dll"));
        if (hPsapi == NULL)
            return FALSE;

        pEPM = (tEPM)GetProcAddress(hPsapi, "Enum_ProcessModules");
        pGMFNE = (tGMFNE)GetProcAddress(hPsapi, "GetModuleFileNameExA");
        pGMBN = (tGMFNE)GetProcAddress(hPsapi, "GetModuleBaseNameA");
        pGMI = (tGMI)GetProcAddress(hPsapi, "GetModuleInformation");
        if ((pEPM == NULL) || (pGMFNE == NULL) || (pGMBN == NULL) || (pGMI == NULL)) {
            // we couldnґt find all functions
            FreeLibrary(hPsapi);
            return FALSE;
        }

        hMods = (HMODULE *)malloc(sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
        tt = (char *)malloc(sizeof(char) * TTBUFLEN);
        tt2 = (char *)malloc(sizeof(char) * TTBUFLEN);
        if ((hMods == NULL) || (tt == NULL) || (tt2 == NULL))
            goto cleanup;

        if (!pEPM(hProcess, hMods, TTBUFLEN, &cbNeeded)) {
            //_ftprintf(fLogFile, _T("%lu: EPM failed, GetLastError = %lu\n"),
            // g_dwShowCount, gle );
            goto cleanup;
        }

        if (cbNeeded > TTBUFLEN) {
            //_ftprintf(fLogFile, _T("%lu: More than %lu module handles. Huh?\n"),
            // g_dwShowCount, lenof( hMods ) );
            goto cleanup;
        }

        for (i = 0; i < cbNeeded / sizeof hMods[0]; i++) {
            // base address, size
            pGMI(hProcess, hMods[i], &mi, sizeof mi);
            // image file name
            tt[0] = 0;
            pGMFNE(hProcess, hMods[i], tt, TTBUFLEN);
            // module name
            tt2[0] = 0;
            pGMBN(hProcess, hMods[i], tt2, TTBUFLEN);

            DWORD dwRes =
                this->LoadModule(hProcess, tt, tt2, (DWORD64)mi.lpBaseOfDll, mi.SizeOfImage);
            if (dwRes != ERROR_SUCCESS)
                this->m_parent->OnDbgHelpErr("LoadModule", dwRes, 0);
            cnt++;
        }

    cleanup:
        if (hPsapi != NULL)
            FreeLibrary(hPsapi);
        if (tt2 != NULL)
            free(tt2);
        if (tt != NULL)
            free(tt);
        if (hMods != NULL)
            free(hMods);

        return cnt != 0;
    } // GetModuleListPSAPI

    DWORD LoadModule(HANDLE hProcess, LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size) {
        CHAR *szImg = _strdup(img);
        CHAR *szMod = _strdup(mod);
        DWORD result = ERROR_SUCCESS;
        if ((szImg == NULL) || (szMod == NULL))
            result = ERROR_NOT_ENOUGH_MEMORY;
        else {
            if (pSLM(hProcess, 0, szImg, szMod, baseAddr, size) == 0)
                result = GetLastError();
        }
        ULONGLONG fileVersion = 0;
        if ((m_parent != NULL) && (szImg != NULL)) {
            // try to retrive the file-version:
            if ((this->m_Parent->m_Options & StackWalker::RetrieveFileVersion) != 0) {
                VS_FIXEDFILEINFO *fInfo = NULL;
                DWORD dwHandle;
                DWORD dwSize = GetFileVersionInfoSizeA(szImg, &dwHandle);
                if (dwSize > 0) {
                    LPVOID vData = malloc(dwSize);
                    if (vData != NULL) {
                        if (GetFileVersionInfoA(szImg, dwHandle, dwSize, vData) != 0) {
                            UINT len;
                            TCHAR szSubBlock[] = _T("\\");
                            if (VerQueryValue(vData, szSubBlock, (LPVOID *)&fInfo, &len) == 0)
                                fInfo = NULL;
                            else {
                                fileVersion = ((ULONGLONG)fInfo->dwFileVersionLS) +
                                              ((ULONGLONG)fInfo->dwFileVersionMS << 32);
                            }
                        }
                        free(vData);
                    }
                }
            }

            // Retrive some additional-infos about the module
            IMAGEHLP_MODULE64_V2 Module;
            const char *szSym_Type = "-unknown-";
            if (this->GetModuleInfo(hProcess, baseAddr, &Module) != FALSE) {
                switch (Module.Sym_Type) {
                case Sym_None:
                    szSym_Type = "-nosymbols-";
                    break;
                case Sym_Coff:
                    szSym_Type = "COFF";
                    break;
                case Sym_Cv:
                    szSym_Type = "CV";
                    break;
                case Sym_Pdb:
                    szSym_Type = "PDB";
                    break;
                case Sym_Export:
                    szSym_Type = "-exported-";
                    break;
                case Sym_Deferred:
                    szSym_Type = "-deferred-";
                    break;
                case Sym_Sym:
                    szSym_Type = "SYM";
                    break;
                case 8: // Sym_Virtual:
                    szSym_Type = "Virtual";
                    break;
                case 9: // Sym_Dia:
                    szSym_Type = "DIA";
                    break;
                }
            }
            this->m_parent->OnLoadModule(
                img, mod, baseAddr, size, result, szSym_Type, Module.LoadedImageName, fileVersion);
        }
        if (szImg != NULL)
            free(szImg);
        if (szMod != NULL)
            free(szMod);
        return result;
    }

public:
    BOOL LoadModules(HANDLE hProcess, DWORD dwProcessId) {
        // first try toolhelp32
        if (GetModuleListTH32(hProcess, dwProcessId))
            return true;
        // then try psapi
        return GetModuleListPSAPI(hProcess);
    }

    BOOL GetModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64_V2 *pModuleInfo) {
        if (this->pSGMI == NULL) {
            SetLastError(ERROR_DLL_INIT_FAILED);
            return FALSE;
        }
        // First try to use the larger ModuleInfo-Structure
        //    memset(pModuleInfo, 0, sizeof(IMAGEHLP_MODULE64_V3));
        //    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V3);
        //    if (this->pSGMI_V3 != NULL)
        //    {
        //      if (this->pSGMI_V3(hProcess, baseAddr, pModuleInfo) != FALSE)
        //        return TRUE;
        //      // check if the parameter was wrong (size is bad...)
        //      if (GetLastError() != ERROR_INVALID_PARAMETER)
        //        return FALSE;
        //    }
        // could not retrive the bigger structure, try with the smaller one (as
        // defined in VC7.1)...
        pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
        void *pData = malloc(4096); // reserve enough memory, so the bug in v6.3.5.1
                                    // does not lead to memory-overwrites...
        if (pData == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        memcpy(pData, pModuleInfo, sizeof(IMAGEHLP_MODULE64_V2));
        if (this->pSGMI(hProcess, baseAddr, (IMAGEHLP_MODULE64_V2 *)pData) != FALSE) {
            // only copy as much memory as is reserved...
            memcpy(pModuleInfo, pData, sizeof(IMAGEHLP_MODULE64_V2));
            pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
            free(pData);
            return TRUE;
        }
        free(pData);
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
    }
};

// #############################################################
StackWalker::StackWalker(DWORD dwProcessId, HANDLE hProcess) {
    this->m_Options = OptionsAll;
    this->m_ModulesLoaded = FALSE;
    this->m_HProcess = hProcess;
    this->m_Sw = new StackWalkerInternal(this, this->m_HProcess);
    this->m_DwProcessId = dwProcessId;
    this->m_SzSym_Path = NULL;
}
StackWalker::StackWalker(int options, LPCSTR szSym_Path, DWORD dwProcessId, HANDLE hProcess) {
    this->m_Options = options;
    this->m_ModulesLoaded = FALSE;
    this->m_HProcess = hProcess;
    this->m_Sw = new StackWalkerInternal(this, this->m_HProcess);
    this->m_DwProcessId = dwProcessId;
    if (szSym_Path != NULL) {
        this->m_SzSym_Path = _strdup(szSym_Path);
        this->m_Options |= Sym_BuildPath;
    } else
        this->m_SzSym_Path = NULL;
}

StackWalker::~StackWalker() {
    if (m_SzSym_Path != NULL)
        free(m_SzSym_Path);
    m_SzSym_Path = NULL;
    if (this->m_Sw != NULL)
        delete this->m_Sw;
    this->m_Sw = NULL;
}

BOOL StackWalker::LoadModules() {
    if (this->m_Sw == NULL) {
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
    }
    if (m_ModulesLoaded != FALSE)
        return TRUE;

    // Build the sym-path:
    char *szSym_Path = NULL;
    if ((this->m_Options & Sym_BuildPath) != 0) {
        const size_t nSym_PathLen = 4096;
        szSym_Path = (char *)malloc(nSym_PathLen);
        if (szSym_Path == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        szSym_Path[0] = 0;
        // Now first add the (optional) provided sympath:
        if (this->m_szSym_Path != NULL) {
            strcat_s(szSym_Path, nSym_PathLen, this->m_szSym_Path);
            strcat_s(szSym_Path, nSym_PathLen, ";");
        }

        strcat_s(szSym_Path, nSym_PathLen, ".;");

        const size_t nTempLen = 1024;
        char szTemp[nTempLen];
        // Now add the current directory:
        if (GetCurrentDirectoryA(nTempLen, szTemp) > 0) {
            szTemp[nTempLen - 1] = 0;
            strcat_s(szSym_Path, nSym_PathLen, szTemp);
            strcat_s(szSym_Path, nSym_PathLen, ";");
        }

        // Now add the path for the main-module:
        if (GetModuleFileNameA(NULL, szTemp, nTempLen) > 0) {
            szTemp[nTempLen - 1] = 0;
            for (char *p = (szTemp + strlen(szTemp) - 1); p >= szTemp; --p) {
                // locate the rightmost path separator
                if ((*p == '\\') || (*p == '/') || (*p == ':')) {
                    *p = 0;
                    break;
                }
            } // for (search for path separator...)
            if (strlen(szTemp) > 0) {
                strcat_s(szSym_Path, nSym_PathLen, szTemp);
                strcat_s(szSym_Path, nSym_PathLen, ";");
            }
        }
        if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", szTemp, nTempLen) > 0) {
            szTemp[nTempLen - 1] = 0;
            strcat_s(szSym_Path, nSym_PathLen, szTemp);
            strcat_s(szSym_Path, nSym_PathLen, ";");
        }
        if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", szTemp, nTempLen) > 0) {
            szTemp[nTempLen - 1] = 0;
            strcat_s(szSym_Path, nSym_PathLen, szTemp);
            strcat_s(szSym_Path, nSym_PathLen, ";");
        }
        if (GetEnvironmentVariableA("SYSTEMROOT", szTemp, nTempLen) > 0) {
            szTemp[nTempLen - 1] = 0;
            strcat_s(szSym_Path, nSym_PathLen, szTemp);
            strcat_s(szSym_Path, nSym_PathLen, ";");
            // also add the "system32"-directory:
            strcat_s(szTemp, nTempLen, "\\system32");
            strcat_s(szSym_Path, nSym_PathLen, szTemp);
            strcat_s(szSym_Path, nSym_PathLen, ";");
        }

        if ((this->m_Options & Sym_BuildPath) != 0) {
            if (GetEnvironmentVariableA("SYSTEMDRIVE", szTemp, nTempLen) > 0) {
                szTemp[nTempLen - 1] = 0;
                strcat_s(szSym_Path, nSym_PathLen, "SRV*");
                strcat_s(szSym_Path, nSym_PathLen, szTemp);
                strcat_s(szSym_Path, nSym_PathLen, "\\websymbols");
                strcat_s(szSym_Path, nSym_PathLen, "*http://msdl.microsoft.com/download/symbols;");
            } else
                strcat_s(szSym_Path,
                         nSym_PathLen,
                         "SRV*c:\\websymbols*http://msdl.microsoft.com/download/symbols;");
        }
    }

    // First Init the whole stuff...
    BOOL bRet = this->m_Sw->Init(szSym_Path);
    if (szSym_Path != NULL)
        free(szSym_Path);
    szSym_Path = NULL;
    if (bRet == FALSE) {
        this->OnDbgHelpErr("Error while initializing dbghelp.dll", 0, 0);
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
    }

    bRet = this->m_Sw->LoadModules(this->m_HProcess, this->m_DwProcessId);
    if (bRet != FALSE)
        m_ModulesLoaded = TRUE;
    return bRet;
}

// The following is used to pass the "userData"-Pointer to the user-provided
// readMemoryFunction This has to be done due to a problem with the
// "hProcess"-parameter in x64... Because this class is in no case
// multi-threading-enabled (because of the limitations of dbghelp.dll) it is
// "safe" to use a static-variable
static StackWalker::PReadProcessMemoryRoutine s_readMemoryFunction = NULL;
static LPVOID s_readMemoryFunction_UserData = NULL;

BOOL StackWalker::ShowCallstack(HANDLE hThread,
                                const CONTEXT *context,
                                PReadProcessMemoryRoutine readMemoryFunction,
                                LPVOID pUserData) {
    CONTEXT c;
    ;
    struct CallstackEntry csEntry;
    IMAGEHLP_SYMBOL64 *pSym = NULL;
    StackWalkerInternal::IMAGEHLP_MODULE64_V2 Module;
    IMAGEHLP_LINE64 Line;
    int frameNum;

    if (m_ModulesLoaded == FALSE)
        this->LoadModules(); // ignore the result...

    if (this->m_Sw->m_hDbhHelp == NULL) {
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
    }

    s_readMemoryFunction = readMemoryFunction;
    s_readMemoryFunction_UserData = pUserData;

    if (context == NULL) {
        // If no context is provided, capture the context
        if (hThread == GetCurrentThread()) {
            GET_CURRENT_CONTEXT(c, USED_CONTEXT_FLAGS);
        } else {
            SuspendThread(hThread);
            memset(&c, 0, sizeof(CONTEXT));
            c.ContextFlags = USED_CONTEXT_FLAGS;
            if (GetThreadContext(hThread, &c) == FALSE) {
                ResumeThread(hThread);
                return FALSE;
            }
        }
    } else
        c = *context;

    // init STACKFRAME for first call
    STACKFRAME64 s; // in/out stackframe
    memset(&s, 0, sizeof(s));
    DWORD imageType;
#ifdef _M_IX86
    // normally, call ImageNtHeader() and use machine info from PE header
    imageType = IMAGE_FILE_MACHINE_I386;
    s.AddrPC.Offset = c.Eip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Ebp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Esp;
    s.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    imageType = IMAGE_FILE_MACHINE_AMD64;
    s.AddrPC.Offset = c.Rip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Rsp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Rsp;
    s.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    imageType = IMAGE_FILE_MACHINE_IA64;
    s.AddrPC.Offset = c.StIIP;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.IntSp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrBStore.Offset = c.RsBSP;
    s.AddrBStore.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.IntSp;
    s.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

    pSym = (IMAGEHLP_SYMBOL64 *)malloc(sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
    if (!pSym)
        goto cleanup; // not enough memory...
    memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
    pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    pSym->MaxNameLength = STACKWALK_MAX_NAMELEN;

    memset(&Line, 0, sizeof(Line));
    Line.SizeOfStruct = sizeof(Line);

    memset(&Module, 0, sizeof(Module));
    Module.SizeOfStruct = sizeof(Module);

    for (frameNum = 0;; ++frameNum) {
        // get next stack frame (StackWalk64(), Sym_FunctionTableAccess64(),
        // Sym_GetModuleBase64()) if this returns ERROR_INVALID_ADDRESS (487) or
        // ERROR_NOACCESS (998), you can assume that either you are done, or that
        // the stack is so hosed that the next deeper frame could not be found.
        // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
        if (!this->m_Sw->pSW(imageType,
                             this->m_HProcess,
                             hThread,
                             &s,
                             &c,
                             myReadProcMem,
                             this->m_Sw->pSFTA,
                             this->m_Sw->pSGMB,
                             NULL)) {
            this->OnDbgHelpErr("StackWalk64", GetLastError(), s.AddrPC.Offset);
            break;
        }

        csEntry.offset = s.AddrPC.Offset;
        csEntry.name[0] = 0;
        csEntry.undName[0] = 0;
        csEntry.undFullName[0] = 0;
        csEntry.offsetFrom_Symbol = 0;
        csEntry.offsetFrom_Line = 0;
        csEntry.lineFileName[0] = 0;
        csEntry.lineNumber = 0;
        csEntry.loadedImageName[0] = 0;
        csEntry.moduleName[0] = 0;
        if (s.AddrPC.Offset == s.AddrReturn.Offset) {
            this->OnDbgHelpErr("StackWalk64-Endless-Callstack!", 0, s.AddrPC.Offset);
            break;
        }
        if (s.AddrPC.Offset != 0) {
            // we seem to have a valid PC
            // show procedure info (Sym_GetSym_From_Addr64())
            if (this->m_Sw->pSGSFA(
                    this->m_HProcess, s.AddrPC.Offset, &(csEntry.offsetFrom_Symbol), pSym) !=
                FALSE) {
                // TODO: Mache dies sicher...!
                strcpy_s(csEntry.name, pSym->Name);
                // UnDecorateSymbolName()
                this->m_Sw->pUDSN(
                    pSym->Name, csEntry.undName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY);
                this->m_Sw->pUDSN(
                    pSym->Name, csEntry.undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE);
            } else {
                this->OnDbgHelpErr("Sym_GetSym_From_Addr64", GetLastError(), s.AddrPC.Offset);
            }

            // show line number info, NT5.0-method (Sym_GetLineFrom_Addr64())
            if (this->m_Sw->pSGLFA != NULL) { // yes, we have Sym_GetLineFrom_Addr64()
                if (this->m_Sw->pSGLFA(
                        this->m_HProcess, s.AddrPC.Offset, &(csEntry.offsetFrom_Line), &Line) !=
                    FALSE) {
                    csEntry.lineNumber = Line.LineNumber;
                    // TODO: Mache dies sicher...!
                    strcpy_s(csEntry.lineFileName, Line.FileName);
                } else {
                    this->OnDbgHelpErr("Sym_GetLineFrom_Addr64", GetLastError(), s.AddrPC.Offset);
                }
            } // yes, we have Sym_GetLineFrom_Addr64()

            // show module info (Sym_GetModuleInfo64())
            if (this->m_Sw->GetModuleInfo(this->m_HProcess, s.AddrPC.Offset, &Module) !=
                FALSE) { // got module info OK
                switch (Module.Sym_Type) {
                case Sym_None:
                    csEntry.sym_TypeString = "-nosymbols-";
                    break;
                case Sym_Coff:
                    csEntry.sym_TypeString = "COFF";
                    break;
                case Sym_Cv:
                    csEntry.sym_TypeString = "CV";
                    break;
                case Sym_Pdb:
                    csEntry.sym_TypeString = "PDB";
                    break;
                case Sym_Export:
                    csEntry.sym_TypeString = "-exported-";
                    break;
                case Sym_Deferred:
                    csEntry.sym_TypeString = "-deferred-";
                    break;
                case Sym_Sym:
                    csEntry.sym_TypeString = "SYM";
                    break;
#if API_VERSION_NUMBER >= 9
                case Sym_Dia:
                    csEntry.sym_TypeString = "DIA";
                    break;
#endif
                case 8: // Sym_Virtual:
                    csEntry.sym_TypeString = "Virtual";
                    break;
                default:
                    //_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.Sym_Type );
                    csEntry.sym_TypeString = NULL;
                    break;
                }

                // TODO: Mache dies sicher...!
                strcpy_s(csEntry.moduleName, Module.ModuleName);
                csEntry.baseOfImage = Module.BaseOfImage;
                strcpy_s(csEntry.loadedImageName, Module.LoadedImageName);
            } // got module info OK
            else {
                this->OnDbgHelpErr("Sym_GetModuleInfo64", GetLastError(), s.AddrPC.Offset);
            }
        } // we seem to have a valid PC

        CallstackEntryType et = nextEntry;
        if (frameNum == 0)
            et = firstEntry;
        this->OnCallstackEntry(et, csEntry);

        if (s.AddrReturn.Offset == 0) {
            this->OnCallstackEntry(lastEntry, csEntry);
            SetLastError(ERROR_SUCCESS);
            break;
        }
    } // for ( frameNum )

cleanup:
    if (pSym)
        free(pSym);

    if (context == NULL)
        ResumeThread(hThread);

    return TRUE;
}

BOOL __stdcall StackWalker::myReadProcMem(HANDLE hProcess,
                                          DWORD64 qwBaseAddress,
                                          PVOID lpBuffer,
                                          DWORD nSize,
                                          LPDWORD lpNumberOfBytesRead) {
    if (s_readMemoryFunction == NULL) {
        SIZE_T st;
        BOOL bRet = ReadProcessMemory(hProcess, (LPVOID)qwBaseAddress, lpBuffer, nSize, &st);
        *lpNumberOfBytesRead = (DWORD)st;
        // printf("ReadMemory: hProcess: %p, baseAddr: %p, buffer: %p, size: %d,
        // read: %d, result: %d\n", hProcess, (LPVOID) qwBaseAddress, lpBuffer,
        // nSize, (DWORD) st, (DWORD) bRet);
        return bRet;
    } else {
        return s_readMemoryFunction(hProcess,
                                    qwBaseAddress,
                                    lpBuffer,
                                    nSize,
                                    lpNumberOfBytesRead,
                                    s_readMemoryFunction_UserData);
    }
}

void StackWalker::OnLoadModule(LPCSTR img,
                               LPCSTR mod,
                               DWORD64 baseAddr,
                               DWORD size,
                               DWORD result,
                               LPCSTR sym_Type,
                               LPCSTR pdbName,
                               ULONGLONG fileVersion) {
    CHAR buffer[STACKWALK_MAX_NAMELEN];
    if (fileVersion == 0)
        _snprintf_s(buffer,
                    STACKWALK_MAX_NAMELEN,
                    "%s:%s (%p), size: %d (result: %d), Sym_Type: '%s', PDB: '%s'\n",
                    img,
                    mod,
                    (LPVOID)baseAddr,
                    size,
                    result,
                    sym_Type,
                    pdbName);
    else {
        DWORD v4 = (DWORD)fileVersion & 0xFFFF;
        DWORD v3 = (DWORD)(fileVersion >> 16) & 0xFFFF;
        DWORD v2 = (DWORD)(fileVersion >> 32) & 0xFFFF;
        DWORD v1 = (DWORD)(fileVersion >> 48) & 0xFFFF;
        _snprintf_s(buffer,
                    STACKWALK_MAX_NAMELEN,
                    "%s:%s (%p), size: %d (result: %d), Sym_Type: '%s', PDB: '%s', "
                    "fileVersion: %d.%d.%d.%d\n",
                    img,
                    mod,
                    (LPVOID)baseAddr,
                    size,
                    result,
                    sym_Type,
                    pdbName,
                    v1,
                    v2,
                    v3,
                    v4);
    }
    OnOutput(buffer);
}

void StackWalker::OnCallstackEntry(CallstackEntryType eType, struct CallstackEntry &entry) {
    CHAR buffer[STACKWALK_MAX_NAMELEN];
    if ((eType != lastEntry) && (entry.offset != 0)) {
        if (entry.name[0] == 0)
            strcpy_s(entry.name, "(function-name not available)");
        if (entry.undName[0] != 0)
            strcpy_s(entry.name, entry.undName);
        if (entry.undFullName[0] != 0)
            strcpy_s(entry.name, entry.undFullName);
        if (entry.lineFileName[0] == 0) {
            strcpy_s(entry.lineFileName, "(filename not available)");
            if (entry.moduleName[0] == 0)
                strcpy_s(entry.moduleName, "module-name not available");

            _snprintf_s(buffer,
                        STACKWALK_MAX_NAMELEN,
                        "%p (%s:%p): %s: %s\n",
                        (LPVOID)entry.offset,
                        entry.moduleName,
                        (LPVOID)entry.baseOfImage,
                        entry.lineFileName,
                        entry.name);
        } else
            _snprintf_s(buffer,
                        STACKWALK_MAX_NAMELEN,
                        "%s (%d): %s\n",
                        entry.lineFileName,
                        entry.lineNumber,
                        entry.name);
        OnOutput(buffer);
    }
}

void StackWalker::OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) {
    CHAR buffer[STACKWALK_MAX_NAMELEN];
    _snprintf_s(buffer,
                STACKWALK_MAX_NAMELEN,
                "ERROR: %s, GetLastError: %d (Address: %p)\n",
                szFuncName,
                gle,
                (LPVOID)addr);
    OnOutput(buffer);
}

void StackWalker::OnSym_Init(LPCSTR szSearchPath, DWORD sym_Options, LPCSTR szUserName) {
    CHAR buffer[STACKWALK_MAX_NAMELEN];
    _snprintf_s(buffer,
                STACKWALK_MAX_NAMELEN,
                "Sym_Init: Symbol-SearchPath: '%s', sym_Options: %d, UserName: '%s'\n",
                szSearchPath,
                sym_Options,
                szUserName);
    OnOutput(buffer);
    // Also display the OS-version
#if _MSC_VER <= 1200
    OSVERSIONINFOA ver;
    ZeroMemory(&ver, sizeof(OSVERSIONINFOA));
    ver.dwOSVersionInfoSize = sizeof(ver);
    if (GetVersionExA(&ver) != FALSE) {
        _snprintf_s(buffer,
                    STACKWALK_MAX_NAMELEN,
                    "OS-Version: %d.%d.%d (%s)\n",
                    ver.dwMajorVersion,
                    ver.dwMinorVersion,
                    ver.dwBuildNumber,
                    ver.szCSDVersion);
        OnOutput(buffer);
    }
#else
    OSVERSIONINFOEXA ver;
    ZeroMemory(&ver, sizeof(OSVERSIONINFOEXA));
    ver.dwOSVersionInfoSize = sizeof(ver);
    if (GetVersionExA((OSVERSIONINFOA *)&ver) != FALSE) {
        _snprintf_s(buffer,
                    STACKWALK_MAX_NAMELEN,
                    "OS-Version: %d.%d.%d (%s) 0x%x-0x%x\n",
                    ver.dwMajorVersion,
                    ver.dwMinorVersion,
                    ver.dwBuildNumber,
                    ver.szCSDVersion,
                    ver.wSuiteMask,
                    ver.wProductType);
        OnOutput(buffer);
    }
#endif
}

void StackWalker::OnOutput(LPCSTR buffer) {
    OutputDebugStringA(buffer);
}
