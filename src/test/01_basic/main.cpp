#include <UDRefl/UDRefl.h>

#include <array>
#include <iostream>

//
//struct [[info("hello world")]] Point {
//	[[not_serialize]]
//	float x;
//	[[range(std::pair<float, float>{0.f, 10.f})]]
//	float y;
//};
//

using namespace Ubpa::UDRefl;
using namespace std;

int main() {
	{ // register
		TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);
		type.name = "struct Point";
		type.attrs["info"].value = std::string("hello world");

		Field& field_x = type.fields["x"];
		Field& field_y = type.fields["y"];
		Field& field_Point = type.fields["Point"];
		Field& field_d_Point = type.fields["~Point"];

		field_x.value = Field::NonStaticVar::Init<float>(0);
		field_x.attrs["not_serialize"];

		field_y.value = Field::NonStaticVar::Init<float>(sizeof(float));
		field_y.attrs["range"].value = std::pair<float, float>{ 0.f, 10.f };

		field_Point.value = Field::Funcs{
			std::function{[]()->void*{
				cout << "construct Point" << endl;
				return malloc(2 * sizeof(float));
			}}
		};
		field_d_Point.value = Field::Funcs{
			std::function{[](void* p) {
				cout << "destruct Point" << endl;
				free(p);
			}}
		};
	}

	// ======================

	/*const*/ TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto point = any_cast<std::function<void*()>>(get<Field::Funcs>(type.fields["Point"].value).front())();

	// set
	get<Field::NonStaticVar>(type.fields["x"].value).setter(point, 1.f);
	get<Field::NonStaticVar>(type.fields["y"].value).setter(point, 2.f);

	// dump
	cout << type.name << endl;

	for (const auto& [name, attr] : type.attrs) {
		cout << name;
		if (attr.value.has_value()) {
			cout << ": ";
			if (attr.value.type() == typeid(string))
				cout << any_cast<string>(attr.value);
			else if (attr.value.type() == typeid(std::pair<float, float>)) {
				auto r = any_cast<std::pair<float, float>>(attr.value);
				cout << r.first << " - " << r.second;
			}
			else
				cout << "[NOT SUPPORT]";
		}
		cout << endl;
	}

	for (const auto& [name, field] : type.fields) {
		cout << name;
		if (auto pV = get_if<Field::NonStaticVar>(&field.value)) {
			cout << ": ";
			auto v = pV->getter(point);
			if (v.type() == typeid(float))
				cout << any_cast<float>(v);
			else
				cout << "[NOT SUPPORT]";
		}
		cout << endl;
		for (const auto& [name, attr] : field.attrs) {
			cout << name;
			if (attr.value.has_value()) {
				cout << ": ";
				if (attr.value.type() == typeid(string))
					cout << any_cast<string>(attr.value);
				else if (attr.value.type() == typeid(std::pair<float, float>)) {
					auto r = any_cast<std::pair<float, float>>(attr.value);
					cout << r.first << " - " << r.second;
				}
				else
					cout << "[NOT SUPPORT]";
			}
			cout << endl;
		}
	}

	any_cast<std::function<void(void*)>>(get<Field::Funcs>(type.fields["~Point"].value).front())(point);
}
