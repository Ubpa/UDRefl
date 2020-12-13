#pragma once

#include <memory>

namespace Ubpa::UDRefl {
	using SharedBuffer = std::shared_ptr<void>;
	using SharedConstBuffer = std::shared_ptr<const void>;
}
