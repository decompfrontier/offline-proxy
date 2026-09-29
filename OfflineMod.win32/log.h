/**
* @file log.h
* @author Arves100
* @date 29/09/2026
* @brief Logging
*/
#pragma once

#include <mutex>

enum class MyLogLevel {
	Erro,
	Warn,
	Info,
	Dbug
};

struct MyLog
{
	MyLog() : mutex(), file(nullptr),
		toStdout(false) {}

	void Free(void)
	{
		if (file) {
			fflush(file);
			fclose(file);
			file = nullptr;
		}
	}

	bool Init(const wchar_t *path, bool toStdout);

	template <typename... Args>
	void Work(MyLogLevel lvl, const char* fmt, Args&&... args)
	{
		std::string buffer = "";
		appendTime(lvl, buffer);

		auto current = buffer.size();
		auto size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
		buffer.resize(current + size + 1);
		std::snprintf(&buffer[current], size, fmt, std::forward<Args>(args)...);
		buffer.resize(current + size);

		mutex.lock();
		fwrite(buffer.c_str(), buffer.size(), 1, file);
		fflush(file);

		if (toStdout) {
			// write to console
			fwrite(buffer.c_str(), buffer.size(), 1, stdout);
		}
		mutex.unlock();
	}

private:
	std::mutex mutex;
	FILE* file;
	bool toStdout;

	void appendTime(MyLogLevel lvl, std::string& data);
};

extern MyLog& getLog(void);

#define LOG_ERRO(x, ...) getLog().Work(MyLogLevel::Erro, x, ##__VA_ARGS__)
#define LOG_WARN(x, ...) getLog().Work(MyLogLevel::Warn, x, ##__VA_ARGS__)
#define LOG_INFO(x, ...) getLog().Work(MyLogLevel::Info, x, ##__VA_ARGS__)
#define LOG_DBUG(x, ...) getLog().Work(MyLogLevel::Dbug, x, ##__VA_ARGS__)

