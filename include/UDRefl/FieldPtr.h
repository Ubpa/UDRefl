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

		ObjectPtr Map(ObjectPtr obj_ptr) const noexcept {
			assert(obj_ptr.GetID() == objID);
			return { valueID, &(obj_ptr.As<Void>().*ptr) };
		}

		ConstObjectPtr Map(ConstObjectPtr obj_ptr) const noexcept { return Map(reinterpret_cast<ObjectPtr&>(obj_ptr)); }

	private:
		size_t objID;
		size_t valueID;
		Void Void::* ptr;
	};
}
