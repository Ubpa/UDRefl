#include <UDRefl/UDRefl.h>
#include <iostream>

#include "A.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Serializer(ConstObjectPtr obj) {
	if (type_name_is_arithmetic(obj->TypeName()))
		std::cout << obj;
	else {
		std::cout << "{";
		std::cout << "\"TYPE\":\"" << obj->TypeName() << "\"";
		auto iter = obj->GetType()->attrs.find(TypeID::of<ContainerType>);
		if (iter != obj->GetType()->attrs.end()) {
			auto containerType = *iter;
			if (containerType == ContainerType::VECTOR) {
				std::cout << ",\"DATA\":[";
				for (size_t i = 0; i < obj->size(); i++) {
					Serializer(obj[i]->DereferenceAsConst());
					if (i + 1 != obj->size())
						std::cout << ",";
				}
				std::cout << "]";
			}
		}
		else { // normal object
			size_t N = obj->GetRVars().size();
			if (N > 0)
				std::cout << ",";
			size_t i = 0;
			obj->ForEachRVar([N, &i](TypeRef type, FieldRef field, ConstObjectPtr var) {
				std::cout << "\"" << Mngr->nregistry.Nameof(field.ID) << "\":";
				Serializer(var);
				if (i != N - 1)
					std::cout << ",";
				i++;
				return true;
			});
		}
		std::cout << "}";
	}
}

int main() {
	RegisterA();

	A a;
	for (size_t i = 0; i < 10; i++) {
		std::vector<std::vector<size_t>> row;
		for (size_t j = 0; j < 10; j++) {
			std::vector<size_t> column;
			for (size_t k = 0; k < 10; k++)
				column.push_back(k);
			row.push_back(std::move(column));
		}
		a.data.push_back(std::move(row));
	}

	Serializer(Ptr(a));
}
