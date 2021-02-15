#include <UDRefl/MethodPtr.h>

using namespace Ubpa::UDRefl;

MethodPtr::MethodPtr(Func func, MethodFlag flag, Type result_type, ParamList paramList)
	: func{ std::move(func) }, flag{ flag }, result_type{ result_type }, paramList{ std::move(paramList) }
{ assert(enum_single(flag)); }

void MethodPtr::Invoke(void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	func(obj, result_buffer, { argptr_buffer,paramList });
};

void MethodPtr::Invoke(const void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	if (flag == MethodFlag::Variable)
		return;
	func(const_cast<void*>(obj), result_buffer, { argptr_buffer,paramList });
};

void MethodPtr::Invoke(void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	if (flag != MethodFlag::Static)
		return;
	func(nullptr, result_buffer, { argptr_buffer,paramList });
};
