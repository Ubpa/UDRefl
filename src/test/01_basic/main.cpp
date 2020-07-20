#include <UDRefl/UDRefl.h>

#include <array>
#include <iostream>

static constexpr size_t PointID = 0;
//
//struct [[info("hello world")]] Point {
//  Point() : x{0.f}, y{0.f} {}
//  Point(float x, float y) : x{0.f}, y{0.f} {}
//	[[not_serialize]]
//	float x;
//	[[range(std::pair<float, float>{0.f, 10.f})]]
//	float y;
//  static size_t num{0};
//
//  float Sum() {
//    return x + y;
//  }
//};
//

using namespace Ubpa::UDRefl;
using namespace std;

int main() {
	{ // register
		TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(PointID);
		type.size = 2 * sizeof(float);
		type.name = "struct Point";
		type.attrs = {
			{"info", std::string("hello world")}
		};
		type.fields.data = {
			{ "x",
				{
					Field::NonStaticVar::Init<float>(0),
					AttrList{{"not_serialize", Attr{}}}
				}
			},
			{ "y",
				{
					Field::NonStaticVar::Init<float>(sizeof(float)),
					AttrList{{"range", std::pair<float, float>{ 0.f, 10.f }}}
				}
			},
			{ "num",
				{
					Field::StaticVar{static_cast<size_t>(0)}
					// no attrs
				}
			},
			{ FieldList::default_constructor,
				{
					Field::Func::Init([](Object obj) {
						TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						type.fields.Set("x", obj, 0.f);
						type.fields.Set("y", obj, 0.f);
						cout << "[ " << FieldList::default_constructor << " ] construct " << type.name
							<< " @" << obj.Pointer() << endl;
					})
					// no attrs
				}
			},
			{ "constructor",
				{
					Field::Func::Init([](Object obj, float x, float y) {
						TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						type.fields.Set("x", obj, x);
						type.fields.Set("y", obj, y);
						type.fields.Get<size_t>("num") += 1;
						cout << "[ constructor ] construct " << type.name
							<< " @" << obj.Pointer() << endl;
					})
					// no attrs
				}
			},
			{ FieldList::destructor,
				{
					Field::Func::Init([](Object obj) {
						TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(obj.ID());
						cout << "[ " << FieldList::destructor << " ] destruct " << type.name
							<< " @" << obj.Pointer() << endl;
					})
					// no attrs
				}
			},
			{ "Sum",
				{
					Field::Func::Init([](Object obj)->float {
						return obj.Var<float>(0) + obj.Var<float>(sizeof(float));
					})
					// no attrs
				}
			}
		};
	}

	// ======================

	TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto point = type.New("constructor", 1.f, 2.f);

	// call func
	cout << "Sum : " << type.fields.Call<float, Object>("Sum", point) << endl;

	// dump
	cout << type.name << endl;

	for (const auto& [name, value] : type.attrs) {
		cout << name;
		if (value.has_value()) {
			cout << ": ";
			if (value.type() == typeid(string))
				cout << any_cast<string>(value);
			else if (value.type() == typeid(std::pair<float, float>)) {
				auto r = any_cast<std::pair<float, float>>(value);
				cout << r.first << " - " << r.second;
			}
			else
				cout << "[NOT SUPPORT]";
		}
		cout << endl;
	}

	for (const auto& [name, field] : type.fields.data) {
		cout << name;
		if (auto pV = get_if<Field::NonStaticVar>(&field.value)) {
			cout << ": ";
			if (pV->TypeIs<float>())
				cout << pV->Get<float>(point);
			else
				cout << "[NOT SUPPORT]";
		}
		else if (auto pV = get_if<Field::StaticVar>(&field.value)) {
			cout << ": ";
			if (pV->type() == typeid(size_t))
				cout << any_cast<size_t>(*pV);
			else
				cout << "[NOT SUPPORT]";
		}
		cout << endl;

		for (const auto& [name, value] : field.attrs) {
			cout << name;
			if (value.has_value()) {
				cout << ": ";
				if (value.type() == typeid(string))
					cout << any_cast<string>(value);
				else if (value.type() == typeid(std::pair<float, float>)) {
					auto r = any_cast<std::pair<float, float>>(value);
					cout << r.first << " - " << r.second;
				}
				else
					cout << "[NOT SUPPORT]";
			}
			cout << endl;
		}
	}

	type.Delete(point);
}
