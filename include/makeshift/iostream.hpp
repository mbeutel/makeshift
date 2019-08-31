
#ifndef INCLUDED_MAKESHIFT_IOSTREAM_HPP_
#define INCLUDED_MAKESHIFT_IOSTREAM_HPP_


#include <iosfwd> // for istream, ostream


namespace makeshift
{


    //ᅟ
    // Function object which wraps the to-stream operator `<<`.
    //
struct to_stream
{
        //ᅟ
        // Writes the argument to the given stream by calling `stream << arg`.
        //
    template <typename T>
        auto operator ()(std::ostream& stream, T const& arg)
            -> decltype(stream << arg)
    {
        return stream << arg;
    }
};


    //ᅟ
    // Function object which wraps the from-stream operator `>>`.
    //
struct from_stream
{
        //ᅟ
        // Reads the argument from the given stream by calling `stream >> arg`.
        //
    template <typename T>
        auto operator ()(std::istream& stream, T& arg)
            -> decltype(stream >> arg)
    {
        return stream >> arg;
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOSTREAM_HPP_
