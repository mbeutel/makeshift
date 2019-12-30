
#ifndef INCLUDED_MAKESHIFT_STDINT_HPP_
#define INCLUDED_MAKESHIFT_STDINT_HPP_


#include <cstddef>     // for ptrdiff_t
#include <type_traits> // for integral_constant<>

#include <gsl-lite/gsl-lite.hpp> // for dim, index, stride, diff


namespace makeshift
{


namespace gsl = ::gsl_lite;


    // TODO: if we could do without *_constant and *_c, this entire file could be discarded


    //ᅟ
    // Represents an index constval.
    //
template <gsl::index Value> using index_constant = std::integral_constant<gsl::index, Value>;

    //ᅟ
    // Represents an index constval.
    //
template <gsl::index Value> constexpr index_constant<Value> index_c{ };


    //ᅟ
    // Represents a difference constval.
    //
template <gsl::diff Value> using diff_constant = std::integral_constant<gsl::diff, Value>;

    //ᅟ
    // Represents a difference constval.
    //
template <gsl::diff Value> constexpr diff_constant<Value> diff_c{ };


    //ᅟ
    // Represents a dimension constval.
    //
template <gsl::dim Value> using dim_constant = std::integral_constant<gsl::dim, Value>;

    //ᅟ
    // Represents an dimension constval.
    //
template <gsl::dim Value> constexpr dim_constant<Value> dim_c{ };


    //ᅟ
    // Represents an array stride constval.
    //
template <gsl::stride Value> using stride_constant = std::integral_constant<gsl::stride, Value>;

    //ᅟ
    // Represents an array stride constval.
    //
template <gsl::stride Value> constexpr stride_constant<Value> stride_c{ };


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STDINT_HPP_
