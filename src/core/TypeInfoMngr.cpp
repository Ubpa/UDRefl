#include <UDRefl/TypeInfoMngr.h>

using namespace Ubpa::UDRefl;

TypeInfo& TypeInfoMngr::GetTypeInfo(size_t id) {
	auto target = id2typeinfo.find(id);
	if (target != id2typeinfo.end())
		return target->second;

	auto [iter, success] = id2typeinfo.try_emplace(id, id);
	assert(success);
	return iter->second;
}
