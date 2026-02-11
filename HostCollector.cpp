#include "HostCollector.h"

HostCollector::HostCollector() {

}

HostCollector::~HostCollector() {

	SetAbort();

	int32_t timeout = 5000;

	while (WaitForResult(std::chrono::milliseconds(20)) > 0 && timeout-- > 0);

}

static std::wstring GetIpString(const SOCKADDR_INET& address)
{
	std::vector<wchar_t> buff(512, 0);

	if (address.si_family == AF_INET && InetNtopW(AF_INET, &address.Ipv4.sin_addr, buff.data(), buff.size() - 1) != nullptr)
		return buff.data();

	if (address.si_family == AF_INET6 && InetNtopW(AF_INET6, &address.Ipv6.sin6_addr, buff.data(), buff.size() - 1) != nullptr)
		return buff.data();

	return L"";
}

void HostCollector::AddHost(const SOCKADDR_INET& address, const std::wstring& adapter, const ULONG64& ifType, eCollectorType collectorType) {

	try
	{
		std::wstring ip = GetIpString(address);
		if (ip.empty()) {
			return;
		}

		auto iter = m_hostmap.find(ip);// + adapter
		if (iter != m_hostmap.end()) {

			iter->second.CollectorType = static_cast<eCollectorType>(static_cast<ECOLLECTORTYPE>(iter->second.CollectorType) | static_cast<ECOLLECTORTYPE>(collectorType));
			
			if (std::find(iter->second.Adapters.begin(), iter->second.Adapters.end(), adapter) == iter->second.Adapters.end())
				iter->second.Adapters.push_back(adapter);

			m_adapterLen = std::max<decltype(m_adapterLen)>(m_adapterLen, static_cast<decltype(m_adapterLen)>(iter->second.GetAdaptersNames().length()));

			return;

		}

		{
			auto sIfType = HostCollector::GetTypeString(ifType);
			auto sFamily = HostCollector::GetFamilyString(address.si_family);

			m_ipLen = std::max<decltype(m_ipLen)>(m_ipLen, static_cast<decltype(m_ipLen)>(ip.length()));
			//m_hostLen = std::max<decltype(m_hostLen)>(m_hostLen, static_cast<decltype(m_hostLen)>(item->GetHostName().length()));
			m_adapterLen = std::max<decltype(m_adapterLen)>(m_adapterLen, static_cast<decltype(m_adapterLen)>(adapter.length()));
			m_familyLen = std::max<decltype(m_familyLen)>(m_familyLen, static_cast<decltype(m_familyLen)>(sFamily.length()));
			m_typeLen = std::max<decltype(m_typeLen)>(m_typeLen, static_cast<decltype(m_typeLen)>(sIfType.length()));

			std::lock_guard<std::mutex> lck(m_hostmapMtx);
			m_hostmap.emplace(ip, HostItem(ip, adapter, sFamily, sIfType, collectorType));
		}

		m_threadRun++;
		auto threadBase = [ip, address](decltype(this) pThis)
		{
			try {
				pThis->SetHostNameReady(ip, address.si_family == AF_INET6 ? GetHostName(&address.Ipv6.sin6_addr) : GetHostName(&address.Ipv4.sin_addr));
			}
			catch (const std::exception& ex) {
				fprintf(stderr, "Thread get host name FATAL: %s", ex.what());
			}
			catch (...) {
				try {
					std::rethrow_exception(std::current_exception());
				}
				catch (const std::exception& ex) {
					fprintf(stderr, "Thread get host name FATAL: %s", ex.what());
				}
			}
			pThis->m_threadRun--;

		};
		(std::thread(threadBase, this)).detach();

	}
	catch (...)
	{
		try {
			std::rethrow_exception(std::current_exception());
		}
		catch (const std::exception& ex) {
			fprintf(stderr, "Thread get host name FATAL: %s", ex.what());
		}
	}

}

