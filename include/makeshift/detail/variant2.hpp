
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT2_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for size_t
#include <variant>
#include <optional>
#include <utility>     // for move(), forward<>(), integer_sequence<>
#include <type_traits> // for decay<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/constexpr.hpp> // for retrieve()
#include <makeshift/compound.hpp>  // for compound_hash<>, compound_equal_to<>
#include <makeshift/reflect2.hpp>  // for values_of()
#include <makeshift/tuple2.hpp>    // for array_transform2()
#include <makeshift/version.hpp>   // for MAKESHIFT_NODISCARD

#include <makeshift/detail/workaround.hpp> // for cmul<>()


namespace makeshift
{

namespace detail
{


template <std::size_t N, typename C, typename... Ts>
    class member_values_t
{
private:
    std::tuple<Ts C::*...> members_;
    std::tuple<std::array<Ts, N>...> values_;

public:
    constexpr member_values_t(std::tuple<Ts C::*...> _members, std::tuple<std::array<Ts, N>...> _values)
        : members_(std::move(_members)), values_(std::move(_values))
    {
    }
    constexpr const std::tuple<Ts C::*...>& members(void) const noexcept { return members_; }
    constexpr const std::tuple<std::array<Ts, N>...>& values(void) const noexcept { return values_; }
    static constexpr std::size_t num_values = N;
};

template <typename C, typename... Ts>
    class members_t
{
private:
    std::tuple<Ts C::*...> members_;

