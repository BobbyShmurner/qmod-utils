#pragma once

#include "libcurl/shared/curl.h"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/error/error.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/error/en.h"

#include <string>

namespace QModUtils {
	namespace WebUtils {
		// Write Function (github.com/darknight1050/SongDownloader/blob/d2341b5b8f09183cb5b5aec253a8aff295288052/src/Utils/WebUtils.cpp#L81-L92)
		inline size_t WriteData(void *contents, size_t size, size_t nmemb, std::string* s)
		{
			std::size_t newLength = size * nmemb;
			try {
				s->append((char*)contents, newLength);
			} catch(std::bad_alloc &e) {
				//handle memory problem
				getLogger().critical("Failed to allocate string of size: %lu", newLength);
				return 0;
			}
			return newLength;
		}

		inline bool DownloadFile(std::string url, std::string downloadFileLoc) {
			CURL* curl = curl_easy_init();
			std::string val;

			if (curl) {
				getLogger().info("Downloading file \"%s\"", url.c_str());
				
				CURLcode res;

				std::system("mkdir -p \"/sdcard/BMBFData/Mods/Temp/Downloads/\"");

				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

				// Follow HTTP redirects if necessary.
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

				res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);

				if (res != CURLE_OK) {
					getLogger().error("Curl Failed to download \"%s\"! Error: (%i) %s", url.c_str(), res, curl_easy_strerror(res));

					return false;
				}

				std::ofstream file(downloadFileLoc);
				file << val;
				file.close();
			} else {
				getLogger().error("Curl failed to initialize for \"%s\". No futher info was given", url.c_str());
				return false;
			}

			return true;
		}

		inline std::string GetData(std::string url, std::function<void(std::string)> onComplete = nullptr) {
			CURL* curl = curl_easy_init();
			std::string val;

			if (curl != nullptr) {
				getLogger().info("Getting data from \"%s\"", url.c_str());
				
				CURLcode res;

				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &val);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

				// Follow HTTP redirects if necessary.
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

				res = curl_easy_perform(curl);

				curl_easy_cleanup(curl);

				if (res != CURLE_OK) {
					getLogger().error("Curl Failed to Get from Url \"%s\"! Error: (%i) %s", url.c_str(), res, curl_easy_strerror(res));

					return "";
				}
			} else {
				getLogger().error("Curl failed to initialize for url \"%s\". No futher info was given", url.c_str());
				return "";
			}

			getLogger().info("Got Data from \"%s\"!", url.c_str());

			if (onComplete != nullptr) onComplete(val);
			return val;
		}

		inline void GetDataAsync(std::string url, std::function<void(std::string)> onComplete = nullptr) {
			std::thread([url, onComplete]() {
				GetData(url, onComplete);
			}).detach();
		}

		inline std::optional<rapidjson::Document> GetJSONData(std::string url) {
			std::string data = GetData(url);

			rapidjson::Document document;

			if (!document.Parse(data).HasParseError()) {
				return document;
			} else {
				return std::nullopt;
			}
		}
	}
}