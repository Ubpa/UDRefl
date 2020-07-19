#include <UDRefl/UDRefl.h>

#include <array>
#include <iostream>

//
//struct [[size(8)]] Point {
//	[[not_serialize]]
//	float x;
//	[[info("hello")]]
//	float y;
//};
//

using namespace Ubpa::UDRefl;
using namespace std;

int main() {
	{ // register
		TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);
		type.name = "struct Point";
		type.attrs.elems["size"].value = 8;

		Field& field_x = type.fields.elems["x"];
		Field& field_y = type.fields.elems["y"];

		field_x.value = Field::NonStaticVar{
			function{[](void* p)->any {
				return Field::GetVar<float>(p, 0);
			}},
			function{[](void* p, any value) {
				Field::GetVar<float>(p, 0) = any_cast<float>(value);
			}}
		};

		field_x.attrs.elems["not_serialize"];

		field_y.value = Field::NonStaticVar{
			function{[](void* p)->any {
				return Field::GetVar<float>(p, sizeof(float));
			}},
			function{[](void* p, any value) {
				Field::GetVar<float>(p, sizeof(float)) = any_cast<float>(value);
			}}
		};
		field_y.attrs.elems["info"].value = string{ "hello" };
	}

	// ======================

	array<uint8_t, sizeof(float) * 2> point;

	/*const*/ TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);

	// set
	get<Field::NonStaticVar>(type.fields.elems["x"].value).setter(point.data(), 1.f);
	get<Field::NonStaticVar>(type.fields.elems["y"].value).setter(point.data(), 2.f);

	// dump
	cout << type.name << endl;

	for (const auto& [name, attr] : type.attrs.elems) {
		cout << name;
		if (attr.value.has_value()) {
			if (attr.value.type() == typeid(string))
				cout << ": " << any_cast<string>(attr.value);
			else if (attr.value.type() == typeid(int))
				cout << ": " << any_cast<int>(attr.value);
		}
		cout << endl;
	}

	for (const auto& [name, field] : type.fields.elems) {
		cout << name;
		if (auto pV = get_if<Field::NonStaticVar>(&field.value)) {
			cout << ": ";
			auto v = pV->getter(point.data());
			if (v.type() == typeid(float))
				cout << any_cast<float>(v);
			else
				cout << "[NOT SUPPORT]";
		}
		cout << endl;
		for (const auto& [name, attr] : field.attrs.elems) {
			cout << name;
			if (attr.value.has_value()) {
				if (attr.value.type() == typeid(string))
					cout << ": " << any_cast<string>(attr.value);
				else if (attr.value.type() == typeid(int))
					cout << ": " << any_cast<int>(attr.value);
			}
			cout << endl;
		}
	}
}
