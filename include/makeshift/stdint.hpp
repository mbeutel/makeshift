
#ifndef INCLUDED_MAKESHIFT_STDINT_HPP_
#define INCLUDED_MAKESHIFT_STDINT_HPP_


#include <cstddef>     // for ptrdiff_t
#include <type_traits> // for integral_constant<>


namespace makeshift
{


    //ᅟ
    // Represents an index value.
    //
using index = std::ptrdiff_t;


    //ᅟ
    // Represents a difference value.
    //
using diff = std::ptrdiff_t;


    //ᅟ
    // Represents a dimension value.
    //
using dim = std::ptrdiff_t;


    //ᅟ
    // Represents an array stride.
    //
using stride = std::ptrdiff_t;


    //ᅟ
    // Represents an index constval.
    //
template <index Value> using index_constant = std::integral_constant<index, Value>;

    //ᅟ
    // Represents an index constval.
    //
template <index Value> constexpr index_constant<Value> index_c{ };


    //ᅟ
    // Represents a difference constval.
    //
template <diff Value> using diff_constant = std::integral_constant<diff, Value>;

    //ᅟ
    // Represents a difference constval.
    //
template <diff Value> constexpr diff_constant<Value> diff_c{ };


    //ᅟ
    // Represents a dimension constval.
    //
template <dim Value> using dim_constant = std::integral_constant<dim, Value>;

    //ᅟ
    // Represents an dimension constval.
    //
template <dim Value> constexpr dim_constant<Value> dim_c{ };


    //ᅟ
    // Represents an array stride constval.
    //
template <stride Value> using stride_constant = std::integral_constant<stride, Value>;

    //ᅟ
    // Represents an array stride constval.
    //
template <stride Value> constexpr stride_constant<Value> stride_c{ };


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STDINT_HPP_
