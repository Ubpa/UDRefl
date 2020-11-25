#pragma once

#include <string>
#include <deque>
#include <unordered_map>

namespace Ubpa::UDRefl {
	class NameRegistry {
	public:
		static NameRegistry& Instance() noexcept {
			static NameRegistry instance;
			return instance;
		}

		size_t Register(std::string_view name);
		size_t GetID(std::string_view name) const noexcept;
		bool IsRegistered(size_t ID) const noexcept { return ID < names.size(); }
		const auto& GetNameIDMap() const noexcept { return name2id; }
		std::string_view Nameof(size_t ID) const noexcept;

	private:
		std::deque<std::string> names;
		std::unordered_map<std::string_view, size_t> name2id;
	};
}
