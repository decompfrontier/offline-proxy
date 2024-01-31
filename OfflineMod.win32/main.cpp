/**
* @file main.cpp
* @author Arves100
* @date 30/10/2024
* @brief Entrypoint for Brave Frontier Windows offline mod
*/
#include "pch.h"
#include <detours/detours.h>

typedef HINTERNET(WINAPI* InternetConnectW_t)(
    _In_ HINTERNET     hInternet,
    _In_ LPCWSTR       lpszServerName,
    _In_ INTERNET_PORT nServerPort,
    _In_ LPCWSTR       lpszUserName,
    _In_ LPCWSTR       lpszPassword,
    _In_ DWORD         dwService,
    _In_ DWORD         dwFlags,
    _In_ DWORD_PTR     dwContext
);

typedef HINTERNET(WINAPI* InternetConnectA_t)(
    _In_ HINTERNET     hInternet,
    _In_ LPCSTR        lpszServerName,
    _In_ INTERNET_PORT nServerPort,
    _In_ LPCSTR        lpszUserName,
    _In_ LPCSTR        lpszPassword,
    _In_ DWORD         dwService,
    _In_ DWORD         dwFlags,
    _In_ DWORD_PTR     dwContext
);

typedef HINTERNET(WINAPI* HttpOpenRequestA_t)(
    _In_ HINTERNET hConnect,
    _In_ LPCSTR    lpszVerb,
    _In_ LPCSTR    lpszObjectName,
    _In_ LPCSTR    lpszVersion,
    _In_ LPCSTR    lpszReferrer,
    _In_ LPCSTR* lplpszAcceptTypes,
    _In_ DWORD     dwFlags,
    _In_ DWORD_PTR dwContext
);

typedef HINTERNET(WINAPI* HttpOpenRequestW_t)(
    _In_ HINTERNET hConnect,
    _In_ LPCWSTR   lpszVerb,
    _In_ LPCWSTR   lpszObjectName,
    _In_ LPCWSTR   lpszVersion,
    _In_ LPCWSTR   lpszReferrer,
    _In_ LPCWSTR* lplpszAcceptTypes,
    _In_ DWORD     dwFlags,
    _In_ DWORD_PTR dwContext
);

static InternetConnectW_t TrueInternetConnectW = InternetConnectW;
static InternetConnectA_t TrueInternetConnectA = InternetConnectA;
static HttpOpenRequestA_t TrueHttpOpenRequestA = HttpOpenRequestA;
static HttpOpenRequestW_t TrueHttpOpenRequestW = HttpOpenRequestW;

