#pragma once
#include <iostream>
#include <string>
#include <map>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#include "HostItem.h"

class HostCollector
{
public:
	HostCollector();
	~HostCollector();

	HostCollector(HostCollector&) = delete;
	HostCollector operator=(HostCollector&) = delete;

	std::vector<HostItem> GetHostList();

	size_t GetPaddingLenIp(const std::wstring& val) const;
	size_t GetPaddingLenHost(const std::wstring& val) const;
	size_t GetPaddingLenFamily(const std::wstring& val) const;
	size_t GetPaddingLenAdapter(const std::wstring& val) const;
	size_t GetPaddingLenType(const std::wstring& val) const;
	/// <summary>
	/// Collect neighbor hosts
	/// </summary>
	/// <param name="family">AF_INET, AF_INET6, AF_UNSPEC</param>
	void CollectNeighborIp(const ADDRESS_FAMILY family = AF_UNSPEC);

	/// <summary>
	///  Collect Unicast Ip
	/// </summary>
	/// <param name="family">AF_INET, AF_INET6, AF_UNSPEC</param>
	void CollectUnicastIp(const ADDRESS_FAMILY family = AF_UNSPEC);

	/// <summary>
	///  Collect Anycast Ip
	/// </summary>
	/// <param name="family">AF_INET, AF_INET6, AF_UNSPEC</param>
	void CollectAnycastIp(const ADDRESS_FAMILY family = AF_UNSPEC);

	/// <summary>
	///  Collect Multicast Ip
	/// </summary>
	/// <param name="family">AF_INET, AF_INET6, AF_UNSPEC</param>
	void CollectMulticastIp(const ADDRESS_FAMILY family = AF_UNSPEC);

	static const std::wstring sring2wstring(const char* str);

	static std::string GetWsaErrorString(int errorCode);

	int64_t WaitForResult(const std::chrono::milliseconds& _Rel_time);

	bool IsAbort();
	void SetAbort();

private:

	void AddHost(const SOCKADDR_INET& address, const std::wstring& adapter, const ULONG64& ifType, eCollectorType collectorType);

	void SetHostNameReady(const std::wstring ip, const std::wstring host);

	static std::wstring GetFamilyString(const int32_t& family);
	static std::wstring GetTypeString(const ULONG64& ifType);

	static std::wstring GetHostName(const IN6_ADDR* ipv6addr);
	static std::wstring GetHostName(const IN_ADDR* ipvaddr);

	static size_t GetPaddingLen(const size_t val, const size_t maxVal);

private:

	/// <summary>
	/// first parameter IP
	/// second parameter NAME
	/// </summary>
	std::map<std::wstring, HostItem> m_hostmap;
	std::mutex m_hostmapMtx;

	/// <summary>
	/// zero if no active threads
	/// </summary>
	std::atomic_int64_t m_threadRun = 0;

	/// <summary>
	/// set to true to force shutdown threads
	/// </summary>
	bool m_abort = false;
	char Padding[7]{};

	size_t m_ipLen = 0;
	size_t m_hostLen = 1;
	size_t m_adapterLen = 0;
	size_t m_familyLen = 0;
	size_t m_typeLen = 0;

	std::condition_variable m_onResultEvent;
	std::mutex m_onResultMutex;

};

void GetIpString(const SOCKADDR_INET& address, std::wstring& ip);
