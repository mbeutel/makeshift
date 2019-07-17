
#include <memory>    // for unique_ptr<>
#include <cstddef>   // for size_t, ptrdiff_t
#include <exception> // for terminate()

#include <gsl/gsl_util>   // for narrow_cast<>()
#include <gsl/gsl_assert> // for Expects()

#include <makeshift/new.hpp>
#include <makeshift/memory.hpp>

#if defined(_WIN32)
 #define WIN32_LEAN_AND_MEAN
 #include <Windows.h>
 #include <Memoryapi.h>
#elif defined(__linux__)
 #include <unistd.h>
 #include <stdio.h>
#elif defined(__APPLE__)
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/sysctl.h>
#else
 #error Unsupported operating system.
#endif


namespace makeshift
{


std::size_t hardware_page_size(void) noexcept
{
    static std::size_t result = []
    {
#if defined(_WIN32)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwPageSize;
#elif defined(__linux__)
        return sysconf(_SC_PAGESIZE);
#elif defined(__APPLE__)
        return getpagesize();
#else
 #error Unsupported operating system.
#endif
    }();
    return result;
}

std::size_t hardware_cache_line_size(void) noexcept
{
    static std::size_t result = []
    {
#if defined(_WIN32)
        std::unique_ptr<SYSTEM_LOGICAL_PROCESSOR_INFORMATION[]> dynSlpi;
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION* pSlpi = nullptr;
        DWORD nbSlpi = 0;
        BOOL success = GetLogicalProcessorInformation(pSlpi, &nbSlpi);
        if (!success && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            dynSlpi = std::make_unique<SYSTEM_LOGICAL_PROCESSOR_INFORMATION[]>((nbSlpi + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) - 1) / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
            pSlpi = dynSlpi.get();
            success = GetLogicalProcessorInformation(pSlpi, &nbSlpi);
        }
        Expects(success);
        for (std::ptrdiff_t i = 0, n = nbSlpi / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); i != n; ++i)
        {
            if (pSlpi[i].Relationship == RelationCache && pSlpi[i].Cache.Level == 1 && (pSlpi[i].Cache.Type == CacheData || pSlpi[i].Cache.Type == CacheUnified))
            {
                // We don't bother further checking the CPU affinity group here because we don't expect to encounter a system with heterogeneous cache line sizes.

                return pSlpi[i].Cache.LineSize;
            }
        }
        /*SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX slpi[1]; // optimistic guess
        std::unique_ptr<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX[]> dynSlpi;
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* pSlpi = slpi;
        DWORD nbSlpi = sizeof slpi;
        BOOL success = GetLogicalProcessorInformationEx(RelationAll, pSlpi, &nbSlpi);
        if (!success && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            dynSlpi = std::make_unique<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX[]>((nbSlpi + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) - 1) / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX));
            pSlpi = dynSlpi.get();
            success = GetLogicalProcessorInformationEx(RelationAll, pSlpi, &nbSlpi);
        }
        Expects(success);
        for (std::ptrdiff_t i = 0, n = nbSlpi / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX); i != n; ++i)
        {
            if (pSlpi[i].Relationship == RelationCache && pSlpi[i].Cache.Level == 1 && (pSlpi[i].Cache.Type == CacheData || pSlpi[i].Cache.Type == CacheUnified))
            {
                // We don't bother further checking the CPU affinity group here because we don't expect to encounter a system with heterogeneous cache line sizes.

                return pSlpi[i].Cache.LineSize;
            }
        }*/
        std::terminate(); // nothing was found; we cannot go on

#elif defined(__linux__)
        long result = 0;
 #ifdef _SC_LEVEL1_DCACHE_LINESIZE
        result = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        if (result > 0) return result;
 #endif // _SC_LEVEL1_DCACHE_LINESIZE
        FILE* f = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
        Expects(f != nullptr);
        fscanf(f, "%ld", &result);
        Expects(result != 0);
        fclose(f);
        return gsl::narrow_cast<std::size_t>(result);

#elif defined(__APPLE__)
        std::size_t result = 0;
        std::size_t nbResult = sizeof result;
        int success = sysctlbyname("hw.cachelinesize", &result, &nbResult, 0, 0);
        Expects(success);
        return result;
#else
 #error Unsupported operating system.
#endif
    }();
    return result;
}


} // namespace makeshift
