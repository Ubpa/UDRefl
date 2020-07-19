#include <UDRefl/UDRefl.h>

#include <array>
#include <iostream>

static constexpr size_t PointID = 0;
//
//struct [[info("hello world")]] Point {
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
					Field::StaticVar::Init<size_t>(0)
					// no attrs
				}
			},
			{ TypeInfo::InternalField::default_constructor,
				{
					Field::Func::Init([]() -> Object {
						auto ptr = malloc(2 * sizeof(float));
						cout << "construct Point @" << ptr << endl;
						return { PointID, ptr };
					})
					// no attrs
				}
			},
			{ TypeInfo::InternalField::destructor,
				{
					Field::Func::Init([](Object obj) {
						cout << "destruct Point @" << obj.Pointer() << endl;
						free(obj.Pointer());
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

	/*const*/ TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto point = type.DefaultConstruct();

	// set
	type.fields.Set("x", point, 1.f);
	type.fields.Set("y", point, 2.f);

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
			auto v = pV->get(point);
			if (v.type() == typeid(float))
				cout << any_cast<float>(v);
			else
				cout << "[NOT SUPPORT]";
		}
		else if (auto pV = get_if<Field::StaticVar>(&field.value)) {
			cout << ": ";
			if (pV->data.type() == typeid(size_t))
				cout << any_cast<size_t>(pV->data);
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

	type.Destruct(point);
}
