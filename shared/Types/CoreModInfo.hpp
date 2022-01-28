#pragma once

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include <string>
#include <optional>

namespace QModUtils {
	struct CoreModInfo {
		std::string id;
		std::string version;
		std::string downloadLink;
		std::string filename;

		CoreModInfo(const rapidjson::Value& info) {
			if (!info.IsObject()) return;

			if (info.HasMember("id") && info["id"].IsString()) id = info["id"].GetString();
			else id = "";

			if (info.HasMember("version") && info["version"].IsString()) version = info["version"].GetString();
			else version = "";

			if (info.HasMember("downloadLink") && info["downloadLink"].IsString()) downloadLink = info["downloadLink"].GetString();
			else downloadLink = "";

			if (info.HasMember("filename") && info["filename"].IsString()) filename = info["filename"].GetString();
			else filename = "";
		}
	};
}