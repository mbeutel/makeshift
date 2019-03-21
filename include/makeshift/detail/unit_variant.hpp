
#ifndef INCLUDED_MAKESHIFT_DETAIL_UNIT_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UNIT_VARIANT_HPP_


#include <cstddef>     // for size_t
#include <utility>     // for move(), forward<>(), get<>, in_place_index<>, in_place_type<>, swap()
#include <type_traits> // for enable_if<>, decay<>, integral_constant<>, index_sequence<>, is_nothrow_default_constructible<>, is_empty<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp>     // for MAKESHIFT_NODISCARD, MAKESHIFT_CXX17

#include <makeshift/detail/workaround.hpp> // for cand()

#ifdef MAKESHIFT_CXX17
 #include <variant> // for monostate, variant_size<>, variant_alternative<>
#endif // MAKESHIFT_CXX17


namespace makeshift
{


#ifdef MAKESHIFT_CXX17
using monostate = std::monostate;
template <typename T> using variant_size = std::variant_size<T>;
template <std::size_t I, typename T> using variant_alternative = std::variant_alternative<I, T>;
using in_place_t = std::in_place_t;
template <typename T> using in_place_type_t = std::in_place_type_t<T>;
template <std::size_t I> using in_place_index_t = std::in_place_index_t<I>;
#else // MAKESHIFT_CXX17
struct monostate { };
template <typename T> struct variant_size;
template <typename T> class variant_size<const T> : variant_size<T> { };
template <typename T> class variant_size<volatile T> : variant_size<T> { };
template <typename T> class variant_size<const volatile T> : variant_size<T> { };
template <std::size_t I, typename T> struct variant_alternative;
template <std::size_t I, typename T> struct variant_alternative<I, const T> : variant_alternative<I, T> { };
template <std::size_t I, typename T> struct variant_alternative<I, volatile T> : variant_alternative<I, T> { };
template <std::size_t I, typename T> struct variant_alternative<I, const volatile T> : variant_alternative<I, T> { };
struct in_place_t
{
    explicit in_place_t(void) = default;
};
template <typename T>
    struct in_place_type_t
{
    explicit in_place_type_t(void) = default;
};
template <std::size_t I>
    struct in_place_index_t
{
    explicit in_place_index_t(void) = default;
};
#endif // MAKESHIFT_CXX17
constexpr inline in_place_t in_place{ };
template <typename T> constexpr inline in_place_type_t<T> in_place_type{ };
template <std::size_t I> constexpr inline in_place_index_t<I> in_place_index{ };


namespace detail
{


    // borrowed from the VC++ STL's variant implementation
template <typename T, std::size_t I>
    struct type_with_index
{
    static constexpr std::size_t index = I;
    using type = T;
};
template <std::size_t I, typename T>
    struct value_overload_
{
    using type = type_with_index<T, I> (*)(T);
    operator type(void) const;
};
template <typename Is, typename... Ts> struct value_overload_set_;
template <std::size_t... Is, typename... Ts> struct value_overload_set_<std::index_sequence<Is...>, Ts...> : value_overload_<Is, Ts>... { };
template <typename... Ts> using value_overload_set = value_overload_set_<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

template <typename EnableT, typename T, typename... Ts> struct value_overload_init_ { };
template <typename T, typename... Ts>
    struct value_overload_init_<void_t<decltype(value_overload_set<Ts...>{ }(std::declval<T>()))>, T, Ts...>
{
    using type = decltype(value_overload_set<Ts...>{ }(std::declval<T>()));
};

template <typename T, typename... Ts> struct value_overload_type { using type = typename value_overload_init_<void, T, Ts...>::type::type; };
template <typename T, typename... Ts> using value_overload_type_t = typename value_overload_init_<void, T, Ts...>::type::type::type;

template <typename T, typename... Ts> struct value_overload_index : std::integral_constant<std::size_t, value_overload_init_<void, T, Ts...>::type::index> { };
template <typename T, typename... Ts> static constexpr std::size_t value_overload_index_v = value_overload_init_<void, T, Ts...>::type::index;


struct index_value_t { };
constexpr inline index_value_t index_value{ };


