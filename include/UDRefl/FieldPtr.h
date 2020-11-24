#pragma once

#include "Object.h"

#include <cassert>
#include <cstdint>

namespace Ubpa::UDRefl {
	class FieldPtr {
	public:
		FieldPtr(size_t objID, size_t valueID, size_t offset) :
			objID{ objID },
			valueID{ valueID },
			offset{ offset } {}

		ObjectPtr Map(ObjectPtr objptr) const noexcept {
			assert(objptr.GetID() == objID);
			return { valueID, reinterpret_cast<std::uint8_t*>(objptr.GetPtr()) + offset };
		}

		ConstObjectPtr Map(ConstObjectPtr objptr) const noexcept { return Map(reinterpret_cast<ObjectPtr&>(objptr)); }

	private:
		size_t objID;
		size_t valueID;
		size_t offset;
	};

	class ConstFieldPtr {
	public:
		ConstFieldPtr(size_t objID, size_t valueID, size_t offset) :
			objID{ objID },
			valueID{ valueID },
			offset{ offset } {}

		ConstObjectPtr Map(ConstObjectPtr objptr) const noexcept {
			assert(objptr.GetID() == objID);
			return { valueID, reinterpret_cast<const std::uint8_t*>(objptr.GetPtr()) + offset };
		}

	private:
		size_t objID;
		size_t valueID;
		size_t offset;
	};
}
