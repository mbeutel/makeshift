
#ifndef INCLUDED_MAKESHIFT_STREAMABLE_HPP_
#define INCLUDED_MAKESHIFT_STREAMABLE_HPP_


#include <utility>     // for forward<>()
#include <iosfwd>      // for istream, ostream

#include <makeshift/type_traits.hpp> // for tag<>


namespace makeshift
{

inline namespace serialize
{


    // To customize stream serialization for arbitrary types, define your own serializer type along with `to_stream_impl()`, `from_stream_impl()`
    // overloads discoverable by ADL.
    //
    // To override parts of the behavior of an existing serializer, define a new serializer that handles only the types you want to customize, and
    // chain it with the existing serializer using `chain()`.


    // defined in serializers/stream.hpp

template <typename BaseT>
    struct stream_serializer;


    //ᅟ
    // Wraps the given lvalue or rvalue as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size(), stream_serializer()) << '\n';
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i, stream_serializer());
    //
template <typename T, typename SerializerT = stream_serializer<void>>
    struct streamable
{
private:
    T value_;
    SerializerT serializer_;

public:
    template <typename = std::enable_if_t<std::is_default_constructible<T>::value && std::is_default_constructible<SerializerT>::value>>
        constexpr streamable(void)
            : value_{ }, serializer_{ }
    {
    }
    template <typename = std::enable_if_t<std::is_copy_constructible<T>::value && std::is_default_constructible<SerializerT>::value>>
        constexpr streamable(const T& _value)
            : value_(_value), serializer_{ }
    {
    }
    template <typename = std::enable_if_t<std::is_move_constructible<T>::value && std::is_default_constructible<SerializerT>::value>>
        constexpr streamable(T&& _value)
            : value_(std::move(_value)), serializer_{ }
    {
    }
    template <typename = std::enable_if_t<std::is_copy_constructible<T>::value>>
        constexpr streamable(const T& _value, SerializerT _serializer)
            : value_(_value), serializer_(std::forward<SerializerT>(_serializer))
    {
    }
    template <typename = std::enable_if_t<std::is_move_constructible<T>::value>>
        constexpr streamable(T&& _value, SerializerT _serializer)
            : value_(std::move(_value)), serializer_(std::forward<SerializerT>(_serializer))
    {
    }

    friend std::ostream& operator <<(std::ostream& stream, const streamable& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable& self)
    {
        from_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }

    constexpr const T& value(void) const & noexcept { return value_; }
    constexpr T value(void) && noexcept { return std::move(value_); }
};
template <typename T, typename SerializerT>
    struct streamable<T&&, SerializerT>
{
private:
    T value_;
    SerializerT serializer_;

public:
    template <typename = std::enable_if_t<std::is_default_constructible<SerializerT>::value>>
        constexpr streamable(T&& _value)
            : value_(std::move(_value)), serializer_{ }
    {
    }
    constexpr streamable(T&& _value, SerializerT _serializer)
        : value_(std::move(_value)), serializer_(std::forward<SerializerT>(_serializer))
    {
    }

    friend std::ostream& operator <<(std::ostream& stream, const streamable& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    struct streamable<const T&, SerializerT>
{
private:
    const T& value_;
    SerializerT serializer_;

public:
    template <typename = std::enable_if_t<std::is_default_constructible<SerializerT>::value>>
        constexpr streamable(const T& _value)
            : value_(_value), serializer_{ }
    {
    }
    constexpr streamable(const T& _value, SerializerT _serializer)
        : value_(_value), serializer_(std::forward<SerializerT>(_serializer))
    {
    }

    friend std::ostream& operator <<(std::ostream& stream, const streamable& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    struct streamable<T&, SerializerT>
{
private:
    T& value_;
    SerializerT serializer_;

public:
    template <typename = std::enable_if_t<std::is_default_constructible<SerializerT>::value>>
        constexpr streamable(T& _value)
            : value_(_value), serializer_{ }
    {
    }
    constexpr streamable(T& _value, SerializerT _serializer)
        : value_(_value), serializer_(std::forward<SerializerT>(_serializer))
    {
    }

    friend std::ostream& operator <<(std::ostream& stream, const streamable& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable& self)
    {
        from_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
};
template <typename T>
    streamable(T&& value) -> streamable<T&&>;
template <typename T, typename SerializerT>
    streamable(T&& value, SerializerT&& serializer) -> streamable<T&&, SerializerT>;


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STREAMABLE_HPP_
