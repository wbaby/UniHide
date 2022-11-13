#pragma once

#include "pdbparse.h"

namespace hdd {
	enum class Error {
		Success,
		AccessDenied,
		NotFound
	};

	Error SetupHDD();
}
