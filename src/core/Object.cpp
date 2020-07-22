#include <UDRefl/Object.h>

#include <UDRefl/TypeInfoMngr.h>

using namespace Ubpa::UDRefl;

TypeInfo* Object::GetTypeInfo() const {
	assert(id != static_cast<size_t>(-1));
	return TypeInfoMngr::Instance().GetTypeInfo(id);
}
