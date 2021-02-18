#include "ReflMngrInitUtil.h"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_0(ReflMngr& mngr) {
	mngr.RegisterType<ContainerType>();
	mngr.AddField<ContainerType::RawArray>("RawArray");
	mngr.AddField<ContainerType::Array>("Array");
	mngr.AddField<ContainerType::Vector>("Vector");
	mngr.AddField<ContainerType::Deque>("Deque");
	mngr.AddField<ContainerType::ForwardList>("ForwardList");
	mngr.AddField<ContainerType::List>("List");
	mngr.AddField<ContainerType::Set>("Set");
	mngr.AddField<ContainerType::MultiSet>("MultiSet");
	mngr.AddField<ContainerType::Map>("Map");
	mngr.AddField<ContainerType::MultiMap>("MultiMap");
	mngr.AddField<ContainerType::UnorderedSet>("UnorderedSet");
	mngr.AddField<ContainerType::UnorderedMultiSet>("UnorderedMultiSet");
	mngr.AddField<ContainerType::UnorderedMap>("UnorderedMap");
	mngr.AddField<ContainerType::UnorderedMultiMap>("UnorderedMultiMap");
	mngr.AddField<ContainerType::Stack>("Stack");
	mngr.AddField<ContainerType::PriorityQueue>("PriorityQueue");
	mngr.AddField<ContainerType::Queue>("Queue");
	mngr.AddField<ContainerType::Tuple>("Tuple");
	mngr.AddField<ContainerType::Pair>("Pair");
	mngr.AddField<ContainerType::Span>("Span");
}
