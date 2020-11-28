#pragma once

#include "Object.h"
#include "Util.h"

namespace Ubpa::UDRefl {
	class BaseInfo {
	public:
		constexpr BaseInfo(InheritCastFunctions funcs, bool is_polymorphic = false, bool is_virtual = false) noexcept :
			is_polymorphic{ is_polymorphic },
			is_virtual{ is_virtual },
			funcs{ Fill(funcs, is_polymorphic, is_virtual) }
		{}

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
		static constexpr InheritCastFunctions Fill(const InheritCastFunctions& funcs, bool is_polymorphic, bool is_virtual) noexcept {
			assert(funcs.static_derived_to_base);
			assert((is_virtual && !funcs.static_base_to_derived) || (!is_virtual && funcs.static_base_to_derived));
			assert((is_polymorphic && funcs.dynamic_base_to_derived) || (!is_polymorphic && !funcs.dynamic_base_to_derived));

			InheritCastFunctions rst;
			rst.static_derived_to_base = funcs.static_derived_to_base;

			if (!is_polymorphic) {
				rst.dynamic_base_to_derived = [](const void*) noexcept -> const void* {
					return nullptr;
				};
			}
			else
				rst.dynamic_base_to_derived = funcs.dynamic_base_to_derived;

			if (is_virtual) {
				rst.static_base_to_derived = [](const void*) noexcept -> const void* {
					return nullptr;
				};
			}
			else
				rst.static_base_to_derived = funcs.static_base_to_derived;

			return rst;
		}

		bool is_polymorphic;
		bool is_virtual;
		InheritCastFunctions funcs;
	};

	template<typename Derived, typename Base>
	static constexpr BaseInfo MakeBaseInfo() noexcept {
		return {
			inherit_cast_functions<Derived, Base>(),
			std::is_polymorphic_v<Base>,
			is_virtual_base_of_v<Base, Derived>
		};
	}
}
