#pragma once

#include <string>
#include <vector>
#include <algorithm>

#define ECOLLECTORTYPE uint64_t

enum class eCollectorType : ECOLLECTORTYPE {
    collectorNeighbor = 1 << 0,
    collectorUnicast = 1 << 1,
    collectorAnycast = 1 << 2,
    collectorMulticast = 1 << 3
};

typedef struct HostItem
{
    HostItem(const std::wstring& ip, const std::wstring& adapter, const std::wstring& family, const std::wstring& ifType, const eCollectorType collectorType) noexcept;
    std::wstring Ip;
    std::vector<std::wstring> Adapters;
    std::wstring Family;
    std::wstring IfType;
    std::wstring Host;
    eCollectorType CollectorType;

    const std::wstring GetAdaptersNames() noexcept;
    static const std::wstring GetCollectorTypes(const eCollectorType &collectorType) noexcept;
    const std::wstring GetCollectorTypes() const noexcept;
}HostItem;
