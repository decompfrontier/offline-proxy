/**
* @file patch/patch_http.cpp
* @author Arves100
* @date 29/09/2026
* @brief HTTP patching API
*/
#include "pch.h"
#include "patch.h"
#include "log.h"
#include "config.h"
#include <detours/detours.h>

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
    return getAPI().http.MyInternetConnectA(
        hInternet,
        lpszServerName,
        nServerPort,
        lpszUserName,
        lpszPassword,
        dwService,
        dwFlags,
        dwContext
    );
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
    return getAPI().http.MyInternetConnectW(
        hInternet,
        lpszServerName,
        nServerPort,
        lpszUserName,
        lpszPassword,
        dwService,
        dwFlags,
        dwContext
    );
}

HINTERNET WINAPI MyHttpOpenRequestA(
    _In_ HINTERNET hConnect,
    _In_ LPCSTR    lpszVerb,
    _In_ LPCSTR    lpszObjectName,
    _In_ LPCSTR    lpszVersion,
    _In_ LPCSTR    lpszReferrer,
    _In_ LPCSTR* lplpxszAcceptTypes,
    _In_ DWORD     dwFlags,
    _In_ DWORD_PTR dwContext
)
{
    return getAPI().http.MyHttpOpenRequestA(
        hConnect,
        lpszVerb,
        lpszObjectName,
        lpszVersion,
        lpszReferrer,
        lplpxszAcceptTypes,
        dwFlags,
        dwContext
    );
}

static HINTERNET WINAPI MyHttpOpenRequestW(
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
    return getAPI().http.MyHttpOpenRequestW(
        hConnect,
        lpszVerb,
        lpszObjectName,
        lpszVersion,
        lpszReferrer,
        lplpszAcceptTypes,
        dwFlags,
        dwContext
    );
}

HINTERNET PatchHTTPAPI::MyInternetConnectA(
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
    LOG_DBUG("InternetConnectA: %s:%u (SVC:%x)\n", lpszServerName, nServerPort, dwService);

    auto& beg = getConfig().skip_url_patch.cbegin();
    const auto& end = getConfig().skip_url_patch.cend();

    for (; beg != end; beg++)
    {
        // microsoft.com
        // wikia.com
        // fandom.com
        if (strstr(lpszServerName, beg->c_str()) != nullptr)
        {
            return TrueInternetConnectA(hInternet, lpszServerName, INTERNET_DEFAULT_HTTP_PORT, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
        }
    }

    return TrueInternetConnectA(hInternet, getConfig().ip.c_str(), getConfig().port, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
}

HINTERNET PatchHTTPAPI::MyInternetConnectW(
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
    LOG_DBUG("InternetConnectW: %S:%u (SVC:%x)\n", lpszServerName, nServerPort, dwService);


    auto& beg = getConfig().skip_url_patch_w.cbegin();
    const auto& end = getConfig().skip_url_patch_w.cend();

    for (; beg != end; beg++)
    {
        // microsoft.com
        // wikia.com
        // fandom.com
        if (wcswcs(lpszServerName, beg->c_str()) != nullptr)
        {
            return TrueInternetConnectW(hInternet, lpszServerName, INTERNET_DEFAULT_HTTP_PORT, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
        }
    }

    return TrueInternetConnectW(hInternet, getConfig().ip_w.c_str(), getConfig().port, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
}


void PatchHTTPAPI::PatchSecurityOptions(HINTERNET hInternet)
{
DWORD dwFlags2 = 0;
DWORD dwBuffLen = sizeof(dwFlags2);

    if (InternetQueryOption(hInternet, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, &dwBuffLen))
    {
        dwFlags2 |= SECURITY_SET_MASK;
        InternetSetOption(hInternet, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, sizeof(dwFlags2));
    }
}

HINTERNET PatchHTTPAPI::MyHttpOpenRequestA(
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

HINTERNET PatchHTTPAPI::MyHttpOpenRequestW(
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

PatchHTTPAPI::PatchHTTPAPI()
: 
	TrueInternetConnectW(InternetConnectW),
	TrueInternetConnectA(InternetConnectA),
	TrueHttpOpenRequestA(HttpOpenRequestA),
	TrueHttpOpenRequestW(HttpOpenRequestW)
{}

void PatchHTTPAPI::Attach(void)
{
    DetourAttach(&(PVOID&)TrueInternetConnectA, (PVOID)::MyInternetConnectA);
    DetourAttach(&(PVOID&)TrueInternetConnectW, (PVOID)::MyInternetConnectW);

    if (!getConfig().enable_https)
    {
        DetourAttach(&(PVOID&)TrueHttpOpenRequestA, (PVOID)::MyHttpOpenRequestA);
        DetourAttach(&(PVOID&)TrueHttpOpenRequestW, (PVOID)::MyHttpOpenRequestW);
    }
}

void PatchHTTPAPI::Detach(void)
{
    DetourAttach(&(PVOID&)TrueInternetConnectA, (PVOID)::MyInternetConnectA);
    DetourAttach(&(PVOID&)TrueInternetConnectW, (PVOID)::MyInternetConnectW);

    if (!getConfig().enable_https)
    {
        DetourAttach(&(PVOID&)TrueHttpOpenRequestA, (PVOID)::MyHttpOpenRequestA);
        DetourAttach(&(PVOID&)TrueHttpOpenRequestW, (PVOID)::MyHttpOpenRequestW);
    }
}
