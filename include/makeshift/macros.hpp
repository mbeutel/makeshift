
#ifndef INCLUDED_MAKESHIFT_MACROS_HPP_
#define INCLUDED_MAKESHIFT_MACROS_HPP_


#include <makeshift/detail/macros.hpp>


    //ᅟ
    // `MAKESHIFT_CXX` specifies the level of language support available.
    // Possible values: 14, 17, 20.
    //
    // (TODO: replace with gsl_CPLUSPLUS?)
    //
#if defined(_MSVC_LANG ) && !defined(__clang__)
 #define MAKESHIFT_CPLUSPLUS _MSVC_LANG
#else
 #define MAKESHIFT_CPLUSPLUS __cplusplus
#endif
#if MAKESHIFT_CPLUSPLUS >= 202000L // speculative
 #define MAKESHIFT_CXX  20
#elif MAKESHIFT_CPLUSPLUS >= 201703L
 #define MAKESHIFT_CXX  17
#elif MAKESHIFT_CPLUSPLUS >= 201402L
 #define MAKESHIFT_CXX  14
#else
 #error makeshift requires compiler support for C++14 or newer.
#endif


    //ᅟ
    // `MAKESHIFT_CXXLEVEL` specifies the base level of language support.
    // This macro is not configurable, but it serves as a marker for code sections that can be simplified when support for older
    // C++ standards is dropped.
    //
#define MAKESHIFT_CXXLEVEL  14


    //ᅟ
    // `MAKESHIFT_IF_CXX(L,...)` expands to `...` if the language support matches the value given by `L`, and to nothing otherwise.
    // `MAKESHIFT_IF_NOT_CXX(L,...)` expands to nothing if the language support matches the value given by `L`, and to `...` otherwise.
    // Possible values for L: 14, 17, 20.
    //
    // (TODO: move to gsl-lite?)
    //
#if MAKESHIFT_CXX >= 20
 #define MAKESHIFT_IF_CXX_20_(...)      __VA_ARGS__
 #define MAKESHIFT_IF_NOT_CXX_20_(...)
#else
 #define MAKESHIFT_IF_CXX_20_(...)
 #define MAKESHIFT_IF_NOT_CXX_20_(...)  __VA_ARGS__
#endif
#if MAKESHIFT_CXX >= 17
 #define MAKESHIFT_IF_CXX_17_(...)      __VA_ARGS__
 #define MAKESHIFT_IF_NOT_CXX_17_(...)
#else
 #define MAKESHIFT_IF_CXX_17_(...)
 #define MAKESHIFT_IF_NOT_CXX_17_(...)  __VA_ARGS__
#endif
#define MAKESHIFT_IF_CXX_14_(...)       __VA_ARGS__
#define MAKESHIFT_IF_NOT_CXX_14_(...)
#define MAKESHIFT_IF_CXX_(F,...)        F(__VA_ARGS__)
#define MAKESHIFT_IF_CXX(L,...)         MAKESHIFT_IF_CXX_(MAKESHIFT_IF_CXX_##L##_,__VA_ARGS__)
#define MAKESHIFT_IF_NOT_CXX(L,...)     MAKESHIFT_IF_CXX_(MAKESHIFT_IF_NOT_CXX_##L##_,__VA_ARGS__)


    //ᅟ
    // `MAKESHIFT_INTELLISENSE` is defined if the compiler is a parser for user interaction.
    // This can be used to elide massive template instantiations in interactive scenarios (e.g. in `visit()`),
    // which can improve responsiveness.
    //
#if defined(_MSC_VER) && defined(__INTELLISENSE__)
 #define MAKESHIFT_INTELLISENSE
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)


    //ᅟ
    // `MAKESHIFT_FORCEINLINE` and `MAKESHIFT_NOINLINE` expand to compiler-specific annotations to either enforce or
    // inhibit inlining of the annotated function.
    //
#if defined(_MSC_VER)
 #define MAKESHIFT_FORCEINLINE  __forceinline
 #define MAKESHIFT_NOINLINE     __declspec(noinline)
#elif defined(__INTEL_COMPILER)
 #define MAKESHIFT_FORCEINLINE  __forceinline // TODO: ??
 #define MAKESHIFT_NOINLINE     __attribute__((noinline)) // TODO: ??
#elif defined(__GNUC__)
 #define MAKESHIFT_FORCEINLINE  __attribute__((always_inline)) inline
 #define MAKESHIFT_NOINLINE     __attribute__((noinline))
#else
 #define MAKESHIFT_FORCEINLINE  inline
 #define MAKESHIFT_NOINLINE
#endif


    //ᅟ
    // Some common abbreviations.
    //
#define MAKESHIFT_NODISCARD    MAKESHIFT_IF_CXX(17, [[nodiscard]])
#define MAKESHIFT_CONSTEXPR17  MAKESHIFT_IF_CXX(17, constexpr)
#define MAKESHIFT_CONSTEXPR20  MAKESHIFT_IF_CXX(20, constexpr)


#endif // INCLUDED_MAKESHIFT_MACROS_HPP_
