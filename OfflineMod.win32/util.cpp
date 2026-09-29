/**
* @file util.cpp
* @author Arves100
* @date 29/09/2026
* @brief Random utilities
*/
#include "pch.h"

static std::wstring g_path = L"";

bool util_init(void)
{
HRESULT hr;
PWSTR shPath = NULL;

	hr = SHGetKnownFolderPath(
		FOLDERID_LocalAppData,
		0,
		NULL,
		&shPath
	);

	if (SUCCEEDED(hr))
	{
		g_path = shPath;
		g_path += L"\\";
		g_path += APP_PATH;

		CoTaskMemFree(shPath);
		return true;
	}
	else if (shPath != NULL) {
		CoTaskMemFree(shPath);
		shPath = NULL;
	}

	return false;
}

LPCWSTR util_get_app_path(void) {
	return g_path.c_str();
}
