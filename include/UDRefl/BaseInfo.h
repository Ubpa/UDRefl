#pragma once

#include "Object.h"

namespace Ubpa::UDRefl {
	struct BaseInfo {
		size_t offset;
	};

	struct VirtualBaseInfo {
		const void* (*func)(const void*) noexcept;

		void* offset(void* ptr) const noexcept {
			assert(func);
			return const_cast<void*>(func(ptr));
		}

		const void* offset(const void* ptr) const noexcept {
			assert(func);
			return func(ptr);
		}
	};
}