    //ᅟ
    // Variant of unit types, i.e. default-constructible types without state.
    //
template <typename... Ts>
    struct unit_variant
{
    static_assert(sizeof...(Ts) > 0, "a variant with no type arguments is ill-formed");
    static_assert(makeshift::detail::cand(std::is_empty<Ts>::value && std::is_nothrow_default_constructible<Ts>::value...), "unit variant types must be no-throw default-constructible and empty");

private:
    std::size_t index_;

public:
    template <typename = std::enable_if_t<std::is_same<nth_type_t<0, Ts...>, monostate>::value>>
        constexpr unit_variant(void) noexcept
            : index_(0)
    {
    }
    constexpr unit_variant(const unit_variant&) noexcept = default;
    constexpr unit_variant& operator =(const unit_variant&) noexcept = default;
    template <typename T>
        constexpr unit_variant(T&&) noexcept
            : index_(makeshift::detail::value_overload_index_v<T, Ts...>)
    {
    }
    template <typename T>
        explicit constexpr unit_variant(in_place_type_t<T>) noexcept
            : index_(index_of_type_v<T, Ts...>)
    {
    }
    template <std::size_t I>
        explicit constexpr unit_variant(in_place_index_t<I>) noexcept
            : index_(I)
    {
    }
    constexpr explicit unit_variant(index_value_t, std::size_t _index)
        : index_(_index)
    {
        Expects(_index < sizeof...(Ts));
    }

        //ᅟ
        // Returns the zero-based index of the alternative that is currently held by the variant.
        //
    MAKESHIFT_NODISCARD constexpr std::size_t index(void) const noexcept { return index_; }

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
    constexpr void swap(unit_variant& rhs) noexcept
    {
        std::swap(index_, rhs.index_);
    }
};

template <typename... Ts> MAKESHIFT_NODISCARD constexpr bool operator ==(const unit_variant<Ts...>& v, const unit_variant<Ts...>& w) noexcept { return v.index() == w.index(); }
template <typename... Ts> MAKESHIFT_NODISCARD constexpr bool operator !=(const unit_variant<Ts...>& v, const unit_variant<Ts...>& w) noexcept { return v.index() != w.index(); }
template <typename... Ts> MAKESHIFT_NODISCARD constexpr bool operator > (const unit_variant<Ts...>& v, const unit_variant<Ts...>& w) noexcept { return v.index() <  w.index(); }
template <typename... Ts> MAKESHIFT_NODISCARD constexpr bool operator < (const unit_variant<Ts...>& v, const unit_variant<Ts...>& w) noexcept { return v.index() >  w.index(); }
template <typename... Ts> MAKESHIFT_NODISCARD constexpr bool operator <=(const unit_variant<Ts...>& v, const unit_variant<Ts...>& w) noexcept { return v.index() <= w.index(); }
template <typename... Ts> MAKESHIFT_NODISCARD constexpr bool operator >=(const unit_variant<Ts...>& v, const unit_variant<Ts...>& w) noexcept { return v.index() >= w.index(); }


template <typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr bool holds_alternative(const unit_variant<Ts...>& v) noexcept
{
    return v.index() == index_of_type_v<T, Ts...>;
}


    //ᅟ
    // Returns the `I`-th alternative in the unit variant.
    //
template <std::size_t I, typename... Ts>
    MAKESHIFT_NODISCARD constexpr nth_type_t<I, Ts...> get(const unit_variant<Ts...>& v)
{
    static_assert(I < sizeof...(Ts), "variant index out of range");
    Expects(v.index() == I);
    return { };
}

    //ᅟ
    // Returns the unit variant alternative of type `T`.
    //
template <typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr T get(const unit_variant<Ts...>& v)
{
    static_assert(try_index_of_type_v<T, Ts...> != std::size_t(-1), "type T does not appear in type sequence");
    Expects((v.index() == try_index_of_type_v<T, Ts...>));
    return { };
}


} // namespace detail

} // namespace makeshift


#ifdef MAKESHIFT_CXX17
namespace std
{
#else // MAKESHIFT_CXX17
namespace makeshift
{
#endif // MAKESHIFT_CXX17


    // Specialize `variant_size<>` and `variant_alternative<>` for `unit_variant<>`.
template <typename... Ts> struct variant_size<makeshift::detail::unit_variant<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> struct variant_alternative<I, makeshift::detail::unit_variant<Ts...>> : public makeshift::detail::nth_type_<I, Ts...> { };


#ifdef MAKESHIFT_CXX17
} // namespace std
#else // MAKESHIFT_CXX17
} // namespace makeshift
#endif // MAKESHIFT_CXX17


#endif // INCLUDED_MAKESHIFT_DETAIL_UNIT_VARIANT_HPP_