    template <std::size_t TI, std::size_t... Is>
        constexpr static std::array<std::tuple_element_t<TI, std::tuple<Ts...>>, sizeof...(Is)> with_values_impl1(std::index_sequence<Is...>, const std::tuple<Ts...> (&vals)[sizeof...(Is)])
    {
        return { std::get<TI>(vals[Is])... };
    }
    template <std::size_t... TIs, std::size_t N>
        constexpr static std::tuple<std::array<Ts, N>...> with_values_impl0(std::index_sequence<TIs...>, const std::tuple<Ts...> (&vals)[N])
    {
        return std::make_tuple(members_t::with_values_impl1<TIs>(std::make_index_sequence<N>{ }, vals)...);
    }

public:
    constexpr members_t(Ts C::*... _members) noexcept
        : members_{ _members... }
    {
    }
    MAKESHIFT_NODISCARD constexpr member_values_t<0, C, Ts...> operator =(no_values_tag) const
    {
        return { members_, { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr member_values_t<N, C, Ts...> operator =(std::tuple<Ts...> (&&vals)[N]) const &&
    {
        return { members_, members_t::with_values_impl0(std::index_sequence_for<Ts...>{ }, vals) };
    }
    constexpr std::tuple<Ts C::*...> members(void) const noexcept { return members_; }
};
template <typename C, typename T>
    class members_t<C, T>
{
private:
    std::tuple<T C::*> member_;

public:
    constexpr members_t(T C::* _member) noexcept
        : member_{ _member }
    {
    }
    MAKESHIFT_NODISCARD constexpr member_values_t<0, C, T> operator =(no_values_tag) const
    {
        return { member_, { } };
    }
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr member_values_t<N, C, T> operator =(T (&&vals)[N]) const &&
    {
        return { member_, std::make_tuple(makeshift::detail::to_array2(vals)) };
    }
    constexpr std::tuple<T C::*> members(void) const noexcept { return member_; }
};

template <typename C, typename T>
    constexpr auto default_values(const members_t<C, T>& member)
{
    auto lvalues = makeshift::values_of(type_v<T>);
    constexpr std::size_t n = std::tuple_size<std::decay_t<decltype(lvalues)>>::value;
    return member_values_t<n, C, T>{ member.members(), std::make_tuple(lvalues) };
}

struct member_values_initializer_t
{
    template <typename C, typename... Ts>
        MAKESHIFT_NODISCARD constexpr makeshift::detail::members_t<C, Ts...> operator ()(Ts C::*... members) const
    {
        // TODO: we should raise an assertion if a member appears more than once
        return { members... };
    }
};

template <typename C, typename... FactorsT>
    class value_product_t
{
private:
    std::tuple<FactorsT...> factors_;

public:
    constexpr value_product_t(const std::tuple<FactorsT...>& _factors)
        : factors_(_factors)
    {
    }
    constexpr const std::tuple<FactorsT...>& factors(void) const noexcept { return factors_; }
};

template <typename C, std::size_t N, typename... Ts>
    constexpr value_product_t<C, member_values_t<N, C, Ts...>> to_value_product(const member_values_t<N, C, Ts...>& values)
{
    return { { values } };
}
template <typename C, typename T>
    constexpr auto to_value_product(const members_t<C, T>& member)
{
    return makeshift::detail::to_value_product(makeshift::detail::default_values(member));
}

template <typename C, typename... LFs, typename... RFs>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, LFs..., RFs...> operator *(const value_product_t<C, LFs...>& lhs, const value_product_t<C, RFs...>& rhs)
{
    // TODO: we should raise an assertion if a member appears more than once
    return { std::tuple_cat(lhs.factors(), rhs.factors()) };
}
template <std::size_t N, typename C, typename... Ts, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, member_values_t<N, C, Ts...>, FactorsT...> operator *(const member_values_t<N, C, Ts...>& lhs, const value_product_t<C, FactorsT...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * rhs;
}
template <std::size_t N, typename C, typename... Ts, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, FactorsT..., member_values_t<N, C, Ts...>> operator *(const value_product_t<C, FactorsT...>& lhs, const member_values_t<N, C, Ts...>& rhs)
{
    return lhs * makeshift::detail::to_value_product(rhs);
}
template <typename C, typename T, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T>& lhs, const value_product_t<C, FactorsT...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * rhs;
}
template <typename C, typename T, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr auto operator *(const value_product_t<C, FactorsT...>& lhs, const members_t<C, T>& rhs)
{
    return lhs * makeshift::detail::to_value_product(rhs);
}
template <std::size_t N1, std::size_t N2, typename C, typename... T1s, typename... T2s>
    MAKESHIFT_NODISCARD constexpr auto operator *(const member_values_t<N1, C, T1s...>& lhs, const member_values_t<N2, C, T2s...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}
template <std::size_t N, typename C, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T>& lhs, const member_values_t<N, C, Ts...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}
template <std::size_t N, typename C, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto operator *(const member_values_t<N, C, Ts...>& lhs, const members_t<C, T>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}
template <typename C, typename T1, typename T2>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T1>& lhs, const members_t<C, T2>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}

// TODO: we could also implement operator |(), but it is a lot of tedious work, and YAGNI

template <typename T, typename C>
    struct member_functor
{
private:
    T C::* member_;

public:
    constexpr member_functor(T C::* _member) noexcept : member_(_member) { }
    constexpr const T& operator ()(const C& obj) noexcept { return obj.*member_; }
};

struct member_transform_functor
{
    template <typename T, typename C>
        constexpr member_functor<T, C> operator ()(T C::* member) noexcept
    {
        return { member };
    }
};
template <std::size_t... Is, typename C, typename... FactorsT>
    constexpr auto members_impl(std::index_sequence<Is...>, const value_product_t<C, FactorsT...>& product) noexcept
{
    return std::tuple_cat(std::get<Is>(product.factors()).members()...);
}
template <typename C, typename... FactorsT>
    constexpr auto _members(const value_product_t<C, FactorsT...>& product) noexcept
{
    return makeshift::tuple_transform2(member_transform_functor{ }, makeshift::detail::members_impl(std::index_sequence_for<FactorsT...>{ }, product));
}

template <std::size_t N>
    constexpr std::array<std::size_t, N> shape_to_strides(const std::array<std::size_t, N>& shape) noexcept
{
    auto result = std::array<std::size_t, N>{ };
    std::size_t stride = 1;
    for (std::size_t i = 0; i != N; ++i)
    {
        result[i] = stride;
        stride *= shape[i];
    }
    return result;
}


template <std::size_t... Is, std::size_t N, typename C, typename... Ts>
    constexpr void apply_value_impl(std::index_sequence<Is...>, C& result, const member_values_t<N, C, Ts...>& memberValues, std::size_t i)
{
    ((result.*(std::get<Is>(memberValues.members())) = std::get<Is>(memberValues.values())[i]), ...);
}
template <std::size_t N, typename C, typename... Ts>
    constexpr void apply_value(C& result, const member_values_t<N, C, Ts...>& memberValues, std::size_t i)
{
    makeshift::detail::apply_value_impl(std::index_sequence_for<Ts...>{ }, result, memberValues, i);
}
template <std::size_t... Is, typename C, typename... FactorsT>
    constexpr void apply_values_impl(std::index_sequence<Is...>,
        C& result, const value_product_t<C, FactorsT...>& product,
        const std::array<std::size_t, sizeof...(FactorsT)>& strides,
        std::size_t i)
{
    (makeshift::detail::apply_value(result, std::get<Is>(product.factors()), (i / strides[Is]) % FactorsT::num_values), ...);
}
template <typename C, typename... FactorsT>
    constexpr void apply_values(
        C& result, const value_product_t<C, FactorsT...>& product,
        const std::array<std::size_t, sizeof...(FactorsT)>& strides,
        std::size_t i)
{
    makeshift::detail::apply_values_impl(std::index_sequence_for<FactorsT...>{ }, result, product, strides, i);
}

template <typename C, typename... FactorsT>
    struct make_value_functor
{
    const value_product_t<C, FactorsT...>& product;
    const std::array<std::size_t, sizeof...(FactorsT)>& strides;

