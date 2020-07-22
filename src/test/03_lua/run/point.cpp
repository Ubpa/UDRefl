#include "point.h"

#include <array>
#include <iostream>

using namespace Ubpa::UDRefl;
using namespace std;

void Ubpa::Point_init() {
	{ // register
		TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(0);
		type->size = 2 * sizeof(float);
		type->name = "struct Point";
		type->attrs.data = {
			{"info", std::string("hello world")}
		};
		type->fields.data = {
			{ "x",
				{
					Var::Init<float>(0),
					AttrList{{{"not_serialize", Attr{}}}}
				}
			},
			{ "y",
				{
					Var::Init<float>(sizeof(float)),
					AttrList{{{"range", std::pair<float, float>{ 0.f, 10.f }}}}
				}
			},
			{ "num",
				{
					StaticVar{static_cast<size_t>(0)}
					// no attrs
				}
			},
			{ FieldList::default_constructor,
				{
					Func{[](Object obj) {
						TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						type->fields.Set("x", obj, 0.f);
						type->fields.Set("y", obj, 0.f);
						type->fields.Get<size_t>("num") += 1;
						cout << "[ " << FieldList::default_constructor << " ] construct " << type->name
							<< " @" << obj.Pointer() << endl;
					}}
			// no attrs
		}
	},
	{ "constructor",
		{
			Func{[](Object obj, float x, float y) {
				TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
				type->fields.Set("x", obj, x);
				type->fields.Set("y", obj, y);
				type->fields.Get<size_t>("num") += 1;
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
		},
		{ "Sum",
			{
				Func{[](Object obj)->float {
					return obj.Var<float>(0) + obj.Var<float>(sizeof(float));
				}}
							// no attrs
						}
					}
		};
	}
}

void Ubpa::Point_test() {
	TypeInfo* type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto point = type->New("constructor", 1.f, 2.f);

	// call func
	cout << "Sum : " << type->fields.Call<float, Object>("Sum", point) << endl;
	cout << "Sum : " << type->fields.Call("Sum", ArgList{ point }).Cast<float>() << endl;

	// dump
	cout << type->name << endl;

	for (const auto& [name, attr] : type->attrs.data) {
		cout << name;
		if (attr.HasValue()) {
			cout << ": ";
			if (attr.TypeIs<string>())
				cout << attr.Cast<string>();
			else if (attr.TypeIs<std::pair<float, float>>()) {
				auto r = attr.Cast<std::pair<float, float>>();
				cout << r.first << " - " << r.second;
			}
			else
				cout << "[NOT SUPPORT]";
		}
		cout << endl;
	}

	for (const auto& [name, field] : type->fields.data) {
		cout << name;
		/*
		if (auto pV = field.value.CastIf<Var>()) {
			cout << " : ";
			if (pV->TypeIs<float>())
				cout << pV->Get<float>(point);
			else
				cout << "[NOT SUPPORT]";
		}
		else if (auto pV = field.value.CastIf<StaticVar>()) {
			cout << " : ";
			if (pV->TypeIs<size_t>())
				cout << pV->Cast<size_t>();
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
				if (v.TypeIs<float>())
					cout << v.Get<float>(point);
				else
					cout << "[NOT SUPPORT]";
			}
			else if constexpr (std::is_same_v<T, StaticVar>) {
				cout << " : ";
				if (v.TypeIs<size_t>())
					cout << v.Cast<size_t>();
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

		for (const auto& [name, attr] : field.attrs.data) {
			cout << name;
			if (attr.HasValue()) {
				cout << " : ";
				if (auto p = attr.CastIf<string>())
					cout << *p;
				else if (auto p = attr.CastIf<std::pair<float, float>>()) {
					cout << p->first << " - " << p->second;
				}
				else
					cout << "[NOT SUPPORT]";
			}
			cout << endl;
		}
	}

	// query attr
	cout << "contain info : " << type->attrs.Contains("info") << endl;
	cout << "info : " << type->attrs.Get<string>("info") << endl;

	type->Delete(point);
}
