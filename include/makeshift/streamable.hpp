
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
    struct streamable_rvalue
{
    streamable_rvalue(const streamable_rvalue&) = delete;
    streamable_rvalue(streamable_rvalue&&) = delete;
    streamable_rvalue& operator =(const streamable_rvalue&) = delete;
    streamable_rvalue& operator =(streamable_rvalue&&) = delete;

private:
    const T& value_;
    SerializerT serializer_;

public:
    constexpr streamable_rvalue(const T& _value, SerializerT _serializer) noexcept : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_rvalue& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    streamable_rvalue(const T& value, SerializerT&& serializer) -> streamable_rvalue<T, remove_rvalue_reference_t<SerializerT>>;

template <typename T, typename SerializerT>
    struct streamable_lvalue
{
    streamable_lvalue(const streamable_lvalue&) = delete;
    streamable_lvalue(streamable_lvalue&&) = delete;
    streamable_lvalue& operator =(const streamable_lvalue&) = delete;
    streamable_lvalue& operator =(streamable_lvalue&&) = delete;

private:
    T& value_;
    SerializerT serializer_;

public:
    constexpr streamable_lvalue(T& _value, SerializerT _serializer) noexcept : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_lvalue& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable_lvalue& self)
    {
        from_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    streamable_lvalue(T& value, SerializerT&& serializer) -> streamable_lvalue<T, remove_rvalue_reference_t<SerializerT>>;


} // namespace detail


inline namespace serialize
{


    // To customize stream serialization for arbitrary types, define your own serializer type along with `to_stream_impl()`, `from_stream_impl()`
    // overloads discoverable by ADL.
    //
    // To override parts of the behavior of an existing serializer, define a new serializer that handles only the types you want to customize, and
    // chain it with the existing serializer using `chain_serializers()`.


    //ᅟ
    // Wraps the given rvalue as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size(), stream_serializer) << '\n';
    //
template <typename T, typename SerializerT>
    auto streamable(const T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_rvalue { value, std::forward<SerializerT>(serializer) };
}


    //ᅟ
    // Wraps the given lvalue as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i, stream_serializer);
    //ᅟ    std::cout << streamable(i, stream_serializer) << '\n';
    //
template <typename T, typename SerializerT>
    auto streamable(T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_lvalue { value, std::forward<SerializerT>(serializer) };
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STREAMABLE_HPP_
