
#ifndef INCLUDED_MAKESHIFT_STREAMABLE_HPP_
#define INCLUDED_MAKESHIFT_STREAMABLE_HPP_


#include <utility>     // for forward<>()
#include <iosfwd>      // for istream, ostream

#include <makeshift/type_traits.hpp> // for tag<>

#include <makeshift/detail/cfg.hpp> // for MAKESHIFT_DLLFUNC


namespace makeshift
{

namespace detail
{


template <typename T> struct as_lvalue{ using type = const T; };
template <typename T> struct as_lvalue<T&> { using type = T; };
template <typename T> struct as_lvalue<const T&> { using type = const T; };
template <typename T> struct as_lvalue<T&&> { using type = const T; };
template <typename T> using as_lvalue_t = typename as_lvalue<T>::type;


template <typename T, typename SerializerT>
    struct streamable_ref_base
{
    streamable_ref_base(const streamable_ref_base&) = delete;
    streamable_ref_base& operator =(const streamable_ref_base&) = delete;

private:
    T& value_;
    SerializerT& serializer_;

public:
    constexpr streamable_ref_base(T& _value, SerializerT& _serializer) noexcept : value_(_value), serializer_(_serializer) { }

    constexpr T& value(void) const noexcept { return value_; }
    constexpr SerializerT& serializer(void) const noexcept { return serializer_; }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_ref_base& self)
    {
        to_stream_impl(self.value(), stream, self.serializer(), self.serializer());
        return stream;
    }
};
template <typename T>
    struct streamable_ref_base<T, void>;


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
    //ᅟ    std::cout << streamable(vec.size(), stream_serializer{ }) << '\n';
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i, stream_serializer{ });
    //
template <typename T, typename SerializerT = void>
    struct streamable_ref : makeshift::detail::streamable_ref_base<T, SerializerT>
{
    using base = makeshift::detail::streamable_ref_base<T, SerializerT>;
    using base::base;

    friend std::istream& operator >>(std::istream& stream, const streamable_ref& self)
    {
        from_stream_impl(self.value(), stream, self.serializer(), self.serializer());
        return stream;
    }
};
template <typename T, typename SerializerT>
    struct streamable_ref<const T, SerializerT> : makeshift::detail::streamable_ref_base<const T, SerializerT>
{
    using base = makeshift::detail::streamable_ref_base<const T, SerializerT>;
    using base::base;
};
template <typename T>
    struct streamable_ref<T, void>;
template <typename T>
    struct streamable_ref<const T, void>;
template <typename T>
    streamable_ref(T&&) -> streamable_ref<makeshift::detail::as_lvalue_t<T>, void>;
template <typename T, typename SerializerT>
    streamable_ref(T&&, SerializerT&&) -> streamable_ref<makeshift::detail::as_lvalue_t<T>, makeshift::detail::as_lvalue_t<SerializerT>>;


    //ᅟ
    // Wraps the given reference as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size(), stream_serializer{ }) << '\n';
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i, stream_serializer{ });
    //
template <typename T, typename SerializerT>
    constexpr streamable_ref<makeshift::detail::as_lvalue_t<T>, makeshift::detail::as_lvalue_t<SerializerT>>
    streamable(T&& value, SerializerT&& serializer) noexcept
{
    return { value, serializer };
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STREAMABLE_HPP_