std::wstring HostCollector::GetFamilyString(const int32_t& family)
{
	switch (family)	{
	case AF_UNSPEC: return L"Unspecified";
	case AF_UNIX: return L"local to host (pipes, portals)";
	case AF_INET: return L"IPv4";
	case AF_IMPLINK: return L"ARPANET imp addresses";
	case AF_PUP: return L"pup protocols: e.g. BSP";
	case AF_CHAOS: return L"MIT CHAOS protocols";
	case AF_IPX: return L"IPX protocols: IPX, SPX, etc.";
	case AF_ISO: return L"ISO protocols";
	case AF_ECMA: return L"European computer manufacturers";
	case AF_DATAKIT: return L"datakit protocols";
	case AF_CCITT: return L"CCITT protocols, X.25 etc";
	case AF_SNA: return L"IBM SNA";
	case AF_DECnet: return L"DECnet";
	case AF_DLI: return L"Direct data link interface";
	case AF_LAT: return L"LAT";
	case AF_HYLINK: return L"NSC Hyperchannel";
	case AF_APPLETALK: return L"AppleTalk";
	case AF_NETBIOS: return L"NetBios-style addresses";
	case AF_VOICEVIEW: return L"VoiceView";
	case AF_FIREFOX: return L"Protocols from Firefox";
	case AF_UNKNOWN1: return L"Somebody is using this!";
	case AF_BAN: return L"Banyan";
	case AF_ATM: return L"Native ATM Services";
	case AF_INET6: return L"IPv6";
	case AF_CLUSTER: return L"Microsoft Wolfpack";
	case AF_12844: return L"IEEE 1284.4 WG AF";
	case AF_IRDA: return L"IrDA";
	case AF_NETDES: return L"Network Designers OSI & gateway";
	default: return L"";
	}
}

std::wstring HostCollector::GetTypeString(const ULONG64& ifType)
{
	switch (ifType)	{
	case IF_TYPE_OTHER: return L"Other";
	case IF_TYPE_ETHERNET_CSMACD: return L"Ethernet";
	case IF_TYPE_ISO88025_TOKENRING: return L"Token ring";
	case IF_TYPE_PPP: return L"PPP";
	case IF_TYPE_SOFTWARE_LOOPBACK: return L"Software loopback";
	case IF_TYPE_ATM: return L"ATM";
	case IF_TYPE_IEEE80211: return L"802.11 wireless";
	case IF_TYPE_TUNNEL: return L"Tunnel encapsulation";
	case IF_TYPE_IEEE1394: return L"IEEE 1394 (Firewire)";
	default: return L"";
	}
}

std::vector<HostItem> HostCollector::GetHostList() {
	std::lock_guard<std::mutex> lck(m_hostmapMtx);
	std::vector<HostItem> items;
	for (const auto& x : m_hostmap)
		items.push_back(x.second);
	return items;
}

size_t HostCollector::GetPaddingLenIp(const std::wstring& val) const {
	return GetPaddingLen(val.length(), m_ipLen);
}

size_t HostCollector::GetPaddingLenHost(const std::wstring& val) const {
	return  GetPaddingLen(val.length(), m_hostLen);
}

size_t HostCollector::GetPaddingLenFamily(const std::wstring& val) const {
	return GetPaddingLen(val.length(), m_familyLen);
}

size_t HostCollector::GetPaddingLenAdapter(const std::wstring& val) const {
	return GetPaddingLen(val.length(), m_adapterLen);
}

size_t HostCollector::GetPaddingLenType(const std::wstring& val) const {
	return GetPaddingLen(val.length(), m_typeLen);
}

void HostCollector::CollectNeighborIp(const ADDRESS_FAMILY family)
{
	PMIB_IPNET_TABLE2 pipTable = nullptr;
	if (GetIpNetTable2(family, &pipTable) == NO_ERROR && pipTable != nullptr) {
		for (decltype(pipTable->NumEntries) i = 0; i < pipTable->NumEntries; i++) {
			const auto item = pipTable->Table[i];
			std::vector<wchar_t> adapter(NDIS_IF_MAX_STRING_SIZE + 2, 0);
			ConvertInterfaceLuidToAlias(&item.InterfaceLuid, adapter.data(), adapter.size() - 1);
			AddHost(item.Address, adapter.data(), item.InterfaceLuid.Info.IfType, eCollectorType::collectorNeighbor);
		}
	}
	if (pipTable != nullptr)
		FreeMibTable(pipTable);
}

void HostCollector::CollectUnicastIp(const ADDRESS_FAMILY family)
{
	PMIB_UNICASTIPADDRESS_TABLE pipTable = nullptr;
	if (GetUnicastIpAddressTable(family, &pipTable) == NO_ERROR && pipTable != nullptr) {
		for (decltype(pipTable->NumEntries) i = 0; i < pipTable->NumEntries; i++) {
			const auto item = pipTable->Table[i];
			std::vector<wchar_t> adapter(NDIS_IF_MAX_STRING_SIZE + 2, 0);
			ConvertInterfaceLuidToAlias(&item.InterfaceLuid, adapter.data(), adapter.size() - 1);
			AddHost(item.Address, adapter.data(), item.InterfaceLuid.Info.IfType, eCollectorType::collectorUnicast);
		}
	}
	if (pipTable != nullptr)
		FreeMibTable(pipTable);
}

