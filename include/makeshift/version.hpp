
#ifndef INCLUDED_MAKESHIFT_VERSION_HPP_
#define INCLUDED_MAKESHIFT_VERSION_HPP_


#include <makeshift/detail/export.hpp>  // for MAKESHIFT_PUBLIC
#include <makeshift/detail/version.hpp>


    // compiler-specific attributes

    // TODO: replace with Hedley
#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000
    // Selectively enable the empty base optimization for a given type.
    // __declspec(empty_bases) was added in VC++ 2015 Update 2 and is expected to become unnecessary in the next ABI-breaking release.
 #define MAKESHIFT_EMPTY_BASES __declspec(empty_bases)
#else // defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000
 #define MAKESHIFT_EMPTY_BASES
#endif // defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000

    // TODO: replace with Hedley
#if defined(_MSC_VER)
 #define MAKESHIFT_FORCEINLINE __forceinline
 #define MAKESHIFT_NOINLINE __declspec(noinline)
#elif defined(__INTEL_COMPILER)
 #define MAKESHIFT_FORCEINLINE __forceinline // TODO: ??
 #define MAKESHIFT_NOINLINE __attribute__((noinline)) // TODO: ??
#elif defined(__GNUC__)
 #define MAKESHIFT_FORCEINLINE __attribute__((always_inline)) inline
 #define MAKESHIFT_NOINLINE __attribute__((noinline))
#else
 #define MAKESHIFT_FORCEINLINE inline
 #define MAKESHIFT_NOINLINE
#endif

    // TODO: remove?
#ifdef _MSC_VER
 #define MAKESHIFT_VECTORCALL __vectorcall
#else // _MSC_VER
 #define MAKESHIFT_VECTORCALL
#endif // _MSC_VER

    // TODO: make internal
#if defined(_MSC_VER) && defined(__INTELLISENSE__)
 #define MAKESHIFT_INTELLISENSE_PARSER
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)

    // TODO: make internal
#define MAKESHIFT_NODISCARD [[nodiscard]]

    // TODO: make internal
#define MAKESHIFT_CONSTEXPR_CXX17 constexpr

    // TODO: make internal
#define MAKESHIFT_CONSTEXPR_CXX20

    // TODO: make internal
#define MAKESHIFT_CXX17


#endif // INCLUDED_MAKESHIFT_VERSION_HPP_
