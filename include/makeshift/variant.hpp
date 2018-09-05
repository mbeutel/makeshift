﻿
#ifndef INCLUDED_MAKESHIFT_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_VARIANT_HPP_


#include <cstddef>     // for size_t
#include <variant>     // for monostate, variant_size<>, variant_alternative<>
#include <optional>
#include <utility>     // for move(), forward<>(), get<>, in_place_index<>, swap()
#include <stdexcept>   // for invalid_argument
#include <type_traits> // for decay<>, integral_constant<>, index_sequence<>, is_nothrow_default_constructible<>

#include <makeshift/type_traits.hpp> // for can_apply<>

#include <makeshift/detail/workaround.hpp> // for cand()


namespace makeshift
{

namespace detail
{


template <typename T> using is_variant_like_r = std::integral_constant<std::size_t, std::variant_size<T>::value>;

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
        if (_index >= sizeof...(Ts))
            throw std::invalid_argument("index out of range");
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
    // Returns the `I`-th alternative in the type variant. Raises `std::bad_variant_access` if the variant does not currently hold the `I`-th alternative.
    //
template <std::size_t I, typename... Ts>
    constexpr nth_type_t<I, Ts...> get(const type_variant<Ts...>& v)
{
    static_assert(I < sizeof...(Ts), "variant index out of range");
    if (v.index() != I)
        throw std::bad_variant_access();
    return { };
}

    //ᅟ
    // Returns the type variant alternative of type `T`. Raises `std::bad_variant_access` if the variant does not currently hold the alternative of type `T`.
    //
template <typename T, typename... Ts>
    constexpr T get(const type_variant<Ts...>& v)
{
    static_assert(try_index_of_type_v<T, Ts...> != std::size_t(-1), "type T does not appear in type sequence");
    if (v.index() != try_index_of_type_v<T, Ts...>)
        throw std::bad_variant_access();
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


    //ᅟ
    // Determines whether a type has a variant-like interface (i.e. whether `std::variant_size<T>::value` is well-formed).
    //
template <typename T> struct is_variant_like : can_apply<makeshift::detail::is_variant_like_r, T> { };

    //ᅟ
    // Determines whether a type has a variant-like interface (i.e. whether `std::variant_size<T>::value` is well-formed).
    //
template <typename T> constexpr bool is_variant_like_v = is_variant_like<T>::value;


} // inline namespace types


namespace detail
{


template <typename DerivedT>
    struct variant_stream_base
{
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        friend constexpr auto operator |(VariantT&& variant, const DerivedT& self)
    {
        return self(std::forward<VariantT>(variant));
    }
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        friend constexpr auto operator |(VariantT&& variant, DerivedT&& self)
    {
        return std::move(self)(std::forward<VariantT>(variant));
    }
};

template <typename R, std::size_t I, typename F, typename VariantT>
    constexpr R variant_apply(F&& func, VariantT&& variant);
template <typename R, std::size_t I, typename F, typename VariantT>
    constexpr R variant_apply_recur(F&& func, VariantT&& variant)
{
    if (variant.index() == I)
        return { func(get<I>(std::forward<VariantT>(variant))) };
    else
        return variant_apply<R, I + 1>(std::forward<F>(func), std::forward<VariantT>(variant));
}
template <typename R, std::size_t I, typename F, typename VariantT>
    constexpr R variant_apply(F&& func, VariantT&& variant)
{
    (void) func;
    (void) variant;
    if constexpr (I == std::variant_size<std::decay_t<VariantT>>::value)
        throw std::bad_variant_access(); // cannot happen, we just need to silence the compiler about not being able to formally return a value
    else
        return variant_apply_recur<R, I>(std::forward<F>(func), std::forward<VariantT>(variant));
}


template <typename T, typename... Ts> struct rebind_template;
template <template <typename...> class T, typename... Us, typename... Ts> struct rebind_template<T<Us...>, Ts...> { using type = T<Ts...>; };
template <typename T, typename... Ts> using rebind_template_t = typename rebind_template<T, Ts...>::type;


template <typename F>
    struct variant_map_t : F, variant_stream_base<variant_map_t<F>>
{
    constexpr variant_map_t(F func) : F(std::move(func)) { }

private:
    template <typename VariantT, std::size_t... Is>
        static constexpr auto invoke(const F& func, VariantT&& variant, std::index_sequence<Is...>)
            -> rebind_template_t<std::decay_t<VariantT>, decltype(func(std::declval<std::variant_alternative_t<Is, std::decay_t<VariantT>>>()))...>
    {
        using ResultVariant = rebind_template_t<std::decay_t<VariantT>, decltype(func(std::declval<std::variant_alternative_t<Is, std::decay_t<VariantT>>>()))...>;
        return variant_apply<ResultVariant, 0>(func, std::forward<VariantT>(variant));
    }
    
public:
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr auto operator ()(VariantT&& v) const
    {
        return invoke(*this, std::forward<VariantT>(v), std::make_index_sequence<std::variant_size<std::decay_t<VariantT>>::value>{ });
    }
};


template <typename F>
    struct variant_reduce_t : F
{
    constexpr variant_reduce_t(F func) : F(std::move(func)) { }

private:
    template <std::size_t... Is, typename ValT, typename VariantT>
        constexpr auto invoke(std::index_sequence<Is...>, ValT&& initialValue, VariantT&& variant) const
    {
        (void) variant;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        auto wrappedInitialValue = make_accumulator_wrapper(static_cast<const F&>(*this), std::forward<ValT>(initialValue));
        return (std::move(wrappedInitialValue) + ... + get<Is>(std::forward<VariantT>(variant))).get();
    }
    
public:
    template <typename ValT, typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr auto operator ()(VariantT&& variant) const
    {
        const F& func = *this;
        return std::visit(func, std::forward<VariantT>(variant));
    }
};


