
#ifndef INCLUDED_MAKESHIFT_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_VARIANT_HPP_


#include <cstddef>     // for size_t
#include <variant>     // for variant_size<>, variant_alternative<>
#include <exception>   // for terminate()
#include <utility>     // for move(), forward<>(), get<>
#include <type_traits> // for enable_if<>, decay<>, integral_constant<>, index_sequence<>

#include <makeshift/type_traits.hpp> // for can_instantiate<>
#include <makeshift/tuple.hpp>       // for is_tuple_like<>
#include <makeshift/variant2.hpp>    // for is_variant_like<>


namespace makeshift
{

inline namespace types
{

template <typename T>
    struct unknown_value : std::monostate
{
    using value_type = T;

    T value;

    constexpr unknown_value(T _value) : value(_value) { }
};
template <typename T>
    unknown_value(T&&) -> unknown_value<std::decay_t<T>>;


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

template <typename R, typename F, typename VariantT>
    [[noreturn]] R variant_apply(std::index_sequence<>, F&&, VariantT&&)
{
    std::terminate(); // cannot happen, we just need to silence the compiler about not being able to formally return a value
}
template <typename R, std::size_t I0, std::size_t... Is, typename F, typename VariantT>
    constexpr R variant_apply(std::index_sequence<I0, Is...>, F&& func, VariantT&& variant)
{
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    if (variant.index() == I0)
        return { func(get<I0>(std::forward<VariantT>(variant))) };
    else
        return variant_apply<R>(std::index_sequence<Is...>{ }, std::forward<F>(func), std::forward<VariantT>(variant));
}

template <typename R, std::size_t I, typename ExcFuncT, typename VariantT>
    [[noreturn]] constexpr R variant_apply_or_throw(std::index_sequence<>, ExcFuncT&&, VariantT&&)
{
    std::terminate(); // cannot happen, we just need to silence the compiler about not being able to formally return a value
}
template <typename R, std::size_t I, std::size_t I0, std::size_t... Is, typename ExcFuncT, typename VariantT>
    constexpr R variant_apply_or_throw(std::index_sequence<I0, Is...>, ExcFuncT&& excFunc, VariantT&& variant)
{
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    static_assert(I <= I0); // sequence must be ordered
    if constexpr (I == I0)
    {
        if (variant.index() == I)
            throw excFunc(get<I>(std::forward<VariantT>(variant)));
        else
            return variant_apply_or_throw<R, I + 1>(std::index_sequence<Is...>{ }, std::forward<ExcFuncT>(excFunc), std::forward<VariantT>(variant));
    }
    else
    {
        if (variant.index() == I)
            return { get<I>(std::forward<VariantT>(variant)) };
        else
            return variant_apply_or_throw<R, I + 1>(std::index_sequence<I0, Is...>{ }, std::forward<ExcFuncT>(excFunc), std::forward<VariantT>(variant));
    }
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
        return variant_apply<ResultVariant>(std::index_sequence<Is...>{ }, func, std::forward<VariantT>(variant));
    }
    
public:
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr auto operator ()(VariantT&& v) const
    {
        return invoke(*this, std::forward<VariantT>(v), std::make_index_sequence<std::variant_size<std::decay_t<VariantT>>::value>{ });
    }
};


template <typename T, typename F>
    struct variant_map_to_t : F, variant_stream_base<variant_map_to_t<T, F>>
{
    constexpr variant_map_to_t(F func) : F(std::move(func)) { }

public:
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr T operator ()(VariantT&& variant) const
    {
        static_assert(std::variant_size<std::decay_t<VariantT>>::value > 0);
        return std::visit(static_cast<const F&>(*this), std::forward<VariantT>(variant));
    }
};


template <typename SelectedIs, typename Is, typename VariantT, typename PredT> struct select_variant_indices_;
template <std::size_t... SelectedIs, std::size_t NextI, std::size_t... Is, typename VariantT, typename PredT>
    struct select_variant_indices_<std::index_sequence<SelectedIs...>, std::index_sequence<NextI, Is...>, VariantT, PredT>
        : select_variant_indices_<typename select_next_index_<std::index_sequence<SelectedIs...>, NextI, PredT::template type<std::variant_alternative_t<NextI, VariantT>>::value>::type, std::index_sequence<Is...>, VariantT, PredT>
{
};
template <std::size_t... SelectedIs, typename VariantT, typename PredT>
    struct select_variant_indices_<std::index_sequence<SelectedIs...>, std::index_sequence<>, VariantT, PredT>
{
    using type = std::index_sequence<SelectedIs...>;
};
template <typename VariantT, typename PredT>
    using select_variant_indices_t = typename select_variant_indices_<std::index_sequence<>, std::make_index_sequence<std::variant_size<VariantT>::value>, VariantT, PredT>::type;


template <typename PredT>
    struct variant_filter_t : variant_stream_base<variant_filter_t<PredT>>
{
private:
    template <typename VariantT, std::size_t... Is>
        static constexpr auto invoke(VariantT&& variant, std::index_sequence<Is...>)
            -> rebind_template_t<std::decay_t<VariantT>, std::variant_alternative_t<Is, std::decay_t<VariantT>>...>
    {
        using ResultVariant = rebind_template_t<std::decay_t<VariantT>, std::variant_alternative_t<Is, std::decay_t<VariantT>>...>;
        return variant_apply<ResultVariant>(std::index_sequence<Is...>{ }, identity_transform{ }, std::forward<VariantT>(variant));
    }
 
public:
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr auto operator ()(VariantT&& variant) const
    {
        return invoke(std::forward<VariantT>(variant), select_variant_indices_t<std::decay_t<VariantT>, PredT>{ });
    }
};

template <typename PredT, typename ExcFuncT>
    struct variant_filter_or_throw_t : ExcFuncT, variant_stream_base<variant_filter_or_throw_t<PredT, ExcFuncT>>
{
    constexpr variant_filter_or_throw_t(ExcFuncT func) : ExcFuncT(std::move(func)) { }

private:
    template <typename VariantT, std::size_t... Is>
        constexpr auto invoke(VariantT&& variant, std::index_sequence<Is...>) const
            -> rebind_template_t<std::decay_t<VariantT>, std::variant_alternative_t<Is, std::decay_t<VariantT>>...>
    {
        using ResultVariant = rebind_template_t<std::decay_t<VariantT>, std::variant_alternative_t<Is, std::decay_t<VariantT>>...>;
        return variant_apply_or_throw<ResultVariant, 0>(std::index_sequence<Is...>{ }, static_cast<const ExcFuncT&>(*this), std::forward<VariantT>(variant));
    }
 
public:
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr auto operator ()(VariantT&& variant) const
    {
        return invoke(std::forward<VariantT>(variant), select_variant_indices_t<std::decay_t<VariantT>, PredT>{ });
    }
};

template <typename R, typename... Vs> struct flat_variant_0_;
template <template <typename...> class V, typename... Rs> struct flat_variant_0_<V<Rs...>> { using type = V<Rs...>; };
template <template <typename...> class V, typename... Rs, typename... Ts, typename... Vs> struct flat_variant_0_<V<Rs...>, V<Ts...>, Vs...> : flat_variant_0_<V<Rs..., Ts...>, Vs...> { };
template <typename V> struct flat_variant;
template <template <typename...> class V, typename... Vs> struct flat_variant<V<Vs...>> : flat_variant_0_<V<>, Vs...> { };
template <typename V> using flat_variant_t = typename flat_variant<V>::type;

template <typename R, std::size_t Base, std::size_t J, typename ElemVariantT>
    constexpr R variant_cat_1(constant<Base>, constant<J>, ElemVariantT&& elemVariant)
{
    if constexpr (J == std::variant_size<std::decay_t<ElemVariantT>>::value)
        std::terminate(); // cannot happen, we just need to silence the compiler about not being able to formally return a value
    else
    {
        if (elemVariant.index() == J)
            return R{ std::in_place_index_t<Base + J>{ }, std::get<J>(std::forward<ElemVariantT>(elemVariant)) };
        else
            return variant_cat_1<R>(constant<Base>{ }, constant<J + 1>{ }, std::forward<ElemVariantT>(elemVariant));
    }
}
template <typename R, std::size_t Base, std::size_t I, typename VariantT>
    constexpr R variant_cat_0(constant<Base>, constant<I>, VariantT&& variant)
{
    if constexpr (I == std::variant_size<std::decay_t<VariantT>>::value)
        std::terminate(); // cannot happen, we just need to silence the compiler about not being able to formally return a value
    else
    {
        if (variant.index() == I)
            return variant_cat_1<R>(constant<Base + I>{ }, constant<0>{ }, std::get<I>(std::forward<VariantT>(variant)));
        else
        {
            constexpr std::size_t elemVariantSize = std::variant_size<std::variant_alternative<I, std::decay_t<VariantT>>>::value;
            return variant_cat_0<R>(constant<Base + elemVariantSize>{ }, constant<I + 1>{ }, std::forward<VariantT>(variant));
        }
    }
}

struct variant_cat_t : variant_stream_base<variant_cat_t>
{
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
        constexpr auto operator ()(VariantT&& variant) const
    {
        using ResultVariant = flat_variant_t<std::decay_t<VariantT>>;
        return variant_cat_0<ResultVariant>(constant<0>{ }, constant<0>{ }, std::forward<VariantT>(variant));
    }
};


template <typename R, typename T, typename EqualToT, typename TupleT>
    [[noreturn]] R expand_impl(std::true_type /* fail */, std::index_sequence<>, T&&, EqualToT&&, TupleT&&)
{
    std::terminate(); // we end up here if the value was not found, which is a programming error
}
template <typename R, typename T, typename EqualToT, typename TupleT>
    constexpr R expand_impl(std::false_type /* fail */, std::index_sequence<>, T&& value, EqualToT&&, TupleT&&)
{
    return unknown_value{ std::forward<T>(value) };
}
template <typename R, typename RaiseT, std::size_t I0, std::size_t... Is, typename T, typename EqualToT, typename TupleT>
    constexpr R expand_impl(const RaiseT&, std::index_sequence<I0, Is...>, T&& value, EqualToT&& equalToFunc, TupleT&& tuple)
{
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    if (equalToFunc(value, get<I0>(tuple)))
        return { get<I0>(std::forward<TupleT>(tuple)) };
    else
        return expand_impl<R>(RaiseT{ }, std::index_sequence<Is...>{ }, std::forward<T>(value), std::forward<EqualToT>(equalToFunc), std::forward<TupleT>(tuple));
}


template <template <typename...> class VariantT, typename T, typename TupleT> struct apply_variant_type;
template <template <typename...> class VariantT, typename T, template <typename...> class TupleT, typename... Ts> struct apply_variant_type<VariantT, T, TupleT<Ts...>> { using type = VariantT<unknown_value<T>, Ts...>; };
template <template <typename...> class VariantT, typename T, typename TupleT> using apply_variant_type_t = typename apply_variant_type<VariantT, T, TupleT>::type;


} // namespace detail


