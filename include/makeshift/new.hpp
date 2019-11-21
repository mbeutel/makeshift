
#ifndef INCLUDED_MAKESHIFT_NEW_HPP_
#define INCLUDED_MAKESHIFT_NEW_HPP_


#include <cstddef> // for size_t


namespace makeshift
{


    // It is controversial whether `std::hardware_{constructive|destructive}_interference_size` should really be a constexpr value, cf. the discussion at
    // https://lists.llvm.org/pipermail/cfe-dev/2018-May/058073.html and the proposal paper P0154R1 at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0154r1.html.

    // My current take is that the constexpr constants below should be reasonable but not necessarily accurate values to minimize the impact of false sharing.
    // An accurate value can be determined at runtime by calling `hardware_cache_line_size()`.

    // Defining these constexpr values in makeshift is less controversial than defining them in the standard library because makeshift is not burdened by
    // the ABI implications. However, note that it would be possible to run into ODR violations if we defined different values for different
    // sub-architectures, so the constexpr value defined here should depend only on architecture (e.g. x64), not on sub-architecture (e.g. Intel "Penryn").

#if defined(_M_IX86) || defined(_M_AMD64) || defined(__i386__) || defined(__x86_64__)
  constexpr std::size_t hardware_constructive_interference_size = 64;
  constexpr std::size_t hardware_destructive_interference_size = 64;
#else // defined(__i386__) || defined(__x86_64__)
 #error Unsupported architecture.
#endif // defined(__i386__) || defined(__x86_64__)


    //ᅟ
    // Reports the operating system's large page size in bytes, or 0 if large pages are not available or not supported.
    //
std::size_t hardware_large_page_size(void) noexcept;

    //ᅟ
    // Reports the operating system's page size in bytes.
    //
std::size_t hardware_page_size(void) noexcept;

    //ᅟ
    // Reports the CPU architecture's cache line size in bytes.
    //
std::size_t hardware_cache_line_size(void) noexcept;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_NEW_HPP_