void HostCollector::CollectAnycastIp(const ADDRESS_FAMILY family)
{
	PMIB_ANYCASTIPADDRESS_TABLE pipTable = nullptr;
	if (GetAnycastIpAddressTable(family, &pipTable) == NO_ERROR && pipTable != nullptr) {
		for (decltype(pipTable->NumEntries) i = 0; i < pipTable->NumEntries; i++) {
			const auto item = pipTable->Table[i];
			std::vector<wchar_t> adapter(NDIS_IF_MAX_STRING_SIZE + 2, 0);
			ConvertInterfaceLuidToAlias(&item.InterfaceLuid, adapter.data(), adapter.size() - 1);
			AddHost(item.Address, adapter.data(), item.InterfaceLuid.Info.IfType, eCollectorType::collectorAnycast);
		}
	}
	if (pipTable != nullptr)
		FreeMibTable(pipTable);
}

void HostCollector::CollectMulticastIp(const ADDRESS_FAMILY family)
{
	PMIB_MULTICASTIPADDRESS_TABLE pipTable = nullptr;
	if (GetMulticastIpAddressTable(family, &pipTable) == NO_ERROR && pipTable != nullptr) {
		for (decltype(pipTable->NumEntries) i = 0; i < pipTable->NumEntries; i++) {
			const auto item = pipTable->Table[i];
			std::vector<wchar_t> adapter(NDIS_IF_MAX_STRING_SIZE + 2, 0);
			ConvertInterfaceLuidToAlias(&item.InterfaceLuid, adapter.data(), adapter.size() - 1);
			AddHost(item.Address, adapter.data(), item.InterfaceLuid.Info.IfType, eCollectorType::collectorMulticast);
		}
	}
	if (pipTable != nullptr)
		FreeMibTable(pipTable);
}

const std::wstring HostCollector::sring2wstring(const char* str)
{
	const size_t cSize = strlen(str) + 1;
	std::vector<wchar_t> buf(cSize + 1, 0);
	size_t outSize;
	mbstowcs_s(&outSize, buf.data(), cSize, str, cSize);
	return buf.data();
}

std::string HostCollector::GetWsaErrorString(int errorCode)
{
	std::string retv(std::to_string(errorCode) + ": ");
	std::vector<char> buf(1024, 0);
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, static_cast<DWORD>(errorCode), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf.data(), static_cast<DWORD>(buf.size() - 1), nullptr);
	for (auto& item : buf) {
		if (item == '\r' || item == '\n') {
			item = 0;
		}
	}
	retv.append(buf.data());
	return retv;
}

int64_t HostCollector::WaitForResult(const std::chrono::milliseconds & _Rel_time)
{
	std::unique_lock<std::mutex> locker(m_onResultMutex);
	m_onResultEvent.wait_for(locker, _Rel_time);

	return m_threadRun.load();
}

bool HostCollector::IsAbort() {
	return m_abort;
}

void HostCollector::SetAbort() {
	m_abort = true;
	m_onResultEvent.notify_all();
}

void HostCollector::SetHostNameReady(const std::wstring ip, const std::wstring host)
{
	std::lock_guard<std::mutex> lck(m_hostmapMtx);
	auto item = m_hostmap.find(ip);
	if (item == m_hostmap.end())
		return;
	m_hostLen = std::max<decltype(m_hostLen)>(m_hostLen, static_cast<decltype(m_hostLen)>(host.length()));
	item->second.Host = host;
	m_onResultEvent.notify_all();
}

std::wstring HostCollector::GetHostName(const IN6_ADDR* ipv6addr)
{
	struct sockaddr_in6 sin6 {};
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = *ipv6addr;
	std::vector<wchar_t> buff(512, 0);
	GetNameInfoW((SOCKADDR*)&sin6, sizeof(struct sockaddr_in6), buff.data(), static_cast<DWORD>(buff.size() - 1), NULL, 0, NI_NAMEREQD);
	return buff.data();
}

std::wstring HostCollector::GetHostName(const IN_ADDR* ipvaddr)
{
	struct sockaddr_in sin {};
	sin.sin_family = AF_INET;
	sin.sin_addr = *ipvaddr;
	std::vector<wchar_t> buff(512, 0);
	GetNameInfoW((SOCKADDR*)&sin, sizeof(struct sockaddr_in), buff.data(), static_cast<DWORD>(buff.size() - 1), NULL, 0, NI_NAMEREQD);
	return buff.data();
}

size_t HostCollector::GetPaddingLen(const size_t val, const size_t maxVal)
{
	return val > maxVal ? 1 : maxVal - val;
}
