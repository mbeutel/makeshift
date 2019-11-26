
#ifndef INCLUDED_MAKESHIFT_MACROS_HPP_
#define INCLUDED_MAKESHIFT_MACROS_HPP_


#include <makeshift/detail/macros.hpp>


    //ᅟ
    // `MAKESHIFT_FORCEINLINE` and `MAKESHIFT_NOINLINE` expand to compiler-specific annotations to either enforce or
    // inhibit inlining of the annotated function.
    //
#if defined(_MSC_VER)
# define MAKESHIFT_FORCEINLINE  __forceinline
# define MAKESHIFT_NOINLINE     __declspec(noinline)
#elif defined(__INTEL_COMPILER)
# define MAKESHIFT_FORCEINLINE  __forceinline // TODO: ??
# define MAKESHIFT_NOINLINE     __attribute__((noinline)) // TODO: ??
#elif defined(__GNUC__)
# define MAKESHIFT_FORCEINLINE  __attribute__((always_inline)) inline
# define MAKESHIFT_NOINLINE     __attribute__((noinline))
#else
# define MAKESHIFT_FORCEINLINE  inline
# define MAKESHIFT_NOINLINE
#endif


#endif // INCLUDED_MAKESHIFT_MACROS_HPP_
