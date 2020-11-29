#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

struct Point {
	float x;
	float y;
};

int main() {
	auto ID_Point = ReflMngr::Instance().tregistry.Register("Point");
	auto ID_float = ReflMngr::Instance().tregistry.Register("float");

	auto ID_x = ReflMngr::Instance().nregistry.Register("x");
	auto ID_y = ReflMngr::Instance().nregistry.Register("y");

	auto ID_ctor = ReflMngr::Instance().nregistry.GetID(NameRegistry::Meta::ctor);
	auto ID_dtor = ReflMngr::Instance().nregistry.GetID(NameRegistry::Meta::dtor);

	{ // register Point
		TypeInfo typeinfo{
			sizeof(Point),
			alignof(Point),
			{ // fieldinfos
				{ ID_x, { { ID_float, offsetof(Point, x) } }},
				{ ID_y, { { ID_float, offsetof(Point, y) } }}
			},
			{ // methods
				{ID_ctor, {MethodPtr::GenerateDefaultConstructor<Point>()}},
				{ID_dtor, {MethodPtr::GenerateDestructor<Point>()}}
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Point, std::move(typeinfo));
	}
	
	ObjectPtr p = ReflMngr::Instance().New(ID_Point);
	ReflMngr::Instance().RWVar(p, ID_x).As<float>() = 1.f;
	ReflMngr::Instance().RWVar(p, ID_y).As<float>() = 2.f;

	ReflMngr::Instance().ForEachRVar(
		p,
		[](Type type, Field field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
		}
	);
	ReflMngr::Instance().Delete(p);
}
