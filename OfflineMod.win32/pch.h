/**
* @file pch.h
* @author Arves100
* @date 30/10/2024
* @brief Precompiler headers
*/
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <wininet.h>

#ifndef __MINGW_BUILD__
#include <winhttp.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
