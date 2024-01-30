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

static InternetConnectW_t TrueInternetConnectW = InternetConnectW;
static InternetConnectA_t TrueInternetConnectA = InternetConnectA;

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
    //MessageBoxA(nullptr, "SLEEP", "DEBUG", MB_OK);
#endif
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
    //MessageBoxA(nullptr, "SLEEP", "DEBUG", MB_OK);
#endif
    return TrueInternetConnectW(hInternet, L"127.0.0.1", 9960, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
}

static void DetourDetach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)TrueInternetConnectA, MyInternetConnectA);
    DetourDetach(&(PVOID&)TrueInternetConnectW, MyInternetConnectW);
    DetourTransactionCommit();
}

static void DetourAttach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)TrueInternetConnectA, MyInternetConnectA);
    DetourAttach(&(PVOID&)TrueInternetConnectW, MyInternetConnectW);
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
