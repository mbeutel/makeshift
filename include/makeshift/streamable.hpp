
#ifndef INCLUDED_MAKESHIFT_STREAMABLE_HPP_
#define INCLUDED_MAKESHIFT_STREAMABLE_HPP_


#include <utility>     // for forward<>()
#include <iosfwd>      // for istream, ostream

#include <makeshift/type_traits.hpp> // for tag<>, remove_rvalue_reference<>


namespace makeshift
{

namespace detail
{


template <typename T, typename SerializerT>
    struct streamable_t
{
private:
    T value_;
    SerializerT serializer_;

public:
    constexpr streamable_t(const T& _value, SerializerT _serializer) : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }
    constexpr streamable_t(T&& _value, SerializerT _serializer) : value_(std::move(_value)), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_t& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    struct streamable_t<const T&, SerializerT>
{
private:
    const T& value_;
    SerializerT serializer_;

public:
    constexpr streamable_t(const T& _value, SerializerT _serializer) : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_t& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    struct streamable_t<T&, SerializerT>
{
private:
    T& value_;
    SerializerT serializer_;

public:
    constexpr streamable_t(T& _value, SerializerT _serializer) : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_t& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable_t& self)
    {
        from_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    streamable_t(T&& value, SerializerT&& serializer) -> streamable_t<remove_rvalue_reference_t<T>, remove_rvalue_reference_t<SerializerT>>;


} // namespace detail


inline namespace serialize
{


    // To customize stream serialization for arbitrary types, define your own serializer type along with `to_stream_impl()`, `from_stream_impl()`
    // overloads discoverable by ADL.
    //
    // To override parts of the behavior of an existing serializer, define a new serializer that handles only the types you want to customize, and
    // chain it with the existing serializer using `chain()`.


    //ᅟ
    // Wraps the given lvalue or rvalue as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size(), stream_serializer()) << '\n';
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i, stream_serializer());
    //
template <typename T, typename SerializerT>
    auto streamable(T&& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_t{ std::forward<T>(value), std::forward<SerializerT>(serializer) };
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STREAMABLE_HPP_
