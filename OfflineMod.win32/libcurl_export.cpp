/**
* @file curl_exp.cpp
* @author Arves100
* @date 30/10/2024
* @brief libcurl exports
*/
#include "pch.h"
#include "log.h"
#include "config.h"
#include <curl/curl.h>

extern "C" CURLcode Curl_vsetopt(struct Curl_easy* data, CURLoption option, va_list arg);
extern "C" CURLcode Curl_getinfo(struct Curl_easy* data, CURLINFO info, ...);

// console only
static void dump(const char* text,
	FILE* stream, unsigned char* ptr, size_t size)
{
	size_t i;
	size_t c;
	unsigned int width = 0x10;

	fprintf(stream, "curl: stream %s %10.10ld bytes (0x%8.8lx)\n", text, (long)size, (long)size);

	for (i = 0; i < size; i += width) {
		fprintf(stream, "%4.4lx: ", (long)i);

		/* show hex to the left */
		for (c = 0; c < width; c++) {
			if (i + c < size)
				fprintf(stream, "%02x ", ptr[i + c]);
			else
				fputs("   ", stream);
		}

		/* show data on the right */
		for (c = 0; (c < width) && (i + c < size); c++) {
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			fputc(x, stream);
		}

		fputc('\n', stream); /* newline */
	}
}

static int debug_callback(CURL* handle,
	curl_infotype type,
	char* data,
	size_t size,
	void* clientp)
{
	const char* text;
	(void)handle; /* prevent compiler warning */
	(void)clientp;

	switch (type) {
	case CURLINFO_TEXT:
		fputs("== Info: ", stdout);
		fwrite(data, size, 1, stdout);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, stdout, (unsigned char*)data, size);
	return 0;
}

extern "C" CURL* proxy_curl_init()
{
	auto c = curl_easy_init();
	if (!c)
		return c;

	long q = CURLUSESSL_NONE;
	curl_easy_setopt(c, CURLOPT_USE_SSL, q);
	q = 0;
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, q);

	if (getConfig().enable_diagnostics) {
		curl_easy_setopt(c, CURLOPT_DEBUGFUNCTION, debug_callback);
	}

	return c;
}

extern "C" CURLcode proxy_curl_setopt(CURL* handle, CURLoption option, ...)
{
long q;
va_list arg;
CURLcode ret;

	if (!handle)
		return CURLE_BAD_FUNCTION_ARGUMENT;


	if (getConfig().enable_diagnostics) {
		fprintf(stdout, "proxy: curl setopt -> %x", option);
	}

	switch (option)
	{
		// Disable SSL usage and verification
	case CURLOPT_USE_SSL:
		q = CURLUSESSL_NONE;
		curl_easy_setopt(handle, CURLOPT_USE_SSL, q);
		return CURLE_OK;
	case CURLOPT_SSL_VERIFYHOST:
		q = 0;
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, q);
		return CURLE_OK;
	case CURLOPT_DEBUGDATA:
		return CURLE_OK;
	case CURLOPT_DEBUGFUNCTION:
		return CURLE_OK;
	}


	va_start(arg, option);

	if (option == CURLOPT_URL && getConfig().enable_diagnostics)
	{
		va_list arg2;
		char* ip;
		va_start(arg2, option);
		ip = va_arg(arg2, char*);
		fprintf(stdout, "proxy: curl connecting to %s", ip);
		va_end(arg2);
	}

	ret = Curl_vsetopt((struct Curl_easy*)handle, option, arg);
	va_end(arg);

	return ret;
}

extern "C" char* proxy_easy_escape(CURL* curl, const char* string, int length)
{
	return curl_easy_escape(curl, string, length);
}

extern "C" void proxy_easy_cleanup(CURL* handle)
{
	curl_easy_cleanup(handle);
}

extern "C" const char* proxy_easy_strerror(CURLcode errornum)
{
	const auto& x = curl_easy_strerror(errornum);
	LOG_WARN("curl error: %s", x);
	return x;
}

extern "C" CURLcode proxy_easy_perform(CURL* easy_handle)
{
	return  curl_easy_perform(easy_handle);
}

extern "C" CURLcode proxy_easy_getinfo(CURL* data, CURLINFO info, ...)
{
va_list arg;
void* paramp;
CURLcode result;

	va_start(arg, info);
	paramp = va_arg(arg, void*);

	result = Curl_getinfo((struct Curl_easy*)data, info, paramp);

	va_end(arg);
	return result;
}

extern "C" struct curl_slist* proxy_slist_append(struct curl_slist* list,
	const char* string)
{
	return curl_slist_append(list, string);
}

extern "C" void proxy_slist_free_all(struct curl_slist* list)
{
	curl_slist_free_all(list);
}

