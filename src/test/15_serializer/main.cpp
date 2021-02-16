#include <UDRefl/UDRefl.h>
#include <iostream>

#include "Vector.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Serializer(ObjectView obj) {
	if (type_name_is_arithmetic(obj.GetType().GetName()))
		std::cout << obj;
	else {
		std::cout << "{" << "\"TYPE\":\"" << obj.GetType().GetName() << "\",";
		auto iter = Mngr->GetTypeInfo(obj.GetType())->attrs.find(Type_of<ContainerType>);
		if (iter != Mngr->GetTypeInfo(obj.GetType())->attrs.end()) {
			if (*iter == ContainerType::Vector) {
				std::cout << "\"Vector\":[";
				for (size_t i = 0; i < obj.size(); i++) {
					Serializer(obj[i].RemoveReference());
					if (i + 1 != obj.size())
						std::cout << ",";
				}
				std::cout << "]";
			}
		}
		else { // normal object
			size_t N = obj.GetVars().size();
			size_t i = 0;
			for (const auto& [type, field, var] : obj.GetTypeFieldVars()) {
				std::cout << "\"" << field.name.GetView() << "\":";
				Serializer(var);
				if (++i != N)
					std::cout << ",";
			}
		}
		std::cout << "}";
	}
}

int main() {
	RegisterVector();

	auto a = Mngr->MakeShared(Type_of<Vector>);

	for (size_t i = 0; i < 10; i++) {
		std::vector<size_t> row;
		for (size_t j = 0; j < 10; j++)
			row.push_back(j);
		a.Var("data").push_back(std::move(row));
	}

	Serializer(a);

	return 0;
}

