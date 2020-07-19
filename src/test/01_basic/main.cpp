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
//};
//

using namespace Ubpa::UDRefl;
using namespace std;

int main() {
	{ // register
		TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(PointID);
		type.name = "struct Point";
		type.attrs["info"].value = std::string("hello world");

		Field& field_x = type.fields["x"];
		Field& field_y = type.fields["y"];
		Field& field_default_constructor = type.fields[Field::default_constructor];
		Field& field_destructor = type.fields[Field::destructor];

		field_x.value = Field::NonStaticVar::Init<float>(0);
		field_x.attrs["not_serialize"];

		field_y.value = Field::NonStaticVar::Init<float>(sizeof(float));
		field_y.attrs["range"].value = std::pair<float, float>{ 0.f, 10.f };

		field_default_constructor.value = Field::Funcs{
			std::function{[]()->Object{
				auto ptr = malloc(2 * sizeof(float));
				cout << "construct Point @" << ptr << endl;
				return { PointID, ptr };
			}}
		};
		field_destructor.value = Field::Funcs{
			std::function{[](Object obj) {
				cout << "destruct Point @" << obj.Pointer() << endl;
				free(obj.Pointer());
			}}
		};
	}

	// ======================

	/*const*/ TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto point = type.DefaultConstruct();

	// set
	get<Field::NonStaticVar>(type.fields["x"].value).set(point, 1.f);
	get<Field::NonStaticVar>(type.fields["y"].value).set(point, 2.f);

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
			auto v = pV->get(point);
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

	type.Destruct(point);
}