static HINTERNET WINAPI MyInternetConnectA(
    _In_ HINTERNET     hInternet,
    _In_ LPCSTR        lpszServerName,
    _In_ INTERNET_PORT nServerPort,
    _In_ LPCSTR        lpszUserName,
    _In_ LPCSTR        lpszPassword,
    _In_ DWORD         dwService,
    _In_ DWORD         dwFlags,
    _In_ DWORD_PTR     dwContext
)
{
#ifdef _DEBUG
    printf("InternetConnectA: %s:%u (SVC:%x)\n", lpszServerName, nServerPort, dwService);
#endif

    if (strstr(lpszServerName, "microsoft.com") != nullptr || strstr(lpszServerName, "wikia.com") != nullptr || strstr(lpszServerName, "fandom.com") != nullptr)
        return TrueInternetConnectA(hInternet, lpszServerName, INTERNET_DEFAULT_HTTP_PORT, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);

    return TrueInternetConnectA(hInternet, "127.0.0.1", 9960, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
}

static HINTERNET WINAPI MyInternetConnectW(
    _In_ HINTERNET     hInternet,
    _In_ LPCWSTR        lpszServerName,
    _In_ INTERNET_PORT nServerPort,
    _In_ LPCWSTR        lpszUserName,
    _In_ LPCWSTR        lpszPassword,
    _In_ DWORD         dwService,
    _In_ DWORD         dwFlags,
    _In_ DWORD_PTR     dwContext
)
{
#ifdef _DEBUG
    wprintf(L"InternetConnectW: %s:%u (SVC:%x)\n", lpszServerName, nServerPort, dwService);
#endif

    if (wcswcs(lpszServerName, L"microsoft.com") != nullptr || wcswcs(lpszServerName, L"wikia.com") != nullptr || wcswcs(lpszServerName, L"fandom.com") != nullptr)
        return TrueInternetConnectW(hInternet, lpszServerName, INTERNET_DEFAULT_HTTP_PORT, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);

    return TrueInternetConnectW(hInternet, L"127.0.0.1", 9960, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
}

static void PatchSecurityOptions(HINTERNET hInternet)
{
    DWORD dwFlags2;
    DWORD dwBuffLen = sizeof(dwFlags2);

    if (InternetQueryOption(hInternet, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, &dwBuffLen))
    {
        printf("patched options...\n");
        dwFlags2 |= SECURITY_SET_MASK;
        InternetSetOption(hInternet, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, sizeof(dwFlags2));
    }
}

static HINTERNET WINAPI MyHttpOpenRequestA(
    _In_ HINTERNET hConnect,
    _In_ LPCSTR    lpszVerb,
    _In_ LPCSTR    lpszObjectName,
    _In_ LPCSTR    lpszVersion,
    _In_ LPCSTR    lpszReferrer,
    _In_ LPCSTR* lplpszAcceptTypes,
    _In_ DWORD     dwFlags,
    _In_ DWORD_PTR dwContext
)
{
    dwFlags &= ~INTERNET_FLAG_SECURE;
    dwFlags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
    auto ret = TrueHttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);

    if (!ret)
        return nullptr;

    PatchSecurityOptions(ret);
    return ret;
}

HINTERNET WINAPI MyHttpOpenRequestW(
    _In_ HINTERNET hConnect,
    _In_ LPCWSTR   lpszVerb,
    _In_ LPCWSTR   lpszObjectName,
    _In_ LPCWSTR   lpszVersion,
    _In_ LPCWSTR   lpszReferrer,
    _In_ LPCWSTR* lplpszAcceptTypes,
    _In_ DWORD     dwFlags,
    _In_ DWORD_PTR dwContext
)
{
    dwFlags &= ~INTERNET_FLAG_SECURE;
    dwFlags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
    auto ret = TrueHttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);

    if (!ret)
        return nullptr;

    PatchSecurityOptions(ret);
    return ret;
}

static void DetourDetach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)TrueInternetConnectA, MyInternetConnectA);
    DetourDetach(&(PVOID&)TrueInternetConnectW, MyInternetConnectW);
    DetourDetach(&(PVOID&)TrueHttpOpenRequestA, MyHttpOpenRequestA);
    DetourDetach(&(PVOID&)TrueHttpOpenRequestW, MyHttpOpenRequestW);
    DetourTransactionCommit();
}

static void DetourAttach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)TrueInternetConnectA, MyInternetConnectA);
    DetourAttach(&(PVOID&)TrueInternetConnectW, MyInternetConnectW);
    DetourAttach(&(PVOID&)TrueHttpOpenRequestA, MyHttpOpenRequestA);
    DetourAttach(&(PVOID&)TrueHttpOpenRequestW, MyHttpOpenRequestW);
    DetourTransactionCommit();
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    (void)hinstDLL;
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_DETACH)
    {
        DetourDetach();
    }
    else if (fdwReason == DLL_PROCESS_ATTACH)
    {
#ifdef _DEBUG
        if (!AllocConsole())
        {
            MessageBoxA(nullptr, "ALLOC_CONSOLE_FAILED", "DEBUG", MB_OK | MB_ICONERROR);
        }
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
#endif
        DetourAttach();
    }

    return TRUE;
}