    /*
        which functions would be worth implementing?

        Sorting: ??
        
        Set operations:
        - Distinct: ?
        - Except: ?
        - Intersect: ?
        - Union: ?

        Quantifiers:
        - All/Any: ?
        - Contains: ?

        Projections:
        - SelectMany: ?

        Partitioning: ?

        Join: ?

        Group: ?

        Generation:
        - DefaultIfEmpty: ?
        - Range: ?
        - Repeat: ?

        Element:
        - ElementAtOrDefault: ?
        - First/Last: ?
        - FirstOrDefault/LastOrDefault: ?
        
        Concatenation: ?

    */


} // namespace detail


inline namespace types
{


    //ᅟ
    // Takes a scalar unary function (i.e. a function with a non-variant arguments and non-variant return type) and returns a function which can be called 
    // with a variant, and whose result will be a variant of the results of the function applied to the variant elements.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto squaredNumber = number
    //ᅟ        | variant_map([](auto x) { return x*x; }); // returns variant<int, unsigned>{ 9 }
    //
template <typename F>
    constexpr makeshift::detail::variant_map_t<std::decay_t<F>>
    variant_map(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a variant and a scalar unary function (i.e. a function with a non-variant argument and non-variant return type) and returns a variant of the
    // results of the function applied to the variant elements.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto squaredNumber = variant_map(number, [](auto x) { return x*x; }); // returns variant<int, unsigned>{ 9 }
    //
template <typename VariantT, typename F,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr auto
    variant_map(VariantT&& variant, F&& func)
{
    return variant_map(std::forward<F>(func))(std::forward<VariantT>(variant));
}


    //ᅟ
    // Takes a unary visitor function (i.e. a function with non-variant argument and non-variant return type) and returns a function which maps a variant
    // to a non-variant by calling the visitor function with `std::visit()`.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto numberAsInt = number
    //ᅟ        | variant_reduce([](auto x) { return int(x); }); // returns 3
    //
template <typename F>
    constexpr makeshift::detail::variant_reduce_t<std::decay_t<F>>
    variant_reduce(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a variant and a unary visitor function (i.e. a function with non-variant argument and non-variant return type) and maps the variant to a 
    // non-variant by calling the visitor function with using `std::visit()`.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto numberAsInt = variant_reduce(number, [](auto x) { return int(x); }); // returns 3
    //
template <typename VariantT, typename F,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr auto
    variant_reduce(VariantT&& variant,F&& func)
{
    return variant_reduce(std::forward<F>(func))(std::forward<VariantT>(variant));
}


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `variant_size<>` and `variant_alternative<>` for `type_variant<>`.
template <typename... Ts> class variant_size<makeshift::type_variant<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class variant_alternative<I, makeshift::type_variant<Ts...>> : public makeshift::detail::nth_type_<I, Ts...> { };


} // namespace std


#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_