    constexpr C operator ()(std::size_t i) const
    {
        auto result = C{ };
        makeshift::detail::apply_values(result, product, strides, i);
        return result;
    }
};

template <typename C, typename... FactorsT>
    constexpr auto to_array(const value_product_t<C, FactorsT...>& product) noexcept
{
    std::array<std::size_t, sizeof...(FactorsT)> strides = makeshift::detail::shape_to_strides(std::array{ FactorsT::num_values... });
    constexpr std::size_t numValues = cmul<std::size_t>(FactorsT::num_values...);
    return makeshift::array_transform2<numValues>(
        make_value_functor<C, FactorsT...>{ product, strides },
        tuple_index);
}
template <std::size_t N, typename C, typename... Ts>
    constexpr auto to_array(const member_values_t<N, C, Ts...>& memberValues) noexcept
{
    return to_array(makeshift::to_value_product(memberValues));
}


template <typename T, typename R, std::size_t I>
    struct value_functor
{
    constexpr T operator ()(void) const
    {
        return makeshift::retrieve<R>()[I];
    }
};

template <typename T, typename R, typename Is>
    struct expand_type_;
template <typename T, typename R, std::size_t... Is>
    struct expand_type_<T, R, std::index_sequence<Is...>>
{
    using type = std::variant<value_functor<T, R, Is>...>;
};
template <typename ExpandTypeT>
    using expand_type_factory = ExpandTypeT (*)(void);

template <typename ExpandTypeT>
    struct expand_type_factory_functor
{
    template <std::size_t I>
        static constexpr auto invoke(void)
    {
        return ExpandTypeT{ std::in_place_index<I> };
    }
    template <std::size_t I>
        constexpr expand_type_factory<ExpandTypeT> operator ()(std::integral_constant<std::size_t, I>) const
    {
        return invoke<I>;
    }
};

template <typename ExpandTypeT, std::size_t N>
    constexpr auto make_expand_factories(void)
{
    return makeshift::array_transform2<N>(
        expand_type_factory_functor<ExpandTypeT>{ },
        tuple_index);
}
template <typename T, typename R, typename HashT, typename EqualToT>
     constexpr auto value_to_variant(const T& value, R valueArrayR, HashT&& /*hash*/, EqualToT&& equal)
{
    constexpr auto lvalues = makeshift::retrieve(valueArrayR);
    constexpr std::size_t numValues = std::tuple_size<decltype(lvalues)>::value;
    using ExpandType = typename expand_type_<T, R, std::make_index_sequence<numValues>>::type;

    constexpr auto factories = make_expand_factories<ExpandType, numValues>();

    for (std::size_t i = 0; i != numValues; ++i)
        if (equal(value, lvalues[i]))
            return std::optional<ExpandType>{ ExpandType{ factories[i]() } };
    return std::optional<ExpandType>{ std::nullopt };
}

struct value_array_functor
{
    template <typename T>
        constexpr auto operator ()(T&& arg) const
    {
        return std::forward<T>(arg).values;
    }
};

struct to_array_functor
{
    template <typename T>
        constexpr auto operator ()(T&& arg) const
    {
        return to_array(std::forward<T>(arg));
    }
};

template <typename T>
    struct values_of_retriever
{
    constexpr auto operator ()(void) const
    {
        return values_initializer_t<T>{ } = values_of(type_v<T>);
    }
};

template <typename T, typename R, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2(const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    auto valueArrayR = makeshift::retriever_transform(value_array_functor{ }, valuesR);
    
        // If hash and equal are empty, we can build a hash table at compile time.
        // We won't bother in the case where they are not because we need to traverse all elements at runtime anyway.
        // (TODO: implement)
        // (TODO: is there an acceptable way of checking for constexpr-ness?)

    return makeshift::detail::value_to_variant(value, valueArrayR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename T, typename R, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2_compound(const T& value, R productR, HashT&& hash, EqualToT&& equal)
{
    constexpr auto product = makeshift::retrieve(productR);
    constexpr auto valueArrayR = makeshift::retriever_transform(to_array_functor{ }, productR);

        // If hash and equal are empty, we can build a hash table at compile time.
        // We won't bother in the case where they are not because we need to traverse all elements at runtime anyway.
        // (TODO: implement)
        // (TODO: is there an acceptable way of checking for constexpr-ness?)

    auto memberAccessor = [members = _members(product)](type<T>) { return members; };
    auto compoundHash = compound_hash<decltype(memberAccessor), std::decay_t<HashT>>{ std::forward<HashT>(hash), memberAccessor };
    auto compoundEqual = compound_equal_to<decltype(memberAccessor), std::decay_t<EqualToT>>{ std::forward<EqualToT>(equal), memberAccessor };

    return makeshift::detail::value_to_variant(value, valueArrayR, std::move(compoundHash), std::move(compoundEqual));
}

template <typename T, typename... FactorsT, typename R, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(type<value_product_t<T, FactorsT...>>, const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2_compound(value, valuesR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename T, std::size_t N, typename... Ts, typename R, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(type<member_values_t<N, T, Ts...>>, const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2_compound(value, valuesR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename T, std::size_t N, typename R, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(type<values_t<T, N>>, const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2(value, valuesR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename T, typename R, typename HashT, typename EqualToT>
    constexpr auto expand2_impl0(const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl1(type_v<retrieved_t<R>>, value, valuesR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT2_HPP_
