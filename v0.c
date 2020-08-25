/*++
 * build x86/x64 with:
 *   cl -W4 v0.c -link ntdll.lib advapi32.lib
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

/*++ s(116) ... */
typedef struct _NT_VERSION
{
    DWORD major;
    DWORD minor;
    DWORD build;
    DWORD ubr;
    DWORD release_id;
} NT_VERSION, *PNT_VERSION;

/*++ nt-sysinfo.c */
extern int              __nt_getversion(NT_VERSION* pver);
extern int              __nt_getversion_w32(NT_VERSION* pver);

/*++ nt-sysinfo.c */
static BOOL             __get_version_rtl(NT_VERSION* pver);
static BOOL             __get_version_w32(NT_VERSION* pver);
static size_t           __wcslen(const wchar_t* str);
static int              __wtoi(const wchar_t* str);

static ULONG        g_defflags         = 0xBAADF00D;
static wchar_t*     g_defstring        = L"BAADF00D";
static wchar_t*     g_version_path_rtl = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion";
static wchar_t*     g_version_path_w32 = L"Software\\Microsoft\\Windows NT\\CurrentVersion";

/*++ >>>>>>>>>>>>>>>>>>>>>>>           |           <<<<<<<<<<<<<<<<<<<<<<< --*/
/*++ >>>>>>>>>>>>>>>>>>>>>>>          wnt          <<<<<<<<<<<<<<<<<<<<<<< --*/
/*++ >>>>>>>>>>>>>>>>>>>>>>>   obliti privatorum   <<<<<<<<<<<<<<<<<<<<<<< --*/

/*++ from ntdef.h ... */
typedef LONG        NTSTATUS;
typedef NTSTATUS    *PNTSTATUS;

#define NT_SUCCESS(Status)              ((NTSTATUS)(Status) >= 0)
#define NT_INFORMATION(Status)          ((ULONG)(Status) >> 30 == 1)
#define NT_WARNING(Status)              ((ULONG)(Status) >> 30 == 2)
#define NT_ERROR(Status)                ((ULONG)(Status) >> 30 == 3)

/*++ ... */
typedef struct _UNICODE_STRING 
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

#define UNICODE_NULL                    ((WCHAR)0)  /*++ winnt  */

/*++
 */
NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString (
    PUNICODE_STRING UnicodeString
    );

/*++
 */
NTSYSAPI
VOID
NTAPI
RtlSetLastWin32ErrorAndNtStatusFromNtStatus (
    __in NTSTATUS Status
    );

/*++ registry bits ... */

