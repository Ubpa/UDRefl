#include <UDRefl/Registry.h>

#include <cassert>

using namespace Ubpa::UDRefl;

size_t Registry::Register(std::string_view name) {
	assert(!name.empty());

	auto target = name2id.find(name);
	if (target != name2id.end())
		return target->second;

	size_t idx = names.size();
	names.push_back(std::string{ name });
	name2id.emplace_hint(target, std::pair{ std::string_view{names[idx]}, idx });

	return idx;
}

size_t Registry::GetID(std::string_view name) const noexcept {
	auto target = name2id.find(name);
	return target == name2id.end() ? static_cast<size_t>(-1) : target->second;
}

std::string_view Registry::Nameof(size_t ID) const noexcept
{
	if (IsRegistered(ID))
		return names[ID];
	else
		return {};
}
