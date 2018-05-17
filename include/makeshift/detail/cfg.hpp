
#ifndef MAKESHIFT_DETAIL_CFG_HPP_
#define MAKESHIFT_DETAIL_CFG_HPP_


#include <utility>


#pragma region User configuration
// User configuration options:
// ===========================
//
//     MAKESHIFT_DEBUG_INDEX_CHECK
//     Define this macro in the build environment of your project to enable range checks for all indexing operations.
//     (This macro only affects inline functions defined in headers, so it does neither impair binary compatibility nor violate the ODR.)
#pragma endregion User configuration


#pragma region ABI
#define MAKESHIFT_DETAIL_CONCAT3_(a,b,c) a ## b ## c
#define MAKESHIFT_DETAIL_CONCAT7_(a,b,c,d,e,f,g) a ## b ## c ## d ## e ## f ## g
#define MAKESHIFT_DETAIL_CONCAT3(a,b,c) MAKESHIFT_DETAIL_CONCAT3_(a,b,c)
#define MAKESHIFT_DETAIL_CONCAT7(a,b,c,d,e,f,g) MAKESHIFT_DETAIL_CONCAT7_(a,b,c,d,e,f,g)

#if defined(_MSC_VER)

 // distinguish Visual C++ toolkit versions, which indicate binary compatibility
 #if _MSC_VER >= 1800 && _MSC_VER < 1900
  #define MAKESHIFT_DETAIL_ABI_VERSION 18
 #elif _MSC_VER >= 1900 && _MSC_VER < 2000
  #define MAKESHIFT_DETAIL_ABI_VERSION 19
 #elif _MSC_VER >= 2000 && _MSC_VER < 2100
  #define MAKESHIFT_DETAIL_ABI_VERSION 20
 #elif _MSC_VER >= 2100 && _MSC_VER < 2200
  #define MAKESHIFT_DETAIL_ABI_VERSION 21
 #elif _MSC_VER >= 2200 && _MSC_VER < 2300
  #define MAKESHIFT_DETAIL_ABI_VERSION 22
 #elif _MSC_VER >= 2300 && _MSC_VER < 2400
  #define MAKESHIFT_DETAIL_ABI_VERSION 23
 #elif _MSC_VER >= 2400 && _MSC_VER < 2500
  #define MAKESHIFT_DETAIL_ABI_VERSION 24
 #elif _MSC_VER >= 2500 && _MSC_VER < 2600
  #define MAKESHIFT_DETAIL_ABI_VERSION 25
 #else
  #error Undetected Visual C++ toolkit version. Please update this header file accordingly.
 #endif
 
 // distinguish Debug and Release builds (some STL types have binary incompatibilities due to checked iterators)
 #ifdef _DEBUG
  #define MAKESHIFT_DETAIL_ABI_DEBUG d
 #else //  _DEBUG
  #define MAKESHIFT_DETAIL_ABI_DEBUG
 #endif //  _DEBUG
 
 // define MAKESHIFT_STATIC_LIBRARIES if not using the dynamic RTL
 // (this is not part of the ABI string because mixing static/dynamic builds is caught by the linker)
 #ifndef _DLL
  #define MAKESHIFT_STATIC_LIBRARIES
 #endif // _DLL

 #define MAKESHIFT_DETAIL_ABI MAKESHIFT_DETAIL_CONCAT3(vc,MAKESHIFT_DETAIL_ABI_VERSION,MAKESHIFT_DETAIL_ABI_DEBUG)
 
#elif defined(__GNUC__) || defined(__clang__)

 // we don't bother with an ABI version here because g++/libstdc++ tends to be quite stable; maybe we should?
 #define MAKESHIFT_DETAIL_ABI gcc
 
#else // compilers

 #error Unknown compiler. Please update this header file to recognize the compiler.
 
#endif // compilers
#pragma endregion ABI


#pragma region Shared library implementation details
#ifdef _WIN32
 #ifdef MAKESHIFT_STATIC_LIBRARIES
  #define MAKESHIFT_EXPORT
  #define MAKESHIFT_IMPORT
 #else // MAKESHIFT_STATIC_LIBRARIES
  #define MAKESHIFT_EXPORT __declspec(dllexport)
  #define MAKESHIFT_IMPORT __declspec(dllimport)
 #endif // MAKESHIFT_STATIC_LIBRARIES
