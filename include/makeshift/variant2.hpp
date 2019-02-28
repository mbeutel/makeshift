
#ifndef INCLUDED_MAKESHIFT_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_VARIANT2_HPP_


#include <cstddef> // for size_t
#include <array>
#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>
#include <numeric>     // for iota()
#include <algorithm>   // for sort()

#include <makeshift/reflect2.hpp> // for values_of()
#include <makeshift/tuple2.hpp>   // for tuple_reduce(), array_transform2()
#include <makeshift/compound.hpp> // for compound_hash<>, compound_equal_to<>
#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/workaround.hpp>


namespace makeshift
{

namespace detail
{


template <typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2(const T (&array)[N])
{
    return array_transform2<N>([&](auto i) { return array[decltype(i)::value]; }, tuple_index);
}
template <typename T, std::size_t N>
    constexpr std::array<std::tuple<std::remove_cv_t<T>>, N> to_tuple_array(const T (&array)[N])
{
    return array_transform2<N>([&](auto i) { return array[decltype(i)::value]; }, tuple_index);
}

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
        return std::make_tuple(with_values_impl1<TIs>(std::make_index_sequence<N>{ }, vals)...);
    }

public:
    constexpr members_t(Ts C::*... _members) noexcept
        : members_{ _members... }
    {
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr member_values_t<N, C, Ts...> operator =(std::tuple<Ts...> (&&vals)[N]) const &&
    {
        return { members_, with_values_impl0(std::index_sequence_for<Ts...>{ }, vals) };
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
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr member_values_t<N, C, T> operator =(T (&&vals)[N]) const &&
    {
        return { member_, std::make_tuple(to_array2(vals)) };
    }
    constexpr std::tuple<T C::*> members(void) const noexcept { return member_; }
};

template <typename C, typename T>
    constexpr auto _default_values(const members_t<C, T>& member)
{
    auto values = values_of(type_v<T>);
    constexpr std::size_t n = std::tuple_size<std::decay_t<decltype(values)>>::value;
    return member_values_t<n, C, T>{ member.members(), std::make_tuple(values) };
}

template <std::size_t N, typename C>
    struct values_t
{
    std::array<C, N> values;
};

template <typename C, bool IsClass>
    class object0_t;
template <typename C>
    class object0_t<C, false>
{
};
template <typename C>
    class object0_t<C, true> : public object0_t<C, false>
{
public:
    template <typename... Ts>
        MAKESHIFT_NODISCARD constexpr makeshift::detail::members_t<C, Ts...> operator ()(Ts C::*... members)
    {
        // TODO: we should raise an assertion if a member appears more than once
        return { members... };
    }
};
template <typename C>
    class object_t : public object0_t<C, std::is_class<C>::value>
{
public:
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr values_t<N, C> operator =(C (&&vals)[N]) const
    {
        return { to_array2(vals) };
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
    constexpr value_product_t<C, member_values_t<N, C, Ts...>> _to_value_product(const member_values_t<N, C, Ts...>& values)
{
    return { { values } };
}
template <typename C, typename T>
    constexpr auto _to_value_product(const members_t<C, T>& member)
{
    return _to_value_product(_default_values(member));
}
template <typename C, typename... FactorsT>
    constexpr value_product_t<C, FactorsT...> _to_value_product(const value_product_t<C, FactorsT...>& product)
{
    return product;
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
    return _to_value_product(lhs) * rhs;
}
template <std::size_t N, typename C, typename... Ts, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, FactorsT..., member_values_t<N, C, Ts...>> operator *(const value_product_t<C, FactorsT...>& lhs, const member_values_t<N, C, Ts...>& rhs)
{
    return lhs * _to_value_product(rhs);
}
template <typename C, typename T, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T>& lhs, const value_product_t<C, FactorsT...>& rhs)
{
    return _to_value_product(lhs) * rhs;
}
template <typename C, typename T, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr auto operator *(const value_product_t<C, FactorsT...>& lhs, const members_t<C, T>& rhs)
{
    return lhs * _to_value_product(rhs);
}
template <std::size_t N1, std::size_t N2, typename C, typename... T1s, typename... T2s>
    MAKESHIFT_NODISCARD constexpr auto operator *(const member_values_t<N1, C, T1s...>& lhs, const member_values_t<N2, C, T2s...>& rhs)
{
    return _to_value_product(lhs) * _to_value_product(rhs);
}
template <std::size_t N, typename C, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T>& lhs, const member_values_t<N, C, Ts...>& rhs)
{
    return _to_value_product(lhs) * _to_value_product(rhs);
}
template <std::size_t N, typename C, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto operator *(const member_values_t<N, C, Ts...>& lhs, const members_t<C, T>& rhs)
{
    return _to_value_product(lhs) * _to_value_product(rhs);
}
template <typename C, typename T1, typename T2>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T1>& lhs, const members_t<C, T2>& rhs)
{
    return _to_value_product(lhs) * _to_value_product(rhs);
}

// TODO: we could also implement operator |(), but it is a lot of tedious work, and YAGNI

template <std::size_t... Is, typename C, typename... FactorsT>
    constexpr auto _members_impl(std::index_sequence<Is...>, const value_product_t<C, FactorsT...>& product) noexcept
{
    return std::tuple_cat(std::get<Is>(product.factors()).members()...);
}
template <typename C, typename... FactorsT>
    constexpr auto _members(const value_product_t<C, FactorsT...>& product) noexcept
{
    return _members_impl(std::index_sequence_for<FactorsT...>{ }, product);
}

template <std::size_t N>
    constexpr std::array<std::size_t, N> _shape_to_strides(const std::array<std::size_t, N>& shape) noexcept
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
    constexpr void _apply_value_impl(std::index_sequence<Is...>, C& result, const member_values_t<N, C, Ts...>& memberValues, std::size_t i)
{
    ((result.*(std::get<Is>(memberValues.members())) = std::get<Is>(memberValues.values())[i]), ...);
}
template <std::size_t N, typename C, typename... Ts>
    constexpr void _apply_value(C& result, const member_values_t<N, C, Ts...>& memberValues, std::size_t i)
{
    _apply_value_impl(std::index_sequence_for<Ts...>{ }, result, memberValues, i);
}
template <std::size_t... Is, typename C, typename... FactorsT>
    constexpr void _apply_values_impl(std::index_sequence<Is...>,
        C& result, const value_product_t<C, FactorsT...>& product,
        const std::array<std::size_t, sizeof...(FactorsT)>& strides,
        std::size_t i)
{
    (_apply_value(result, std::get<Is>(product.factors()), (i / strides[Is]) % FactorsT::num_values), ...);
}
template <typename C, typename... FactorsT>
    constexpr void _apply_values(
        C& result, const value_product_t<C, FactorsT...>& product,
        const std::array<std::size_t, sizeof...(FactorsT)>& strides,
        std::size_t i)
{
    _apply_values_impl(std::index_sequence_for<FactorsT...>{ }, result, product, strides, i);
}

template <typename C, typename... FactorsT>
    constexpr auto _values_in(const value_product_t<C, FactorsT...>& product) noexcept
{
    auto strides = _shape_to_strides(std::array{ FactorsT::num_values... });
    constexpr std::size_t numValues = cmul<std::size_t>(FactorsT::num_values...);
    return array_transform2<numValues>(
        [&](std::size_t i)
        {
            auto result = C{ };
            _apply_values(result, product, strides, i);
            return result;
        },
        tuple_index);
}


} // namespace detail


inline namespace types
{





} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_VARIANT2_HPP_