typedef NTSTATUS (NTAPI * PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

/*++ ... */
typedef struct _RTL_QUERY_REGISTRY_TABLE 
{
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;
} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;

/*++ query table flags values ... */
#define RTL_QUERY_REGISTRY_SUBKEY       0x00000001
#define RTL_QUERY_REGISTRY_TOPKEY       0x00000002
#define RTL_QUERY_REGISTRY_REQUIRED     0x00000004
#define RTL_QUERY_REGISTRY_NOVALUE      0x00000008
#define RTL_QUERY_REGISTRY_NOEXPAND     0x00000010
#define RTL_QUERY_REGISTRY_DIRECT       0x00000020
#define RTL_QUERY_REGISTRY_DELETE       0x00000040

/*++ relativeto values ... */
#define RTL_REGISTRY_ABSOLUTE           0
#define RTL_REGISTRY_SERVICES           1
#define RTL_REGISTRY_CONTROL            2
#define RTL_REGISTRY_WINDOWS_NT         3
#define RTL_REGISTRY_DEVICEMAP          4
#define RTL_REGISTRY_USER               5
#define RTL_REGISTRY_MAXIMUM            6
#define RTL_REGISTRY_HANDLE             0x40000000
#define RTL_REGISTRY_OPTIONAL           0x80000000

/*++
 */
NTSYSAPI
NTSTATUS
NTAPI
RtlQueryRegistryValues (
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

/*++ >>>>>>>>>>>>>>>>>>>>>>>           |           <<<<<<<<<<<<<<<<<<<<<<< --*/
/*++ >>>>>>>>>>>>>>>>>>>>>>>        publius        <<<<<<<<<<<<<<<<<<<<<<< --*/

/*++
 */
int
__nt_getversion (
    NT_VERSION* pver )
{
    if( __get_version_rtl(pver) == FALSE)
    {
        /*++ last error set by function ... */
        return -1;
    }
    return 0;
}

/*++
 */
int
__nt_getversion_w32 (
    NT_VERSION* pver )
{
    if( __get_version_w32(pver) == FALSE)
    {
        /*++ last error set by function ... */
        return -1;
    }
    return 0;
}

/*++ >>>>>>>>>>>>>>>>>>>>>>>           |           <<<<<<<<<<<<<<<<<<<<<<< --*/
/*++ >>>>>>>>>>>>>>>>>>>>>>>        statics        <<<<<<<<<<<<<<<<<<<<<<< --*/

/*++
 */
static
BOOL
__get_version_rtl (
    NT_VERSION* pver )
{
    ULONG RelativeTo = (RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL);
    ULONG cbdefstring;
    NTSTATUS status;
    UNICODE_STRING usBuild = {0};
    UNICODE_STRING usRelease = {0};
    RTL_QUERY_REGISTRY_TABLE QueryTable[8] = {0};

    if(pver == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    cbdefstring = (ULONG)((__wcslen(g_defstring) + 1) * sizeof(WCHAR));

    QueryTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[0].Name          = L"CurrentMajorVersionNumber";
    QueryTable[0].EntryContext  = &(pver->major);
    QueryTable[0].DefaultType   = REG_DWORD;
    QueryTable[0].DefaultData   = &g_defflags;
    QueryTable[0].DefaultLength = sizeof(ULONG);

    QueryTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[1].Name          = L"CurrentMinorVersionNumber";
    QueryTable[1].EntryContext  = &(pver->minor);
    QueryTable[1].DefaultType   = REG_DWORD;
    QueryTable[1].DefaultData   = &g_defflags;
    QueryTable[1].DefaultLength = sizeof(ULONG);

    QueryTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[2].Name          = L"CurrentBuildNumber";
    QueryTable[2].EntryContext  = &usBuild;
    QueryTable[2].DefaultType   = REG_SZ;
    QueryTable[2].DefaultData   = g_defstring;
    QueryTable[2].DefaultLength = cbdefstring;

    QueryTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[3].Name          = L"UBR";
    QueryTable[3].EntryContext  = &(pver->ubr);
    QueryTable[3].DefaultType   = REG_DWORD;
    QueryTable[3].DefaultData   = &g_defflags;
    QueryTable[3].DefaultLength = sizeof(ULONG);

    QueryTable[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[4].Name          = L"ReleaseId";
    QueryTable[4].EntryContext  = &usRelease;
    QueryTable[4].DefaultType   = REG_SZ;
    QueryTable[4].DefaultData   = g_defstring;
    QueryTable[4].DefaultLength = cbdefstring;

    __try
    {
        status = RtlQueryRegistryValues(RelativeTo, g_version_path_rtl, &QueryTable[0], NULL, NULL);
        if( !NT_SUCCESS(status))
        {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(status);
            return FALSE;
        }

        pver->build = __wtoi(usBuild.Buffer);
        pver->release_id = __wtoi(usRelease.Buffer);
    }
    __finally
    {
        RtlFreeUnicodeString(&usBuild);
        RtlFreeUnicodeString(&usRelease);
    }
    return TRUE;
}

/*++
 */
static
BOOL
__get_version_w32 (
    NT_VERSION* pver )
{
    HKEY osk = NULL;
    LONG status;
    DWORD size;
    DWORD type = REG_SZ;
    wchar_t buffer[64] = {0};

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, g_version_path_w32, 0, KEY_QUERY_VALUE, &osk);
    if(status != ERROR_SUCCESS)
    {
        SetLastError(status);
        return FALSE;
    }

    __try
    {
        type = REG_DWORD;
        size = sizeof(pver->major);
        status = RegQueryValueExW(osk, L"CurrentMajorVersionNumber", 0, &type, (void*)&(pver->major), &size);
        if(status != ERROR_SUCCESS)
        {
            SetLastError(status);
            return FALSE;
        }

        type = REG_DWORD;
        size = sizeof(pver->minor);
        status = RegQueryValueExW(osk, L"CurrentMinorVersionNumber", 0, &type, (void*)&(pver->minor), &size);
        if(status != ERROR_SUCCESS)
        {
            SetLastError(status);
            return FALSE;
        }

        type = REG_SZ;
        size = sizeof(buffer);
        status = RegQueryValueExW(osk, L"CurrentBuildNumber", 0, &type, (void*)&buffer, &size);
        if(status != ERROR_SUCCESS)
        {
            SetLastError(status);
            return FALSE;
        }

        pver->build = __wtoi(buffer);

        type = REG_DWORD;
        size = sizeof(pver->ubr);
        status = RegQueryValueExW(osk, L"UBR", 0, &type, (void*)&(pver->ubr), &size);
        if(status != ERROR_SUCCESS)
        {
            SetLastError(status);
            return FALSE;
        }

        type = REG_SZ;
        size = sizeof(buffer);
        status = RegQueryValueExW(osk, L"ReleaseId", 0, &type, (void*)&buffer, &size);
        if(status != ERROR_SUCCESS)
        {
            SetLastError(status);
            return FALSE;
        }

        pver->release_id = __wtoi(buffer);
    }
    __finally
    {
        RegCloseKey(osk);
    }
    return TRUE;
}

/*++
 */
static
size_t
__wcslen (
    const wchar_t* str )
{
    const wchar_t* eos = str;
    while(eos && *eos++);
    return (size_t)((eos != NULL) ? (eos - str - 1) : 0);
}

/*++
 */
static
int
__wtoi (
    const wchar_t* str )
{
    int n = 0;
    int signum = 0;
    for(;; str++)
    {
        switch(*str)
        {
        case L' ':
        case L'\t': continue;
        case L'-':  signum++;
        case L'+':  str++;
        }
        break;
    }
    while((*str >= L'0') && (*str <= L'9'))
    {
        n = n * 10 + *str++ - L'0';
    }
    return ((signum != 0) ? -n : n);
}

/*++ >>>>>>>>>>>>>>>>>>>>>>>           |           <<<<<<<<<<<<<<<<<<<<<<< --*/
/*++ >>>>>>>>>>>>>>>>>>>>>>>         tests         <<<<<<<<<<<<<<<<<<<<<<< --*/

//#ifdef __NT_SYSINFO_TESTAPP__
int
wmain (
    int argc,
    wchar_t** argv )
{
    /*++ */
    NT_VERSION ver = {0};
    if( __nt_getversion(&ver) != 0)
    {
        fprintf(stderr, "get version(rtl) failed, status(%X)\n", GetLastError());
        return 1;
    }

    printf(
     "Windows NT %d.%d.%d.%d Release %d [rtl]\n",
     ver.major, 
     ver.minor, 
     ver.build, 
     ver.ubr,
     ver.release_id
     );

    /*++ */
    NT_VERSION ver_w32 = {0};
    if( __nt_getversion(&ver_w32) != 0)
    {
        fprintf(stderr, "get version(w32) failed, status(%X)\n", GetLastError());
        return 1;
    }

    printf(
     "Windows NT %d.%d.%d.%d Release %d [w32]\n",
     ver_w32.major, 
     ver_w32.minor, 
     ver_w32.build, 
     ver_w32.ubr,
     ver_w32.release_id
     );

    return 0;
}
//#endif  /* __NT_SYSINFO_TESTAPP__ */