inline namespace types
{


    //ᅟ
    // Returns a functor that maps a variant to a new variant which can hold only the alternatives for which the given type predicate holds.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto signedNumber = number
    //ᅟ        | variant_filter(trait_v<std::is_signed>); // returns variant<int>{ 3 }
    //
template <typename PredT>
    constexpr makeshift::detail::variant_filter_t<std::decay_t<PredT>>
    variant_filter(PredT&&) noexcept
{
    return { };
}


    //ᅟ
    // Maps a variant to a new variant which can hold only the alternatives for which the given type predicate holds.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto signedNumber = variant_filter(number, trait_v<std::is_signed>); // returns variant<int>{ 3 }
    //
template <typename PredT, typename VariantT,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr auto
    variant_filter(VariantT&& variant, PredT&&) noexcept
{
    return variant_filter(PredT{ })(std::forward<VariantT>(variant));
}


    //ᅟ
    // Returns a functor that maps a variant to a new variant which can hold only the alternatives for which the given type predicate holds.
    // If the variant holds an alternative for which the given type predicate does not hold, `excFunc` is called with the alternative held,
    // and its return value is thrown.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto signedNumber = number
    //ᅟ        | variant_filter_or_throw(trait_v<std::is_signed>,
    //ᅟ              [](auto v) { return std::runtime_error("value is not a signed integer"); }); // returns variant<int>{ 3 }
    //
template <typename PredT, typename ExcFuncT>
    constexpr makeshift::detail::variant_filter_or_throw_t<std::decay_t<PredT>, std::decay_t<ExcFuncT>>
    variant_filter_or_throw(PredT&&, ExcFuncT&& excFunc) noexcept
{
    return { std::forward<ExcFuncT>(excFunc) };
}


    //ᅟ
    // Returns a functor that maps a variant to a new variant which can hold only the alternatives for which the given type predicate holds.
    // If the variant holds an alternative for which the given type predicate does not hold, `excFunc` is called with the alternative held,
    // and its return value is thrown.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto signedNumber = variant_filter_or_throw(number, trait_v<std::is_signed>,
    //ᅟ        [](auto v) { return std::runtime_error("value is not a signed integer"); }); // returns variant<int>{ 3 }
    //
template <typename PredT, typename VariantT, typename ExcFuncT,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr auto
    variant_filter_or_throw(VariantT&& variant, PredT&&, ExcFuncT&& excFunc) noexcept
{
    return variant_filter_or_throw(PredT{ }, std::forward<ExcFuncT>(excFunc))(std::forward<VariantT>(variant));
}


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
    //ᅟ        | variant_map_to<int>([](auto x) { return int(x); }); // returns 3
    //
template <typename T, typename F>
    constexpr makeshift::detail::variant_map_to_t<T, std::decay_t<F>>
    variant_map_to(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a variant and a unary visitor function (i.e. a function with non-variant argument and non-variant return type) and maps the variant to a 
    // non-variant by calling the visitor function with using `std::visit()`.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto numberAsInt = variant_map_to<int>(number, [](auto x) { return int(x); }); // returns 3
    //
template <typename T, typename VariantT, typename F,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr T
    variant_map_to(VariantT&& variant, F&& func)
{
    return variant_map_to<T>(std::forward<F>(func))(std::forward<VariantT>(variant));
}


    //ᅟ
    // Returns a function which maps a variant to a non-variant by converting the alternative held by the variant to the given type.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto numberAsInt = number
    //ᅟ        | variant_to_scalar<int>(); // returns 3
    //
template <typename T>
    constexpr makeshift::detail::variant_map_to_t<T, makeshift::detail::implicit_conversion_transform<T>>
    variant_to_scalar(void)
{
    return { { } };
}


    //ᅟ
    // Maps a variant to a non-variant by converting the alternative held by the variant to the given type.
    //ᅟ
    //ᅟ    auto number = std::variant<int, unsigned>{ 3 };
    //ᅟ    auto numberAsInt = variant_to_scalar<int>(number); // returns 3
    //
template <typename T, typename VariantT,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr T
    variant_to_scalar(VariantT&& variant)
{
    return variant_to_scalar<T>()(std::forward<VariantT>(variant));
}


    //ᅟ
    // Concatenates the variants in a variant of variants.
    //ᅟ
    //ᅟ    auto number = std::variant<std::variant<int, unsigned>, std::variant<float, double>>{ 3 };
    //ᅟ    auto flatVariant = number
    //ᅟ        | variant_cat(); // returns std::variant<int, unsigned, float, double>{ 3 };
    //
constexpr inline makeshift::detail::variant_cat_t
variant_cat(void)
{
    return { };
}


    //ᅟ
    // Concatenates the variants in a variant of variants.
    //ᅟ
    //ᅟ    auto number = std::variant<std::variant<int, unsigned>, std::variant<float, double>>{ 3 };
    //ᅟ    auto flatVariant = variant_cat(number); // returns std::variant<int, unsigned, float, double>{ 3 };
    //
template <typename VariantT,
          typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>>>>
    constexpr auto
    variant_cat(VariantT&& variant)
{
    return variant_cat()(std::forward<VariantT>(variant));
}


    //ᅟ
    // Given a runtime value and a tuple of type-encoded possible values, returns a variant of the type-encoded possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = expand(runtimeBits, std::tuple{ c<16>, c<32>, c<64> }, std::equal_to<int>{ }); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT, typename EqualToT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr typename instantiate<std::variant, std::decay_t<TupleT>>::type
    expand(T&& value, TupleT&& tuple, EqualToT&& equalToFunc)
{
    using R = typename instantiate<std::variant, std::decay_t<TupleT>>::type;
    return makeshift::detail::expand_impl<R>(std::true_type{ }, std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<T>(value), std::forward<EqualToT>(equalToFunc), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Given a runtime value and a tuple of type-encoded possible values, returns a variant of the type-encoded possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = expand(runtimeBits, std::tuple{ c<16>, c<32>, c<64> }); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr typename instantiate<std::variant, std::decay_t<TupleT>>::type
    expand(T&& value, TupleT&& tuple)
{
    using R = typename instantiate<std::variant, std::decay_t<TupleT>>::type;
    return makeshift::detail::expand_impl<R>(std::true_type{ }, std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<T>(value), [](const auto& lhs, const auto& rhs) noexcept { return lhs == rhs; }, std::forward<TupleT>(tuple));
}


    //ᅟ
    // Given a runtime value and a tuple of type-encoded possible values, returns a variant of `unknown_value<>` and the type-encoded possible values.
    // The variant holds `unknown_value<>` if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::tuple{ c<16>, c<32>, c<64> }, std::equal_to<int>{ }); // returns std::variant<unknown_value<int>, constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT, typename EqualToT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr typename makeshift::detail::apply_variant_type<std::variant, std::decay_t<T>, std::decay_t<TupleT>>::type
    try_expand(T&& value, TupleT&& tuple, EqualToT&& equalToFunc)
{
    using R = typename makeshift::detail::apply_variant_type<std::variant, std::decay_t<T>, std::decay_t<TupleT>>::type;
    return makeshift::detail::expand_impl<R>(std::false_type{ }, std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<T>(value), std::forward<EqualToT>(equalToFunc), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Given a runtime value and a tuple of type-encoded possible values, returns a variant of `unknown_value<>` and the type-encoded possible values.
    // The variant holds `unknown_value<>` if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::tuple{ c<16>, c<32>, c<64> }); // returns std::variant<unknown_value<int>, constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr typename makeshift::detail::apply_variant_type<std::variant, std::decay_t<T>, std::decay_t<TupleT>>::type
    try_expand(T&& value, TupleT&& tuple)
{
    using R = typename makeshift::detail::apply_variant_type<std::variant, std::decay_t<T>, std::decay_t<TupleT>>::type;
    return makeshift::detail::expand_impl<R>(std::false_type{ }, std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<T>(value), [](const auto& lhs, const auto& rhs) noexcept { return lhs == rhs; }, std::forward<TupleT>(tuple));
}


} // inline namespace types

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_REFLECT_HPP_
 #include <makeshift/detail/variant-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_


#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_
