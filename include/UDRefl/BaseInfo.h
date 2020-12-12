#pragma once

#include "Util.h"

namespace Ubpa::UDRefl {
	class BaseInfo {
	public:
		BaseInfo(InheritCastFunctions funcs, bool is_polymorphic = false, bool is_virtual = false) noexcept :
			is_polymorphic{ is_polymorphic },
			is_virtual{ is_virtual },
			funcs{ std::move(funcs) }
		{
			assert(this->funcs.static_derived_to_base);
			assert((is_virtual && !this->funcs.static_base_to_derived) || (!is_virtual && this->funcs.static_base_to_derived));
			assert((is_polymorphic&& this->funcs.dynamic_base_to_derived) || (!is_polymorphic && !this->funcs.dynamic_base_to_derived));
		}

		bool IsVirtual() const noexcept { return is_virtual; }
		bool IsPolymorphic() const noexcept { return is_virtual; }

		void* StaticCast_DerivedToBase(void* ptr) const noexcept {
			return const_cast<void*>(funcs.static_derived_to_base(ptr));
		}

		const void* StaticCast_DerivedToBase(const void* ptr) const noexcept {
			return funcs.static_derived_to_base(ptr);
		}

		// assert !IsVirtual()
		void* StaticCast_BaseToDerived(void* ptr) const noexcept {
			assert(!IsVirtual());
			return const_cast<void*>(funcs.static_base_to_derived(ptr));
		}

		// assert !IsVirtual()
		const void* StaticCast_BaseToDerived(const void* ptr) const noexcept {
			assert(!IsVirtual());
			return funcs.static_base_to_derived(ptr);
		}

		// assert IsPolymorphic
		void* DynamicCast_BaseToDerived(void* ptr) const noexcept {
			assert(IsPolymorphic());
			return const_cast<void*>(funcs.dynamic_base_to_derived(ptr));
		}

		// assert IsPolymorphic
		const void* DynamicCast_BaseToDerived(const void* ptr) const noexcept {
			assert(IsPolymorphic());
			return funcs.dynamic_base_to_derived(ptr);
		}
		
	private:
		bool is_polymorphic;
		bool is_virtual;
		InheritCastFunctions funcs;
	};
}
