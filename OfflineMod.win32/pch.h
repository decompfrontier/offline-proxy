/**
* @file pch.h
* @author Arves100
* @date 30/10/2024
* @brief Precompiler headers
*/
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#define STRICT 1
#include <windows.h>
#include <wininet.h>
#include <winhttp.h>
#include <ShlObj.h> // SHxyxyxy

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include <string>

#include "proxyep.h"

constexpr static const wchar_t* APP_PATH = L"GimuServer";
