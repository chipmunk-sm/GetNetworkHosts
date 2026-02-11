#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <string>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#include "HostItem.h"
#include "HostCollector.h"

class WinsockContext {
public:
    WinsockContext() {
        WSADATA d;
        if (WSAStartup(MAKEWORD(2, 2), &d) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }
    ~WinsockContext() {
        WSACleanup();
    }

    WinsockContext(const WinsockContext&) = delete;
    WinsockContext& operator=(const WinsockContext&) = delete;
};

int main() {
    try {

        auto xStartTime = std::chrono::high_resolution_clock::now();

        WinsockContext wsContext;

        HostCollector collector;

        collector.CollectNeighborIp();
        collector.CollectUnicastIp();
        collector.CollectAnycastIp();
        collector.CollectMulticastIp();

        auto xLastProgressUpdate = xStartTime;
        const auto progressThreshold = std::chrono::milliseconds(500);

        while (collector.WaitForResult(std::chrono::milliseconds(20)) > 0 && !collector.IsAbort()) {
            auto xCurrentTime = std::chrono::high_resolution_clock::now();

            if (xCurrentTime - xLastProgressUpdate > progressThreshold) {
                auto dElapsed = std::chrono::duration<double>(xCurrentTime - xStartTime).count();

                fwprintf(stdout, L"\rElapsed %9.6f sec.          ", dElapsed);
                fflush(stdout);

                xLastProgressUpdate = xCurrentTime;
            }
        }

        auto xEndTime = std::chrono::high_resolution_clock::now();
        double dTotalTime = std::chrono::duration<double>(xEndTime - xStartTime).count();

        fwprintf(stdout, L"\n--- START --- \n");

        for (auto& item : collector.GetHostList()) {
            //            IP      HOS    FAM    ADA    TYP
            fwprintf(stdout, L"| %s%s | %s%s | %s%s | %s%s | %s%s | %s\n",
                item.Ip.c_str(), std::wstring(collector.GetPaddingLenIp(item.Ip), ' ').c_str(),
                item.Host.c_str(), std::wstring(collector.GetPaddingLenHost(item.Host), ' ').c_str(),
                item.Family.c_str(), std::wstring(collector.GetPaddingLenFamily(item.Family), ' ').c_str(),
                item.GetAdaptersNames().c_str(), std::wstring(collector.GetPaddingLenAdapter(item.GetAdaptersNames()), ' ').c_str(),
                item.IfType.c_str(), std::wstring(collector.GetPaddingLenType(item.IfType), ' ').c_str(),
                item.GetCollectorTypes().c_str()
            );
        }

        fwprintf(stdout, L"--- END --- Collect in %9.6f sec.\n", dTotalTime);

    }
    catch (const std::exception& ex) {
        fprintf(stderr, "\nFATAL ERROR: %s\n", ex.what());
        return -1;
    }
    catch (...) {
        fprintf(stderr, "\nFATAL ERROR: Unknown exception occurred\n");
        return -1;
    }

    return 0;
}
