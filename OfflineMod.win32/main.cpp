/**
* @file main.cpp
* @author Arves100
* @date 30/10/2024
* @brief Entrypoint for Brave Frontier Windows offline mod
*/
#include "pch.h"
#include "config.h"
#include "patch.h"
#include "log.h"
#include "console.h"

static void DetourDetach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)TrueInternetConnectA, (PVOID)MyInternetConnectA);
    DetourDetach(&(PVOID&)TrueInternetConnectW, (PVOID)MyInternetConnectW);
#if SERVICE_DISABLE_HTTPS
    DetourDetach(&(PVOID&)TrueHttpOpenRequestA, (PVOID)MyHttpOpenRequestA);
    DetourDetach(&(PVOID&)TrueHttpOpenRequestW, (PVOID)MyHttpOpenRequestW);
#endif
    FpsCap_Detach();
    DetourTransactionCommit();
}

static void DetourAttach()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)TrueInternetConnectA, (PVOID)MyInternetConnectA);
    DetourAttach(&(PVOID&)TrueInternetConnectW, (PVOID)MyInternetConnectW);
#if SERVICE_DISABLE_HTTPS
    DetourAttach(&(PVOID&)TrueHttpOpenRequestA, (PVOID)MyHttpOpenRequestA);
    DetourAttach(&(PVOID&)TrueHttpOpenRequestW, (PVOID)MyHttpOpenRequestW);
#endif
    FpsCap_Attach();
    DetourTransactionCommit();
}

static ConsoleAPI g_theConsole;
static GimuServerAPI g_theApi;
static ProxyConfig g_config;

static void FreeDll(void);

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    (void)hinstDLL;
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        if (!g_config.Load("proxy.ini"))
        {
            MessageBoxW(nullptr, L"Unable to load the proxy configuration\nPlease make sure the proxy correctly!\nFor more information, visit decompfrontier proxy repository", L"Fatal Error", MB_OK | MB_ICONERROR);
            FreeDll();
            return FALSE;
        }

        if (g_config.enable_diagnostics)
        {
            if (!g_theConsole.Init())
            {
                MessageBoxW(nullptr, L"Unable to allocate diagnostic console...", L"Fatal Error", MB_OK | MB_ICONERROR);
                FreeDll();
                return FALSE;
            }
        }

        if (!getLog().Init(g_config.log_file.c_str(), g_config.enable_diagnostics))
        {
            MessageBoxW(nullptr, L"Unable to spawn logging system...", L"Fatal Error", MB_OK | MB_ICONERROR);
            FreeDll();
            return FALSE;
        }

        if (g_config.enable_deploy_mode)
        {
            if (!g_theApi.Load(GIMUSERVER_DLL_NAME))
            {
                MessageBoxW(nullptr, L"Unable to find or load the offline server component\nPlease make sure you installed gimuserver correctly!\nFor more information, visit decompfrontier proxy repository", L"Fatal Error", MB_OK | MB_ICONERROR);
                FreeDll();
                return FALSE;
            }

            g_theApi.Startup(g_config.port); // startup the offline mod
        }

        if (!PatchStart(g_config))
        {
            MessageBoxW(nullptr, L"Unable to patch the game executable, the proxy will not work\n", L"Fatal Error", MB_OK | MB_ICONERROR);
            FreeDll();
            return FALSE;
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        FreeDll();
    }

    return TRUE;
}

void FreeDll(void)
{
    g_theApi.Free();
    getLog().Free();
    g_theConsole.Free();
}
