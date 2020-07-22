#include <UDRefl/UDRefl.h>

#include <array>
#include <iostream>

//
//enum class Color : int {
//  RED = 0,
//  GREEN = 1,
//  BLUE = 2
//};
//

using namespace Ubpa::UDRefl;
using namespace std;

// TODO : better API

int main() {
	{ // register
		TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(0);
		type->size = sizeof(int);
		type->name = "enum Color";
		type->fields.data = {
			{ FieldList::enum_value,
				{
					Var::Init<int>(0)
					// no attrs
				}
			},
			{ "RED",
				{
					StaticVar{0}
					// no attrs
				}
			},
			{ "GREEN",
				{
					StaticVar{1}
					// no attrs
				}
			},
			{ "BLUE",
				{
					StaticVar{2}
					// no attrs
				}
			},
			{ FieldList::default_constructor,
				{
					Func{[](Object obj) {
						TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						type->fields.Set(FieldList::enum_value, obj, 0);
						cout << "[ " << FieldList::default_constructor << " ] construct " << type->name
							<< " @" << obj.Pointer() << endl;
					}}
					// no attrs
				}
			},
			{ "constructor",
				{
					Func{[](Object obj, int v) {
						TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						type->fields.Set(FieldList::enum_value, obj, v);
						cout << "[ constructor ] construct " << type->name
							<< " @" << obj.Pointer() << endl;
					}}
					// no attrs
				}
			},
			{ FieldList::destructor,
				{
					Func{[](Object obj) {
						TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						cout << "[ " << FieldList::destructor << " ] destruct " << type->name
							<< " @" << obj.Pointer() << endl;
					}}
					// no attrs
				}
			}
		};
	}

	// ======================

	TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto color = type->New("constructor", 1);

	// dump
	cout << type->name << endl;

	for (const auto& [name, field] : type->fields.data) {
		cout << name;
		/*
		if (auto pV = field.value.CastIf<Var>()) {
			cout << " : ";
			if (pV->TypeIs<int>())
				cout << pV->Get<int>(color);
			else
				cout << "[NOT SUPPORT]";
		}
		else if (auto pV = field.value.CastIf<StaticVar>()) {
			cout << " : ";
			if (pV->TypeIs<int>())
				cout << pV->Cast<int>();
			else
				cout << "[NOT SUPPORT]";
		}
		else if (auto pF = field.value.CastIf<Func>()) {
			cout << " [Func]";
		}
		*/
		visit([=](auto&& v) {
			using T = std::decay_t<decltype(v)>;
			if constexpr (std::is_same_v<T, Var>) {
				cout << " : ";
				if (v.TypeIs<int>())
					cout << v.Get<int>(color);
				else
					cout << "[NOT SUPPORT]";
			}
			else if constexpr (std::is_same_v<T, StaticVar>) {
				cout << " : ";
				if (v.TypeIs<int>())
					cout << v.Cast<int>();
				else
					cout << "[NOT SUPPORT]";
			}
			else if constexpr (std::is_same_v<T, Func>) {
				cout << " [Func]";
			}
			else
				static_assert(false, "non-exhaustive visitor!");
		}, field.value.data);
		cout << endl;
	}

	// enum

	// string <-> value
	std::string_view str = "RED";
	int value = 1;
	cout << str << " : " << type->fields.Get<int>(str) << endl;
	auto [name, field] = type->fields.FindStaticField(value);
	assert(field != nullptr);
	cout << value << " : " << name << endl;

	type->Delete(color);
}