#else
 #define MAKESHIFT_EXPORT
 #define MAKESHIFT_IMPORT
#endif

#if defined(_MSC_VER)
 #define MAKESHIFT_DETAIL_SELECTANY_PREFIX __declspec(selectany)
 #define MAKESHIFT_DETAIL_SELECTANY_SUFFIX
#elif defined(__GNUC__) || defined(__clang__)
 #define MAKESHIFT_DETAIL_SELECTANY_PREFIX
 #define MAKESHIFT_DETAIL_SELECTANY_SUFFIX __attribute__((weak))
#else // compilers
 #define MAKESHIFT_DETAIL_SELECTANY_PREFIX static
 #define MAKESHIFT_DETAIL_SELECTANY_SUFFIX
#endif // compilers
#pragma endregion Shared library implementation details


#pragma region Internals
#ifdef MAKESHIFT_DEBUG_INDEX_CHECK
 #define MAKESHIFT_INDEX_CHECK_NOEXCEPT
#else // MAKESHIFT_DEBUG_INDEX_CHECK
 #define MAKESHIFT_INDEX_CHECK_NOEXCEPT noexcept
#endif // MAKESHIFT_DEBUG_INDEX_CHECK
#pragma endregion Internals


#pragma region Compiler-specific workarounds
#if defined(_MSC_VER)
 #define MAKESHIFT_NESTED_PARAMETER_PACK_EXPANSION_BUG
#endif // defined(_MSC_VER)

#if defined(_MSC_VER) && !defined(_SCL_SECURE_NO_WARNINGS) && !defined(_SCL_SECURE_NO_DEPRECATE) // copy for insecure iterators
 #define MAKESHIFT_INSECURE_COPY // define tover::detail::copy() as an unchecked variant of std::copy()
#endif // defined(_MSC_VER)

#if defined(_MSC_VER) || defined(__clang__)
 #define MAKESHIFT_HAVE_UNICODE_IDENTIFIERS
#endif // defined(_MSC_VER) || defined(__clang__)

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
 #define MAKESHIFT_INTELLISENSE_PARSER
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)

#ifdef _MSC_VER
 #define MAKESHIFT_EMPTY_BASES __declspec(empty_bases) // selectively enable empty base class optimization
#else // _MSC_VER
 #define MAKESHIFT_EMPTY_BASES
#endif // _MSC_VER

#if defined(_MSC_VER) && _HAS_CXX17
 #define MAKESHIFT_UNUSED [[maybe_unused]]
#else // defined(_MSC_VER) && _HAS_CXX17
 #define MAKESHIFT_UNUSED
#endif // defined(_MSC_VER) && _HAS_CXX17

#if defined(__GNUC__)
 #define MAKESHIFT_NO_INVALID_OFFSETOF // g++ is very strict about offsetof() on non-standard-layout types
#endif // defined(__GNUC__)

namespace makeshift
{

namespace detail
{

template <class InIt, class OutIt>
    inline OutIt copy(InIt first, InIt last, OutIt dest)
{
#ifdef MAKESHIFT_INSECURE_COPY
    return std::_Copy_no_deprecate(first, last, dest);
#else // MAKESHIFT_INSECURE_COPY
    return std::copy(first, last, dest);
#endif // MAKESHIFT_INSECURE_COPY
}

} // namespace detail

} // namespace makeshift
#pragma endregion Compiler-specific workarounds


#pragma region Components
#ifdef MAKESHIFT_BUILD_MAKESHIFT_SYS
 #define MAKESHIFT_SYS_DLLFUNC MAKESHIFT_EXPORT
#else // MAKESHIFT_BUILD_MAKESHIFT_SYS
 #define MAKESHIFT_SYS_DLLFUNC MAKESHIFT_IMPORT
#endif // MAKESHIFT_BUILD_MAKESHIFT_SYS
#pragma endregion Components


#endif // MAKESHIFT_DETAIL_CFG_HPP_
