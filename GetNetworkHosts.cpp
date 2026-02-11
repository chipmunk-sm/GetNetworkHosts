#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning( push )
#pragma warning( disable : 4365 ) // conversion from '' to ''
#pragma warning( disable : 4668 ) // is not defined as a preprocessor macro
#pragma warning( disable : 4710 ) // function not inlined
#pragma warning( disable : 4711 ) // selected for automatic inline expansion
#pragma warning( disable : 4820 ) // bytes padding added after data member
#pragma warning( disable : 5039 ) // pointer or reference to potentially throwing function passed to 'extern "C"' function under - EHc.Undefined behavior may occur if this function throws an exception.
#pragma warning( disable : 5264 ) // 'const' variable is not used

#include <iostream>
#include <string>
#include <chrono>

#pragma warning( pop )

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

//#if defined(_DEBUG) || defined(DEBUG)
//#define TRACEENABLE
////#include <debugapi.h>
//#endif // DEBUG


#include "HostItem.h"
#include "HostCollector.h"

int wmain(int argc, wchar_t argv[])
{

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv[0]);

	try
	{

		WSAData d;
		if (WSAStartup(MAKEWORD(2, 2), &d) != 0) {
			return -1;
		}

		auto xStartTime = std::chrono::high_resolution_clock::now();

		HostCollector collector;
		//collector.CollectNeighborIp(AF_INET);
		//collector.CollectNeighborIp(AF_INET6);
		//collector.CollectNeighborIp(AF_UNSPEC);
		//collector.CollectUnicastIp(AF_INET);
		//collector.CollectUnicastIp(AF_INET6);
		//collector.CollectUnicastIp(AF_UNSPEC);

		collector.CollectNeighborIp();
		collector.CollectUnicastIp();
		collector.CollectAnycastIp();
		collector.CollectMulticastIp();
		
		{
			auto xLastTime = xStartTime;
			while (collector.WaitForResult(std::chrono::milliseconds(10)) > 0 && !collector.IsAbort()) {
				const auto xCurrentTime = std::chrono::high_resolution_clock::now();
				const auto dInterval = std::chrono::duration_cast<std::chrono::duration<double>>(xCurrentTime - xLastTime).count();// 
				if (dInterval > 2.5) {
					const auto dElapsed = std::chrono::duration_cast<std::chrono::duration<double>>(xCurrentTime - xStartTime).count();// 
					fwprintf(stdout, L"\rElapsed %9.6f sec.          ", dElapsed);
					fflush(stdout);
					xLastTime = xCurrentTime;
				}
			}
		}

		const auto dReady = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - xStartTime).count();// 


#ifdef TRACEENABLE
		std::wstring out(L"--- START ---\n");
		OutputDebugStringW(out.c_str());
#endif

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

		fwprintf(stdout, L"--- END --- Collect in %9.6f sec.\n", dReady);

	}
	catch (const std::exception& ex) {
		fprintf(stderr, "FATAL: %s", ex.what());
	}
	catch (...) {
		try {
			std::rethrow_exception(std::current_exception());
		}
		catch (const std::exception& ex) {
			fprintf(stderr, "FATAL: %s", ex.what());
		}
		catch (...) {
			fprintf(stderr, "FATAL: Unexpected exception");
		}
	}

	WSACleanup();

	return 0;



}

