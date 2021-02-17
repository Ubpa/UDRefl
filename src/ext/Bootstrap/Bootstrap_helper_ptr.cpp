#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ptr() {
	// FieldPtr.h

	Mngr->RegisterType<FieldPtr>();
	Mngr->AddConstructor<FieldPtr, Type, std::size_t>();
	Mngr->AddConstructor<FieldPtr, Type, void*>();
	Mngr->AddConstructor<FieldPtr, ObjectView>();
	Mngr->AddConstructor<FieldPtr, SharedObject>();
	Mngr->AddMethod<&FieldPtr::GetType>("GetType");
	Mngr->AddMethod<&FieldPtr::IsBasic>("IsBasic");
	Mngr->AddMethod<&FieldPtr::IsVirtual>("IsVirtual");
	Mngr->AddMethod<&FieldPtr::IsStatic>("IsStatic");
	Mngr->AddMethod<&FieldPtr::IsDynamicShared>("IsDynamicShared");
	Mngr->AddMethod<&FieldPtr::IsDyanmicBuffer>("IsDyanmicBuffer");
	Mngr->AddMethod<&FieldPtr::IsOwned>("IsOwned");
	Mngr->AddMethod<&FieldPtr::IsUnowned>("IsUnowned");
	Mngr->AddMethod<&FieldPtr::GetFieldFlag>("GetFieldFlag");
	Mngr->AddMethod<MemFuncOf<FieldPtr, ObjectView()noexcept>::get(&FieldPtr::Var)>("Var");
	Mngr->AddMethod<MemFuncOf<FieldPtr, ObjectView(void*)>::get(&FieldPtr::Var)>("Var");

	// MethodPtr.h

	Mngr->RegisterType<ParamList>();

	Mngr->RegisterType<ArgPtrBuffer>();

	Mngr->RegisterType<ArgsView>();
	Mngr->AddConstructor<ArgsView, ArgPtrBuffer, const ParamList&>();
	Mngr->AddMethod<&ArgsView::GetBuffer>("GetBuffer");
	Mngr->AddMethod<&ArgsView::GetParamList>("GetParamList");
	Mngr->AddMethod<&ArgsView::At>("At");

	Mngr->RegisterType<MethodPtr::Func>();
	Mngr->RegisterType<MethodPtr>();
	Mngr->AddConstructor<MethodPtr, MethodPtr::Func, MethodFlag, Type, ParamList>();
	Mngr->AddMethod<&MethodPtr::GetMethodFlag>("GetMethodFlag");
	Mngr->AddMethod<&MethodPtr::GetParamList>("GetParamList");
	Mngr->AddMethod<&MethodPtr::GetResultType>("GetResultType");
	Mngr->AddMethod<&MethodPtr::IsDistinguishableWith>("IsDistinguishableWith");
}