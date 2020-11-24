#pragma once

#include "Object.h"

#include <cassert>

namespace Ubpa::UDRefl {
	class FieldPtr {
		struct Void {};
	public:
		template<typename Obj, typename Value>
		FieldPtr(size_t objID, size_t valueID, Value Obj::* ptr) :
			objID{ objID },
			valueID{ valueID },
			ptr{ reinterpret_cast<Void Void::*>(ptr) } {}

		ObjectPtr Map(ObjectPtr objptr) const noexcept {
			assert(objptr.GetID() == objID);
			return { valueID, &(objptr.As<Void>().*ptr) };
		}

		ConstObjectPtr Map(ConstObjectPtr objptr) const noexcept { return Map(reinterpret_cast<ObjectPtr&>(objptr)); }

	private:
		size_t objID;
		size_t valueID;
		Void Void::* ptr;
	};
}
