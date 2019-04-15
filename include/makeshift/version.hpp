
#ifndef INCLUDED_MAKESHIFT_VERSION_HPP_
#define INCLUDED_MAKESHIFT_VERSION_HPP_


#include <makeshift/detail/export.hpp>  // for MAKESHIFT_PUBLIC
#include <makeshift/detail/version.hpp>


    // compiler-specific attributes

#if defined(_MSC_VER)
 #define MAKESHIFT_SELECTANY_PREFIX __declspec(selectany)
 #define MAKESHIFT_SELECTANY_SUFFIX
#elif defined(__GNUC__) || defined(__clang__)
 #define MAKESHIFT_SELECTANY_PREFIX
 #define MAKESHIFT_SELECTANY_SUFFIX __attribute__((weak))
#else // compilers
 #define MAKESHIFT_SELECTANY_PREFIX static
 #define MAKESHIFT_SELECTANY_SUFFIX
#endif // compilers

#ifdef _MSC_VER
 #define MAKESHIFT_EMPTY_BASES __declspec(empty_bases) // selectively enable empty base class optimization
#else // _MSC_VER
 #define MAKESHIFT_EMPTY_BASES
#endif // _MSC_VER

#ifdef _MSC_VER
 #define MAKESHIFT_NOVTABLE __declspec(novtable) // do not generate VMT for the given class
#else // _MSC_VER
 #define MAKESHIFT_NOVTABLE
#endif // _MSC_VER

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

#ifdef _MSC_VER
 #define MAKESHIFT_VECTORCALL __vectorcall
#else // _MSC_VER
 #define MAKESHIFT_VECTORCALL
#endif // _MSC_VER

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
 #define MAKESHIFT_INTELLISENSE_PARSER
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)

#define MAKESHIFT_NODISCARD [[nodiscard]]
#define MAKESHIFT_MAYBE_UNUSED [[maybe_unused]]

#define MAKESHIFT_CONSTEXPR_CXX17 constexpr
#define MAKESHIFT_CONSTEXPR_CXX20

#define MAKESHIFT_CXX17


#endif // INCLUDED_MAKESHIFT_VERSION_HPP_
