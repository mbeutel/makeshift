
#ifndef INCLUDED_MAKESHIFT_NEW_HPP_
#define INCLUDED_MAKESHIFT_NEW_HPP_


#include <cstddef> // for size_t
#ifdef _MSC_VER
 #include <new> // `hardware_{constructive|destructive}_interference_size` currently only provided by VC++
#endif // _MSC_VER

#include <makeshift/detail/export.hpp> // for MAKESHIFT_PUBLIC


namespace makeshift
{


    // It is controversial whether `std::hardware_{constructive|destructive}_interference_size` should really be a constexpr value, cf. the discussion at
    // https://lists.llvm.org/pipermail/cfe-dev/2018-May/058073.html and the proposal paper P0154R1 at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0154r1.html.

    // My current take is that the constexpr constants below should be reasonable but not necessarily accurate values to minimize the impact of false sharing.
    // An accurate value can be determined at runtime by calling `hardware_cache_line_size()`.

    // Defining these constexpr values in makeshift is less controversial than defining them in the standard library because makeshift is not burdened by
    // the ABI implications. However, note that it would be possible to run into ODR violations if we defined different values for different
    // sub-architectures, so the constexpr value defined here should depend only on architecture (e.g. x64), not on sub-architecture (e.g. Intel "Penryn").

#ifdef _MSC_VER
    constexpr inline std::size_t hardware_constructive_interference_size = std::hardware_constructive_interference_size;
    constexpr inline std::size_t hardware_destructive_interference_size = std::hardware_destructive_interference_size;
#else // _MSC_VER
 #if defined(__i386__) || defined(__x86_64__)
    constexpr inline std::size_t hardware_constructive_interference_size = 64;
    constexpr inline std::size_t hardware_destructive_interference_size = 64;
 #else // defined(__i386__) || defined(__x86_64__)
  #error Unsupported architecture.
 #endif // defined(__i386__) || defined(__x86_64__)
#endif // _MSC_VER


    //ᅟ
    // Reports the operating system's page size in bytes.
    //
MAKESHIFT_PUBLIC std::size_t hardware_page_size(void) noexcept;

    //ᅟ
    // Reports the CPU architecture's cache line size in bytes.
    //
MAKESHIFT_PUBLIC std::size_t hardware_cache_line_size(void) noexcept;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_NEW_HPP_
