#pragma once

#include <string>

namespace QModUtils {
	struct Dependency {
		std::string id;
		std::string version;
		std::string downloadIfMissing;
	};
}