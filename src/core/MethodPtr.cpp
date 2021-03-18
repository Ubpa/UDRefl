#include <UDRefl/MethodPtr.hpp>

using namespace Ubpa::UDRefl;

MethodPtr::MethodPtr(Func func, MethodFlag flag, Type result_type, ParamList paramList)
	: func{ std::move(func) }, flag{ flag }, result_type{ result_type }, paramList{ std::move(paramList) }
{ assert(enum_single(flag)); }

bool MethodPtr::IsMatch(std::span<const Type> argTypes) const noexcept {
	const std::size_t n = paramList.size();
	if (argTypes.size() != n)
		return false;

	for (std::size_t i = 0; i < n; i++) {
		if (paramList[i] == argTypes[i] || paramList[i].Is<ObjectView>())
			continue;
	}

	return true;
}

void MethodPtr::Invoke(void* obj, void* result_buffer, ArgsView args) const {
	assert(IsMatch(args.Types()));
	func(obj, result_buffer, args);
};

void MethodPtr::Invoke(const void* obj, void* result_buffer, ArgsView args) const {
	assert(IsMatch(args.Types()));
	if (flag == MethodFlag::Variable)
		return;
	func(const_cast<void*>(obj), result_buffer, args);
};

void MethodPtr::Invoke(void* result_buffer, ArgsView args) const {
	assert(IsMatch(args.Types()));
	if (flag != MethodFlag::Static)
		return;
	func(nullptr, result_buffer, args);
};
