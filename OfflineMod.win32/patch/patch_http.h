/**
* @file patch_http.h
* @author Arves100
* @date 29/09/2026
* @brief HTTP patching API
*/
#pragma once

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

struct PatchHTTPAPI
{
    PatchHTTPAPI();

    HINTERNET MyInternetConnectA(
        _In_ HINTERNET     hInternet,
        _In_ LPCSTR        lpszServerName,
        _In_ INTERNET_PORT nServerPort,
        _In_ LPCSTR        lpszUserName,
        _In_ LPCSTR        lpszPassword,
        _In_ DWORD         dwService,
        _In_ DWORD         dwFlags,
        _In_ DWORD_PTR     dwContext
    );

    HINTERNET MyInternetConnectW(
        _In_ HINTERNET     hInternet,
        _In_ LPCWSTR        lpszServerName,
        _In_ INTERNET_PORT nServerPort,
        _In_ LPCWSTR        lpszUserName,
        _In_ LPCWSTR        lpszPassword,
        _In_ DWORD         dwService,
        _In_ DWORD         dwFlags,
        _In_ DWORD_PTR     dwContext
    );

    HINTERNET MyHttpOpenRequestA(
        _In_ HINTERNET hConnect,
        _In_ LPCSTR    lpszVerb,
        _In_ LPCSTR    lpszObjectName,
        _In_ LPCSTR    lpszVersion,
        _In_ LPCSTR    lpszReferrer,
        _In_ LPCSTR* lplpszAcceptTypes,
        _In_ DWORD     dwFlags,
        _In_ DWORD_PTR dwContext
    );

    HINTERNET MyHttpOpenRequestW(
        _In_ HINTERNET hConnect,
        _In_ LPCWSTR   lpszVerb,
        _In_ LPCWSTR   lpszObjectName,
        _In_ LPCWSTR   lpszVersion,
        _In_ LPCWSTR   lpszReferrer,
        _In_ LPCWSTR* lplpszAcceptTypes,
        _In_ DWORD     dwFlags,
        _In_ DWORD_PTR dwContext
    );

    void Attach(void);
    void Detach(void);

private:
    void PatchSecurityOptions(HINTERNET hInternet);

	InternetConnectW_t TrueInternetConnectW;
	InternetConnectA_t TrueInternetConnectA;
	HttpOpenRequestA_t TrueHttpOpenRequestA;
	HttpOpenRequestW_t TrueHttpOpenRequestW;
};
