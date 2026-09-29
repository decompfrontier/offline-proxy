/**
* @file patch.cpp
* @author Arves100
* @date 29/09/2026
* @brief Patch entrypoint
*/
#include "pch.h"
#include "patch.h"
#include <detours/detours.h>

static PatchAPI g_theAPI;

bool PatchAPI::Attach(void)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	http.Attach();
	
	if (!fpscap.Attach())
	{
		DetourTransactionAbort();
		return false;
	}

	DetourTransactionCommit();
	return true;
}

void PatchAPI::Detach(void)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	http.Detach();
	fpscap.Detach();
	DetourTransactionCommit();
}

PatchAPI& getAPI(void)
{
	return g_theAPI;
}
