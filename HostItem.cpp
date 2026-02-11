
#include "HostItem.h"

HostItem::HostItem(const std::wstring& ip, const std::wstring& adapter, const std::wstring& family, const std::wstring& ifType, eCollectorType collectorType) noexcept :
	Ip(ip),
	Family(family),
	IfType(ifType),
	CollectorType(collectorType)
{
	Adapters.push_back(adapter);
}

const std::wstring HostItem::GetAdaptersNames() noexcept
{
	std::wstring retv;

	std::sort(Adapters.begin(), Adapters.end());

	for (auto& item : Adapters)
		retv += L"[" + item + L"]";

	//retv.erase(retv.find_last_not_of(L" ") + 1);

	return retv;
}

const std::wstring HostItem::GetCollectorTypes(const eCollectorType &collectorType) noexcept
{
	std::wstring retv;

	auto ct = static_cast<ECOLLECTORTYPE>(collectorType);
	constexpr auto ctNeighbor = static_cast<ECOLLECTORTYPE>(eCollectorType::collectorNeighbor);
	constexpr auto ctUnicast = static_cast<ECOLLECTORTYPE>(eCollectorType::collectorUnicast);
	constexpr auto ctAnycast = static_cast<ECOLLECTORTYPE>(eCollectorType::collectorAnycast);
	constexpr auto ctMulticast = static_cast<ECOLLECTORTYPE>(eCollectorType::collectorMulticast);

	if ((ct & ctNeighbor) == ctNeighbor)
		retv += L"Neighbor ";

	if ((ct & ctUnicast) == ctUnicast)
		retv += L"Unicast ";

	if ((ct & ctAnycast) == ctAnycast)
		retv += L"Anycast ";

	if ((ct & ctMulticast) == ctMulticast)
		retv += L"Multicast ";

	retv.erase(retv.find_last_not_of(L" ") + 1);

	return retv;
}

const std::wstring HostItem::GetCollectorTypes() const noexcept
{
	return GetCollectorTypes(CollectorType);
}
