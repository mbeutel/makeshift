
#ifndef INCLUDED_MAKESHIFT_IOSFWD_HPP_
#define INCLUDED_MAKESHIFT_IOSFWD_HPP_


#include <iosfwd> // for istream, ostream

#include <makeshift/functional2.hpp> // for ??
#include <makeshift/version.hpp>     // for MAKESHIFT_NODISCARD


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Function object which wraps the to-stream operator `<<`.
    //
struct stream_writer
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
    // Writes the argument to the given stream by calling `stream << arg`.
    //
static constexpr inline stream_writer to_stream = { };


    //ᅟ
    // Function object which wraps the from-stream operator `>>`.
    //
struct stream_reader
{
        //ᅟ
        // Reads the argument from the given stream by calling `stream >> arg`.
        //
    template <typename T,
              typename = decltype(std::declval<std::istream&>() >> std::declval<const T&>())>
        void operator ()(std::istream& stream, const T& arg)
    {
        stream >> arg;
    }
};

    //ᅟ
    // Reads the argument from the given stream by calling `stream >> arg`.
    //
static constexpr inline stream_writer from_stream = { };


    //ᅟ
    // Represents the ability of a type to be written to a stream with operator `<<`.
    //
struct stream_writable
{
    using default_operation = std::less<>;

    template <typename ToStreamT = default_operation>
        struct interface
    {
        template <typename T,
                  typename = decltype(std::declval<std::ostream&>() << std::declval<const T&>())>
            MAKESHIFT_NODISCARD friend constexpr std::ostream& operator <<(std::ostream&, const T& arg) noexcept
        {
            return LessT{ }(lhs, rhs);
        }
    };
};


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOSFWD_HPP_
