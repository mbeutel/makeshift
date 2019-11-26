
#ifndef INCLUDED_MAKESHIFT_DETAIL_MACROS_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_MACROS_HPP_


    //ᅟ
    // `MAKESHIFT_DETAIL_CXXLEVEL` specifies the base level of language support.
    // This macro is not configurable, but it serves as a marker for code sections that can be simplified when support for older
    // C++ standards is dropped.
    //
#define MAKESHIFT_DETAIL_CXXLEVEL  14


    //ᅟ
    // `MAKESHIFT_DETAIL_EMPTY_BASES` can be used to selectively enable the empty base optimization for a given type.
    //
#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000
    // 
    // __declspec(empty_bases) was added in VC++ 2015 Update 2 and is expected to become unnecessary in the next ABI-breaking release.
# define MAKESHIFT_DETAIL_EMPTY_BASES __declspec(empty_bases)
#else // defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000
# define MAKESHIFT_DETAIL_EMPTY_BASES
#endif // defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_FULL_VER >= 190023918 && _MSC_VER < 2000


#endif // INCLUDED_MAKESHIFT_DETAIL_MACROS_HPP_
