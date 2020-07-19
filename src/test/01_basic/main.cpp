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
		type.attrs["info"].value = std::string("hello world");

		Field field_x;
		Field field_y;
		Field field_num;

		field_x.value = Field::NonStaticVar::Init<float>(0);
		field_x.attrs["not_serialize"];

		field_y.value = Field::NonStaticVar::Init<float>(sizeof(float));
		field_y.attrs["range"].value = std::pair<float, float>{ 0.f, 10.f };

		field_num.value = Field::StaticVar::Init<size_t>(0);

		type.fields.emplace("x", move(field_x));
		type.fields.emplace("y", move(field_y));
		type.fields.emplace("num", move(field_num));

		type.fields.emplace(
			Field::default_constructor,
			Field{ Field::Func::Init(
				[]()->Object {
					auto ptr = malloc(2 * sizeof(float));
					cout << "construct Point @" << ptr << endl;
					return { PointID, ptr };
				}
			) }
		);
		type.fields.emplace(
			Field::destructor,
			Field{ Field::Func::Init(
				[](Object obj) {
					cout << "destruct Point @" << obj.Pointer() << endl;
					free(obj.Pointer());
				}
			) }
		);
		type.fields.emplace(
			"Sum",
			Field{ Field::Func::Init(
				[](Object obj)->float {
					return obj.Var<float>(0) + obj.Var<float>(sizeof(float));
				}
			) }
		);
	}

	// ======================

	/*const*/ TypeInfo& type = TypeInfoMngr::Instance().GetTypeInfo(0);

	auto point = type.DefaultConstruct();

	// set
	get<Field::NonStaticVar>(type.fields.find("x")->second.value).set(point, 1.f);
	get<Field::NonStaticVar>(type.fields.find("y")->second.value).set(point, 2.f);

	// call func
	cout << "Sum : " << type.Call<float, Object>("Sum", point) << endl;

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
		else if (auto pV = get_if<Field::StaticVar>(&field.value)) {
			cout << ": ";
			if (pV->data.type() == typeid(size_t))
				cout << any_cast<size_t>(pV->data);
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
