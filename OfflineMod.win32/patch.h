/**
* @file patch.h
* @author Arves100
* @date 29/09/2026
* @brief Patch entrypoint
*/
#pragma once

#include "config.h"
#include "patch/patch_fps.h"
#include "patch/patch_http.h"

struct PatchAPI
{
	bool Attach(void);
	void Detach(void);

	PatchFPSCAP fpscap;
	PatchHTTPAPI http;
};

extern PatchAPI& getAPI(void);
