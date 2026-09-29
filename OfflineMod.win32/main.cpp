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

static ConsoleAPI g_theConsole;
static GimuServerAPI g_theServer;
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
            if (!g_theServer.Load(GIMUSERVER_DLL_NAME))
            {
                MessageBoxW(nullptr, L"Unable to find or load the offline server component\nPlease make sure you installed gimuserver correctly!\nFor more information, visit decompfrontier proxy repository", L"Fatal Error", MB_OK | MB_ICONERROR);
                FreeDll();
                return FALSE;
            }

            g_theServer.Startup(g_config.port); // startup the offline mod
        }

        if (!getAPI().Attach())
        {
            MessageBoxW(nullptr, L"Unable to patch the game executable, the proxy will not work\n", L"Fatal Error", MB_OK | MB_ICONERROR);
            FreeDll();
            return FALSE;
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        getAPI().Detach();
        FreeDll();
    }

    return TRUE;
}

void FreeDll(void)
{
    g_theServer.Free();
    getLog().Free();
    g_theConsole.Free();
}
