
#ifndef INCLUDED_MAKESHIFT_DETAIL_MACROS_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_MACROS_HPP_


    //
    // `MAKESHIFT_DETAIL_EMPTY_BASES` can be used to selectively enable the empty base optimization for a given type.
    //
#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000
    // 
    // __declspec(empty_bases) was added in VC++ 2015 Update 2 and is expected to become unnecessary in the next ABI-breaking release.
# define MAKESHIFT_DETAIL_EMPTY_BASES __declspec(empty_bases)
#else // defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000
# define MAKESHIFT_DETAIL_EMPTY_BASES
#endif // defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000


    //
    // `MAKESHIFT_DETAIL_FORCEINLINE` attempts to force the compiler to inline the given function.
    //
#if defined(_MSC_VER)
# define MAKESHIFT_DETAIL_FORCEINLINE  __forceinline
#elif defined(__INTEL_COMPILER)
# define MAKESHIFT_DETAIL_FORCEINLINE  __forceinline // TODO: ??
#elif defined(__GNUC__)
# define MAKESHIFT_DETAIL_FORCEINLINE  __attribute__((always_inline)) inline
#else
# define MAKESHIFT_DETAIL_FORCEINLINE  inline
#endif


#endif // INCLUDED_MAKESHIFT_DETAIL_MACROS_HPP_
