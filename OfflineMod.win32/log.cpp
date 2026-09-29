/**
* @file log.cpp
* @author Arves100
* @date 29/09/2026
* @brief Logging
*/
#include "pch.h"
#include "log.h"
#include "util.h"
#include <time.h>

// msg to present new logs
static constexpr const char g_startupMsg[] = "\r\n--- offline proxy startup ---\r\n";

static constexpr const char *log_get_level_str(MyLogLevel level);

static MyLog g_theLog;

bool MyLog::Init(const wchar_t * path, bool toStdout)
{
std::wstring logPath = L"";

	MyLog::toStdout = toStdout;
	
	logPath = util_get_app_path();
	logPath += L"\\";
	logPath += path;

	_wfopen_s(&file, logPath.c_str(), L"a");
	if (file != nullptr) {
		fwrite(g_startupMsg, sizeof(g_startupMsg), 1, file);
		return true;
	}

	return false;
}

static constexpr const char* log_get_level_str(MyLogLevel level)
{
	switch (level) {
	case MyLogLevel::Dbug:
		return "DBUG";
	case MyLogLevel::Erro:
		return "ERRO";
	case MyLogLevel::Warn:
		return "WARN";
	default:
		break;
	}

	return "INFO";
}


void MyLog::appendTime(MyLogLevel lvl, std::string& data)
{
	char buffer[50] = {};
	auto now = time(nullptr);
	auto local_time = localtime(&now);
	_snprintf_s(buffer, sizeof(buffer), "\r\n[%s] [%02d/%02d/%04d %02d:%02d:%02d] >>", log_get_level_str(lvl),
		local_time->tm_mday,
		local_time->tm_mon + 1,
		local_time->tm_year + 1900,
		local_time->tm_hour,
		local_time->tm_min,
		local_time->tm_sec);

	data = buffer;
}

MyLog& getLog(void)
{
	return g_theLog;
}
