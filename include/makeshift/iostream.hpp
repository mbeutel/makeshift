
#ifndef INCLUDED_MAKESHIFT_IOSTREAM_HPP_
#define INCLUDED_MAKESHIFT_IOSTREAM_HPP_


#include <iosfwd>      // for istream, ostream
#include <type_traits> // for declval<>()


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
    template <typename T,
              typename = decltype(std::declval<std::ostream&>() << std::declval<const T&>())>
        void operator ()(std::ostream& stream, const T& arg)
    {
        stream << arg;
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
    template <typename T,
              typename = decltype(std::declval<std::istream&>() >> std::declval<T&>())>
        void operator ()(std::istream& stream, T& arg)
    {
        stream >> arg;
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOSTREAM_HPP_
