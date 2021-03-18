#include "ReflMngrInitUtil.hpp"

using namespace Ubpa::UDRefl;

namespace Ubpa::UDRefl::details {
	template<typename To, typename From>
	void AddConvertCtor(ReflMngr& mngr) {
		static_assert(std::is_convertible_v<From, To>);
		if constexpr (!std::is_same_v<To, From>) {
			mngr.AddMemberMethod(
				NameIDRegistry::Meta::ctor,
				[](To& obj, const From& value) {
					new(&obj)To{ static_cast<To>(value) };
				}
			);
		}
	}

	template<typename T>
	void RegisterArithmeticConvertion(ReflMngr& mngr) {
		AddConvertCtor<T, bool>(mngr);
		AddConvertCtor<T, std::int8_t>(mngr);
		AddConvertCtor<T, std::int16_t>(mngr);
		AddConvertCtor<T, std::int32_t>(mngr);
		AddConvertCtor<T, std::int64_t>(mngr);
		AddConvertCtor<T, std::uint8_t>(mngr);
		AddConvertCtor<T, std::uint16_t>(mngr);
		AddConvertCtor<T, std::uint32_t>(mngr);
		AddConvertCtor<T, std::uint64_t>(mngr);
		AddConvertCtor<T, float>(mngr);
		AddConvertCtor<T, double>(mngr);
	}
}

void Ubpa::UDRefl::details::ReflMngrInitUtil_4(ReflMngr& mngr) {
	details::RegisterArithmeticConvertion<bool>(mngr);
	details::RegisterArithmeticConvertion<std::int8_t>(mngr);
	details::RegisterArithmeticConvertion<std::int16_t>(mngr);
	details::RegisterArithmeticConvertion<std::int32_t>(mngr);
	details::RegisterArithmeticConvertion<std::int64_t>(mngr);
	details::RegisterArithmeticConvertion<std::uint8_t>(mngr);
	details::RegisterArithmeticConvertion<std::uint16_t>(mngr);
	details::RegisterArithmeticConvertion<std::uint32_t>(mngr);
	details::RegisterArithmeticConvertion<std::uint64_t>(mngr);
	details::RegisterArithmeticConvertion<float>(mngr);
	details::RegisterArithmeticConvertion<double>(mngr);
}
