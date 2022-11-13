#pragma once

#include "pdbparse.h"
#include "common.h"

#include <filesystem>

namespace hdd {
	enum class Error {
		Success,
		AccessDenied,
		NotFound
	};

	Error SetupHDD();
}
