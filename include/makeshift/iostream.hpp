
#ifndef INCLUDED_MAKESHIFT_IOSTREAM_HPP_
#define INCLUDED_MAKESHIFT_IOSTREAM_HPP_


#include <iosfwd> // for istream, ostream


namespace makeshift {


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
    operator ()(std::ostream& stream, T const& arg)
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
    operator ()(std::ostream& stream, T const& arg)
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
    operator ()(std::istream& stream, T& arg)
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
    operator ()(std::istream& stream, T& arg)
    -> decltype(stream >> arg)
    {
        return stream >> arg;
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOSTREAM_HPP_
