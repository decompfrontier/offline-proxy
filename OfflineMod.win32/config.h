/**
* @file config.h
* @author Arves100
* @date 29/09/2026
* @brief Configuration for proxy
*/
#pragma once

#include <string>
#include <vector>

struct ProxyConfig
{
	std::vector<std::string> skip_url_patch;
	std::vector<std::wstring> skip_url_patch_w;
	std::wstring log_file;
	std::string ip;
	std::wstring ip_w;
	uint16_t port;
	uint16_t fps_cap;
	bool enable_https;
	bool enable_diagnostics;
	bool enable_deploy_mode;

	ProxyConfig() : port(0),
		fps_cap(0),
		enable_https(false),
		enable_diagnostics(false),
		enable_deploy_mode(false)
	{
	}


	bool Load(const char *path);

private:
	static std::wstring toWString(const std::string& ptr);
};

extern ProxyConfig& getConfig(void);
