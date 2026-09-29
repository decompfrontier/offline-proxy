#include "pch.h"
#include "config.h"
#include <INIReader.h>
#include <locale>
#include <codecvt>

static ProxyConfig g_theConfig;

static constexpr size_t MAX_STRING = 255;

bool ProxyConfig::Load(const char* path)
{
	INIReader reader(path);
	if (reader.ParseError() < 0) {
		return false;
	}

	g_theConfig.enable_https = reader.GetBoolean("feature", "https", false);
	g_theConfig.enable_diagnostics = reader.GetBoolean("feature", "diagnostics", false);
	g_theConfig.enable_deploy_mode = reader.GetBoolean("feature", "deployMode", true);
	g_theConfig.fps_cap = (uint16_t)reader.GetInteger("config", "fps", 30);
	g_theConfig.port = (uint16_t)reader.GetInteger("connect", "port", 9960);
	g_theConfig.ip = reader.GetString("connect", "ip", "127.0.0.1");
	g_theConfig.ip_w = toWString(g_theConfig.ip);
	g_theConfig.log_file = toWString(reader.GetString("config", "logFile", "proxy.log"));

	std::string currentUrl = "";
	std::string name = "";

	for (size_t i = 0; i < MAX_STRING; i++)
	{
		name = "id" + std::to_string(i + 1);
		currentUrl = reader.GetString("allowlist", name, "");
		if (currentUrl.empty()) {
			break;
		}
		g_theConfig.skip_url_patch.emplace_back(currentUrl);
		const auto currentUrlW = toWString(currentUrl);
		g_theConfig.skip_url_patch_w.emplace_back(currentUrlW);
	}

	return true;
}

ProxyConfig& getConfig(void)
{
	return g_theConfig;
}

std::wstring ProxyConfig::toWString(const std::string& str)
{
	if (str.empty()) {
		return std::wstring();
	}

	try {
		// Use std::wstring_convert with UTF-8 codecvt facet (deprecated in C++17 but still available)
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		return converter.from_bytes(str);
	}
	catch (const std::range_error&) {
		// Fallback: simple byte-by-byte widening (treats input as ASCII/Latin-1)
		// Or rethrow depending on your needs
		return std::wstring(str.begin(), str.end());
	}
}
