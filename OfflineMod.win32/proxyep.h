/**
* @file proxyep.h
* @author Arves100
* @date 29/09/2026
* @brief gimuserver entrypoints
*/
#pragma once

#define GIMUSERVER_DLL_NAME L"gimuserver.dll"

typedef void (WINAPI* GimuServerStartup)(unsigned int port);
typedef void (WINAPI* GimuServerShutdown)(void);
typedef void (WINAPI* GimuServerJoinThread)(void);

struct GimuServerAPI
{
	GimuServerAPI() :
		Startup(nullptr),
		Shutdown(nullptr),
		Join(nullptr),
		m_dll(nullptr) {}

	void Free(void) 
	{
		if (m_dll != nullptr)
		{
			Shutdown();
			Join();
			FreeLibrary(m_dll);
			m_dll = nullptr;
			Startup = nullptr;
			Shutdown = nullptr;
			Join = nullptr;
		}
	}

	bool Load(const wchar_t* path)
	{
		m_dll = LoadLibraryW(path);
		if (m_dll) {
			Startup = (GimuServerStartup)GetProcAddress(m_dll, "GimuServerStartup");
			Shutdown = (GimuServerShutdown)GetProcAddress(m_dll, "GimuServerShutdown");
			Join = (GimuServerJoinThread)GetProcAddress(m_dll, "GimuServerJoinThread");
		}

		return Startup != nullptr && Shutdown != nullptr &&
			Join != nullptr;
	}

	GimuServerStartup Startup;
	GimuServerShutdown Shutdown;
	GimuServerJoinThread Join;

private:
	HMODULE m_dll;
};
