
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT_TYPE_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT_TYPE_VARIANT_HPP_


#include <cstddef>     // for size_t
#include <variant>     // for monostate, variant_size<>, variant_alternative<> 
#include <optional>
#include <utility>     // for move(), forward<>(), get<>, in_place_index<>, in_place_type<>, swap()
#include <type_traits> // for enable_if<>, decay<>, integral_constant<>, index_sequence<>, is_nothrow_default_constructible<>

#include <makeshift/type_traits.hpp> // for can_apply<>

#include <makeshift/detail/workaround.hpp> // for cand()


namespace makeshift
{

namespace detail
{


struct runtime_index_t { };


} // namespace detail


inline namespace types
{


    //ᅟ
    // Type variant (variant without runtime value representation).
    //
template <typename... Ts>
    struct type_variant
{
    static_assert(sizeof...(Ts) > 0, "a variant with no type arguments is ill-formed");
    static_assert(makeshift::detail::cand(std::is_nothrow_default_constructible<Ts>::value...), "variant types must be no-throw default-constructible");

private:
    std::size_t index_;

public:
    template <typename = std::enable_if_t<std::is_same<nth_type_t<0, Ts...>, std::monostate>::value>>
        constexpr type_variant(void) noexcept
            : index_(0)
    {
    }
    constexpr type_variant(const type_variant&) noexcept = default;
    constexpr type_variant& operator =(const type_variant&) noexcept = default;
    template <typename T>
        constexpr type_variant(T&&) noexcept
            : index_(makeshift::detail::value_overload_index_v<T, Ts...>)
    {
    }
    template <typename T>
        explicit constexpr type_variant(std::in_place_type_t<T>) noexcept
            : index_(index_of_type_v<T, Ts...>)
    {
    }
    template <std::size_t I>
        explicit constexpr type_variant(std::in_place_index_t<I>) noexcept
            : index_(I)
    {
    }
    explicit constexpr type_variant(makeshift::detail::runtime_index_t, std::size_t _index)
        : index_(_index)
    {
        Expects(_index >= sizeof...(Ts));
    }

        //ᅟ
        // Returns the zero-based index of the alternative that is currently held by the variant.
        //
    constexpr std::size_t index(void) const noexcept { return index_; }

        //ᅟ
        // Creates a new value of type `T` in-place in an existing variant object.
        //
    template <typename T>
        constexpr T emplace(void) noexcept
    {
        index_ = index_of_type_v<T, Ts...>;
        return { };
    }

        //ᅟ
        // Creates a new value with index `I` in-place in an existing variant object.
        //
    template <std::size_t I>
        constexpr nth_type_t<I, Ts...> emplace(void) noexcept
    {
        index_ = I;
        return { };
    }

        //ᅟ
        // Swaps two variant objects.
        //
    constexpr void swap(type_variant& rhs) noexcept
    {
        std::swap(index_, rhs.index_);
    }
};

template <typename... Ts> constexpr bool operator ==(const type_variant<Ts...>& v, const type_variant<Ts...>& w) noexcept { return v.index() == w.index(); }
template <typename... Ts> constexpr bool operator !=(const type_variant<Ts...>& v, const type_variant<Ts...>& w) noexcept { return v.index() != w.index(); }
template <typename... Ts> constexpr bool operator > (const type_variant<Ts...>& v, const type_variant<Ts...>& w) noexcept { return v.index() <  w.index(); }
template <typename... Ts> constexpr bool operator < (const type_variant<Ts...>& v, const type_variant<Ts...>& w) noexcept { return v.index() >  w.index(); }
template <typename... Ts> constexpr bool operator <=(const type_variant<Ts...>& v, const type_variant<Ts...>& w) noexcept { return v.index() <= w.index(); }
template <typename... Ts> constexpr bool operator >=(const type_variant<Ts...>& v, const type_variant<Ts...>& w) noexcept { return v.index() >= w.index(); }

template <typename T, typename... Ts>
    constexpr bool holds_alternative(const type_variant<Ts...>& v) noexcept
{
    return v.index() == index_of_type_v<T, Ts...>;
}


    //ᅟ
    // Returns the `I`-th alternative in the type variant.
    //
template <std::size_t I, typename... Ts>
    constexpr nth_type_t<I, Ts...> get(const type_variant<Ts...>& v)
{
    static_assert(I < sizeof...(Ts), "variant index out of range");
    Expects(v.index() == I);
    return { };
}

    //ᅟ
    // Returns the type variant alternative of type `T`.
    //
template <typename T, typename... Ts>
    constexpr T get(const type_variant<Ts...>& v)
{
    static_assert(try_index_of_type_v<T, Ts...> != std::size_t(-1), "type T does not appear in type sequence");
    Expects((v.index() == try_index_of_type_v<T, Ts...>));
    return { };
}


    //ᅟ
    // Returns the `I`-th alternative in the type variant, or `std::nullopt` if the variant does not currently hold the `I`-th alternative.
    //
template <std::size_t I, typename... Ts>
    constexpr std::optional<nth_type_t<I, Ts...>> try_get(const type_variant<Ts...>& v) noexcept
{
    static_assert(I < sizeof...(Ts), "variant index out of range");
    if (v.index() == I)
        return nth_type_t<I, Ts...>{ };
    else
        return std::nullopt;
}

    //ᅟ
    // Returns the type variant alternative of type `T`, or `std::nullopt` if the variant does not currently hold the alternative of type `T`.
    //
template <typename T, typename... Ts>
    constexpr T try_get(const type_variant<Ts...>& v) noexcept
{
    static_assert(try_index_of_type_v<T, Ts...> != std::size_t(-1), "type T does not appear in type sequence");
    if (v.index() == try_index_of_type_v<T, Ts...>)
        return T{ };
    else
        return std::nullopt;
}


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `variant_size<>` and `variant_alternative<>` for `type_variant<>`.
template <typename... Ts> class variant_size<makeshift::type_variant<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class variant_alternative<I, makeshift::type_variant<Ts...>> : public makeshift::detail::nth_type_<I, Ts...> { };


} // namespace std


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT_TYPE_VARIANT_HPP_
