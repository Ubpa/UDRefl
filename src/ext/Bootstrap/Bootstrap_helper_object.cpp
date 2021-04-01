#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_object() {
	Mngr.RegisterType<ArgPtrBuffer>();

	Mngr.RegisterType<ArgsView>();
	Mngr.AddConstructor<ArgsView, ArgPtrBuffer, std::span<const Type>>();
	Mngr.AddMethod<&ArgsView::Buffer>("Buffer");
	Mngr.AddMethod<&ArgsView::Types>("Types");

	Mngr.RegisterType<ObjectView>();
	Mngr.AddConstructor<ObjectView, Type, void*>();
	Mngr.AddConstructor<ObjectView, Type>();
	Mngr.AddConstructor<ObjectView, SharedObject>();
	Mngr.AddMethod<&ObjectView::GetType>("GetType");
	Mngr.AddMethod<&ObjectView::GetPtr>("GetPtr");

	Mngr.AddMethod<&ObjectView::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	Mngr.AddMethod<&ObjectView::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	Mngr.AddMethod<&ObjectView::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");
	Mngr.AddMethod<&ObjectView::StaticCast>("StaticCast");
	Mngr.AddMethod<&ObjectView::DynamicCast>("DynamicCast");

	Mngr.AddMethod<MemFuncOf<ObjectView, ObjectView(Name, FieldFlag)const>::get(&ObjectView::Var)>("Var");
	Mngr.AddMethod<MemFuncOf<ObjectView, ObjectView(Type, Name, FieldFlag)const>::get(&ObjectView::Var)>("Var");
	Mngr.AddMemberMethod("Var", [](const ObjectView& obj, Name field_name) { return obj.Var(field_name); });
	Mngr.AddMemberMethod("Var", [](const ObjectView& obj, Type base, Name field_name) { return obj.Var(base, field_name); });

	Mngr.AddMethod<&ObjectView::GetVars>("GetVars");
	Mngr.AddMemberMethod("GetVars", [](const ObjectView& obj) { return obj.GetVars(); });
	Mngr.AddMethod<&ObjectView::GetFields>("GetFields");
	Mngr.AddMemberMethod("GetFields", [](const ObjectView& obj) { return obj.GetFields(); });
	Mngr.AddMethod<&ObjectView::GetMethods>("GetMethods");
	Mngr.AddMemberMethod("GetMethods", [](const ObjectView& obj) { return obj.GetMethods(); });
	Mngr.AddMethod<&ObjectView::GetObjectTree>("GetObjectTree");
	Mngr.AddMemberMethod("GetObjectTree", [](const ObjectView& obj) { return obj.GetObjectTree(); });

	Mngr.AddMethod<&ObjectView::RemoveConst>("RemoveConst");
	Mngr.AddMethod<&ObjectView::RemoveLValueReference>("RemoveLValueReference");
	Mngr.AddMethod<&ObjectView::RemoveRValueReference>("RemoveRValueReference");
	Mngr.AddMethod<&ObjectView::RemoveReference>("RemoveReference");
	Mngr.AddMethod<&ObjectView::RemoveConstReference>("RemoveConstReference");

	Mngr.AddMethod<&ObjectView::AddConst>("AddConst");
	Mngr.AddMethod<&ObjectView::AddLValueReference>("AddLValueReference");
	Mngr.AddMethod<&ObjectView::AddLValueReferenceWeak>("AddLValueReferenceWeak");
	Mngr.AddMethod<&ObjectView::AddRValueReference>("AddRValueReference");
	Mngr.AddMethod<&ObjectView::AddConstLValueReference>("AddConstLValueReference");
	Mngr.AddMethod<&ObjectView::AddConstRValueReference>("AddConstRValueReference");

	Mngr.RegisterType<SharedObject>();
	Mngr.AddConstructor<SharedObject, Type, void*>();
	Mngr.AddConstructor<SharedObject, Type>();
	Mngr.AddConstructor<SharedObject, ObjectView>();
	Mngr.AddMethod<&SharedObject::Reset>("Reset");
	Mngr.AddMethod<&SharedObject::UseCount>("UseCount");
	Mngr.AddMethod<&SharedObject::IsObjectView>("IsObjectView");
	Mngr.AddMethod<&SharedObject::Swap>("Swap");

	Mngr.AddMethod<&SharedObject::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	Mngr.AddMethod<&SharedObject::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	Mngr.AddMethod<&SharedObject::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");
	Mngr.AddMethod<&SharedObject::StaticCast>("StaticCast");
	Mngr.AddMethod<&SharedObject::DynamicCast>("DynamicCast");
}
