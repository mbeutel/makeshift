
#ifndef MAKESHIFT_DETAIL_CFG_HPP_
#define MAKESHIFT_DETAIL_CFG_HPP_


    // shared library implementation details
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


    // compiler-specific attributes
#if defined(_MSC_VER) || defined(__clang__)
 #define MAKESHIFT_HAVE_UNICODE_IDENTIFIERS
#endif // defined(_MSC_VER) || defined(__clang__)

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
 #define MAKESHIFT_INTELLISENSE_PARSER
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)

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


    // components
#ifdef MAKESHIFT_BUILD
 #define MAKESHIFT_DLLFUNC MAKESHIFT_EXPORT
#else // MAKESHIFT_BUILD
 #define MAKESHIFT_DLLFUNC MAKESHIFT_IMPORT
#endif // MAKESHIFT_BUILD


#endif // MAKESHIFT_DETAIL_CFG_HPP_

