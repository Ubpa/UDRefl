#pragma once

#include "Object.h"
#include "Util.h"

namespace Ubpa::UDRefl {
	class FieldPtr {
	public:
		constexpr FieldPtr(size_t valueID, size_t offset) :
			valueID{ valueID },
			offset{ offset } {}

		constexpr size_t GetValueID() const noexcept { return valueID; }

		constexpr ObjectPtr Map(void* obj) const noexcept { return { valueID, forward_offset(obj, offset) }; }
		constexpr ConstObjectPtr Map(const void* objptr) const noexcept { return Map(const_cast<void*>(objptr)); }

	private:
		size_t valueID;
		size_t offset;
	};

	class ConstFieldPtr {
	public:
		constexpr ConstFieldPtr(size_t valueID, size_t offset) :
			valueID{ valueID },
			offset{ offset } {}

		constexpr size_t GetValueID() const noexcept { return valueID; }

		constexpr ConstObjectPtr Map(const void* objptr) const noexcept { return { valueID, forward_offset(objptr, offset) }; }

	private:
		size_t valueID;
		size_t offset;
	};
}
