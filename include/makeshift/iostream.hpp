
#ifndef INCLUDED_MAKESHIFT_IOSTREAM_HPP_
#define INCLUDED_MAKESHIFT_IOSTREAM_HPP_


#include <iosfwd> // for istream, ostream

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP20_OR_GREATER

#if !gsl_CPP20_OR_GREATER
# error makeshift requires C++20 mode or higher
#endif // !gsl_CPP20_OR_GREATER


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Function object which wraps the to-stream operator `<<`.
    //
template <typename T = void>
struct to_stream
{
        //
        // Writes the argument to the given stream by calling `stream << arg`.
        //
    auto
    operator ()(std::ostream& stream, T const& arg) const
    -> decltype(stream << arg)
    {
        return stream << arg;
    }
};

    //
    // Function object which wraps the to-stream operator `<<`.
    //
template <>
struct to_stream<>
{
        //
        // Writes the argument to the given stream by calling `stream << arg`.
        //
    template <typename T>
    auto
    operator ()(std::ostream& stream, T const& arg) const
    -> decltype(stream << arg)
    {
        return stream << arg;
    }
};


    //
    // Function object which wraps the from-stream operator `>>`.
    //
template <typename T = void>
struct from_stream
{
        //
        // Reads the argument from the given stream by calling `stream >> arg`.
        //
    auto
    operator ()(std::istream& stream, T& arg) const
    -> decltype(stream >> arg)
    {
        return stream >> arg;
    }
};

    //
    // Function object which wraps the from-stream operator `>>`.
    //
template <>
struct from_stream<>
{
        //
        // Reads the argument from the given stream by calling `stream >> arg`.
        //
    template <typename T>
    auto
    operator ()(std::istream& stream, T& arg) const
    -> decltype(stream >> arg)
    {
        return stream >> arg;
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOSTREAM_HPP_
