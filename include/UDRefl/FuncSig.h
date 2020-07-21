#pragma once

#include <vector>
#include <typeinfo>
#include <utility>

namespace Ubpa::UDRefl {
	class FuncSig {
	public:
		template<typename... Hashcodes> // size_t
		FuncSig(Hashcodes... hashcodes) : argHashcodes{ hashcodes... } {}

		template<typename... Args>
		static FuncSig Init() {
			return { typeid(Args).hash_code()... };
		}

		template<typename Arg>
		void Append() {
			argHashcodes.push_back(typeid(Arg).hash_code());
		}

		template<typename... Args>
		bool Is() const noexcept {
			return Is<Args...>(std::make_index_sequence<sizeof...(Args)>{});
		}

		bool operator==(const FuncSig& rhs) const noexcept {
			const size_t n = argHashcodes.size();
			if (rhs.argHashcodes.size() != n)
				return false;
			for (size_t i = 0; i < n; i++) {
				if (argHashcodes[i] != rhs.argHashcodes[i])
					return false;
			}
			return true;
		}

		bool operator<(const FuncSig& rhs) const noexcept {
			const size_t n = argHashcodes.size();
			if (rhs.argHashcodes.size() != n)
				return n < rhs.argHashcodes.size();
			for (size_t i = 0; i < n; i++) {
				if (argHashcodes[i] == rhs.argHashcodes[i])
					continue;
				return argHashcodes[i] < rhs.argHashcodes[i];
			}
			return false;
		}
	private:
		friend class ArgList;

		template<typename... Args, size_t... Ns>
		bool Is(std::index_sequence<Ns...>) const noexcept {
			if (sizeof...(Args) != argHashcodes.size())
				return false;
			return ((argHashcodes[Ns] == typeid(std::tuple_element_t<Ns, std::tuple<Args...>>).hash_code()) &&...);
		}

		std::vector<size_t> argHashcodes;
	};
}
