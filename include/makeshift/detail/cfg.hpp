
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
#ifdef MAKESHIFT_BUILD
 #define MAKESHIFT_DLLFUNC MAKESHIFT_EXPORT
#else // MAKESHIFT_BUILD
 #define MAKESHIFT_DLLFUNC MAKESHIFT_IMPORT
#endif // MAKESHIFT_BUILD


    // internal compiler-specific attributes
#if defined(_MSC_VER) || defined(__clang__)
 #define MAKESHIFT_HAVE_UNICODE_IDENTIFIERS
#endif // defined(_MSC_VER) || defined(__clang__)

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
 #define MAKESHIFT_INTELLISENSE_PARSER
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)



#endif // MAKESHIFT_DETAIL_CFG_HPP_

