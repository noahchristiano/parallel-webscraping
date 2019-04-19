#pragma once
#include <curl/curl.h>

typedef struct Webpage {
	char *memory;
	size_t size;
} webpage;

class Request {
public:
	Request() {
		curl_global_init(CURL_GLOBAL_ALL);
	}

	~Request() {
		curl_global_cleanup();
	}

	// credit: libcurl https://curl.haxx.se/libcurl/c/getinmemory.html
	static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
		size_t realsize = size * nmemb;
		webpage *mem = (webpage *) userp;

		mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
		if (mem->memory == NULL) {
			/* out of memory! */ 
			fprintf(stderr, "not enough memory (realloc returned NULL)\n");
			exit(1);
		}

		memcpy(&(mem->memory[mem->size]), contents, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;

		return realsize;
	}

	// get body of webpage
	// 0 fail, 1 succeed
	int get(char *url, webpage *chunk) {
		CURL *curl;
		CURLcode res;

		chunk->memory = (char *) malloc(1);
		chunk->size = 0;

		if(!(curl = curl_easy_init())) {
			fprintf(stderr, "error: curl_easy_init\n");
			return 0;
		}

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) chunk);
		// curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "error: curl_easy_perform %s\n", 
				curl_easy_strerror(res));
			return 0;
		}
		curl_easy_cleanup(curl);

		return 1;
	}
};
