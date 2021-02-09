#pragma once

#include "FieldPtr.h"
#include "MethodPtr.h"

#include <set>

namespace Ubpa::UDRefl {
	using Attr = SharedObject;
	struct AttrLess {
		using is_transparent = int;
		bool operator()(const Attr& lhs, const Attr& rhs) const noexcept { return lhs.GetType() < rhs.GetType(); }
		bool operator()(const Attr& lhs, const Type& rhs) const noexcept { return lhs.GetType() < rhs;           }
		bool operator()(const Type& lhs, const Attr& rhs) const noexcept { return lhs           < rhs.GetType(); }
	};
	using AttrSet = std::set<Attr, AttrLess>;

	class BaseInfo {
	public:
		BaseInfo() noexcept = default;
		BaseInfo(InheritCastFunctions funcs, bool is_polymorphic = false, bool is_virtual = false) :
			is_polymorphic{ is_polymorphic },
			is_virtual{ is_virtual },
			funcs{ std::move(funcs) }
		{
			assert(this->funcs.static_derived_to_base);
			assert((is_virtual && !this->funcs.static_base_to_derived) || (!is_virtual && this->funcs.static_base_to_derived));
			assert((is_polymorphic&& this->funcs.dynamic_base_to_derived) || (!is_polymorphic && !this->funcs.dynamic_base_to_derived));
		}

		bool IsVirtual()     const noexcept { return is_virtual; }
		bool IsPolymorphic() const noexcept { return is_virtual; }

		void* StaticCast_DerivedToBase (void* ptr) const noexcept { return funcs.static_derived_to_base(ptr); }
		// require non virtual
		void* StaticCast_BaseToDerived (void* ptr) const noexcept { return IsVirtual() ? nullptr : funcs.static_base_to_derived(ptr); }
		// require polymorphic
		void* DynamicCast_BaseToDerived(void* ptr) const noexcept { return IsPolymorphic() ? funcs.dynamic_base_to_derived(ptr) : nullptr; }
	private:
		bool is_polymorphic;
		bool is_virtual;
		InheritCastFunctions funcs;
	};

	struct FieldInfo {
		FieldPtr fieldptr;
		AttrSet attrs;
	};

	struct MethodInfo {
		MethodPtr methodptr;
		AttrSet attrs;
	};

	struct TypeInfo {
		size_t size;
		size_t alignment;
		std::unordered_map<Name, FieldInfo> fieldinfos;
		std::unordered_multimap<Name, MethodInfo> methodinfos;
		std::unordered_map<Type, BaseInfo> baseinfos;
		AttrSet attrs;
	};
}
