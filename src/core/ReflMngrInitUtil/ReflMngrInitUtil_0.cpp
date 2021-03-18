#include "ReflMngrInitUtil.hpp"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_0(ReflMngr& mngr) {
	mngr.RegisterType<ContainerType>();
	mngr.AddField<ContainerType::Array>("Array");
	mngr.AddField<ContainerType::Deque>("Deque");
	mngr.AddField<ContainerType::ForwardList>("ForwardList");
	mngr.AddField<ContainerType::List>("List");
	mngr.AddField<ContainerType::Map>("Map");
	mngr.AddField<ContainerType::MultiMap>("MultiMap");
	mngr.AddField<ContainerType::MultiSet>("MultiSet");
	mngr.AddField<ContainerType::Optional>("Optional");
	mngr.AddField<ContainerType::Pair>("Pair");
	mngr.AddField<ContainerType::PriorityQueue>("PriorityQueue");
	mngr.AddField<ContainerType::Queue>("Queue");
	mngr.AddField<ContainerType::RawArray>("RawArray");
	mngr.AddField<ContainerType::Set>("Set");
	mngr.AddField<ContainerType::Span>("Span");
	mngr.AddField<ContainerType::Stack>("Stack");
	mngr.AddField<ContainerType::Tuple>("Tuple");
	mngr.AddField<ContainerType::UnorderedMap>("UnorderedMap");
	mngr.AddField<ContainerType::UnorderedMultiSet>("UnorderedMultiSet");
	mngr.AddField<ContainerType::UnorderedMultiMap>("UnorderedMultiMap");
	mngr.AddField<ContainerType::UnorderedSet>("UnorderedSet");
	mngr.AddField<ContainerType::Variant>("Variant");
	mngr.AddField<ContainerType::Vector>("Vector");
}
