
#include <memory>    // for unique_ptr<>
#include <cstdio>
#include <cstddef>   // for size_t, ptrdiff_t
#include <exception> // for terminate()

#include <gsl/gsl-lite.hpp> // for Expects(), narrow_cast<>()

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


std::size_t hardware_large_page_size(void) noexcept
{
    static std::size_t result = []
    {
#if defined(_WIN32)
        return GetLargePageMinimum();
#elif defined(__linux__)
            // I can't believe that parsing /proc/meminfo is the accepted way to query the default hugepage size and other parameters.
        std::FILE* f = std::fopen("/proc/meminfo", "r");
        if (f == nullptr) return std::size_t(0); // something is really wrong, but the best we can do is to pretend there is no hugepage support
        char line[128];
        long hugePageSize = 0;
        char unit[16+1];
        for (;;)
        {
            if (std::fgets(line, sizeof line, f) == nullptr) break; // either EOF or error, we don't distinguish here
            int nFields = std::sscanf(line, "Hugepagesize: %ld %16s", &hugePageSize, unit);
            if (nFields == 2)
            {
                if (std::strcmp(unit, "kB") == 0) hugePageSize *= 1024l; // this is the only unit the kernel currently emits, so I don't bother with speculative M[i]B etc.
                else hugePageSize = 0; // we don't understand this unit; fall back to no hugepage support
                break;
            }
            else
            {
                hugePageSize = 0; // just in case
                // ...and keep going
            }
        }
        std::fclose(f);
        return gsl::narrow_cast<std::size_t>(hugePageSize);
#elif defined(__APPLE__)
        return 0; // MacOS does support huge pages ("superpages") but we currently didn't write any code to support them
#else
 #error Unsupported operating system.
#endif
    }();
    return result;
}

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
        std::terminate(); // nothing was found; we cannot go on

#elif defined(__linux__)
        long result = 0;
 #ifdef _SC_LEVEL1_DCACHE_LINESIZE
        result = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        if (result > 0) return gsl::narrow_cast<std::size_t>(result);
 #endif // _SC_LEVEL1_DCACHE_LINESIZE
        FILE* f = std::fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
        Expects(f != nullptr);
        int nf = std::fscanf(f, "%ld", &result);
        std::fclose(f);
        Expects(nf == 1 && result != 0);
        return gsl::narrow_cast<std::size_t>(result);

#elif defined(__APPLE__)
        std::size_t result = 0;
        std::size_t nbResult = sizeof result;
        int ec = sysctlbyname("hw.cachelinesize", &result, &nbResult, 0, 0);
        Expects(ec == 0);
        return result;
#else
 #error Unsupported operating system.
#endif
    }();
    return result;
}


} // namespace makeshift
