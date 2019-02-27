
#ifndef INCLUDED_MAKESHIFT_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_VARIANT2_HPP_


#include <cstddef> // for size_t
#include <array>
#include <tuple>
#include <utility>
#include <type_traits>
#include <functional>

#include <makeshift/reflect2.hpp>     // for values_of()
#include <makeshift/type_traits2.hpp> // for type_sequence2<>
#include <makeshift/tuple2.hpp>       // for tuple_reduce(), array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/workaround.hpp>


namespace makeshift
{

namespace detail
{


template <std::size_t... Is, typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2_impl(std::index_sequence<Is...>, T (&array)[N])
{
    (void) array;
    return { array[Is]... };
}
template <typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2(T (&array)[N])
{
    //return array_transform2<N>([&](auto i) constexpr { return array[decltype(i)::value]; }, tuple_index);
    return to_array2_impl(std::make_index_sequence<N>{ }, array);
}

template <std::size_t N, typename C, typename... Ts>
    class member_values_t
{
private:
    std::tuple<Ts C::*...> members_;
    std::array<std::tuple<Ts...>, N> values_;

public:
    constexpr member_values_t(std::tuple<Ts C::*...> _members, std::array<std::tuple<Ts...>, N> _values)
        : members_(std::move(_members)), values_(std::move(_values))
    {
    }
    constexpr const std::tuple<Ts C::*...>& members(void) const noexcept { return members_; }
    constexpr const std::array<std::tuple<Ts...>, N>& values(void) const noexcept { return values_; }
    static constexpr std::size_t num_values = N;
};

template <typename C, typename... Ts>
    class members_t
{
private:
    std::tuple<Ts C::*...> members_;

public:
    constexpr members_t(Ts C::*... _members) noexcept
        : members_{ _members... }
    {
    }
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr member_values_t<N, C, Ts...> operator =(std::tuple<Ts...> (&&vals)[N]) const &&
    {
        return { members_, to_array2(vals) };
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
        return { member_, array_transform2([](auto val) constexpr { return std::make_tuple(val); }, to_array2(vals)) };
    }
    constexpr std::tuple<T C::*> members(void) const noexcept { return member_; }
};

template <typename C, typename T>
    constexpr auto _default_values(const members_t<C, T>& member)
{
    auto values = values_of(type_v<T>);
    auto valueTuples = array_transform2([](auto value) { return std::make_tuple(value); }, values);
    constexpr std::size_t n = std::tuple_size<std::decay_t<decltype(values)>>::value;
    return member_values_t<n, C, T>{ member.members(), valueTuples };
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

template <typename C, typename... Fs>
    class value_product_t
{
private:
    std::tuple<Fs...> factors_;
public:
    constexpr value_product_t(const std::tuple<Fs...>& _factors)
        : factors_(_factors)
    {
    }
    constexpr const std::tuple<Fs...>& factors(void) const noexcept { return factors_; }
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
template <typename C, typename... Fs>
    constexpr value_product_t<C, Fs...> _to_value_product(const value_product_t<C, Fs...>& product)
{
    return product;
}

template <typename C, typename... LFs, typename... RFs>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, LFs..., RFs...> operator *(const value_product_t<C, LFs...>& lhs, const value_product_t<C, RFs...>& rhs)
{
    // TODO: we should raise an assertion if a member appears more than once
    return { std::tuple_cat(lhs.factors(), rhs.factors()) };
}
template <std::size_t N, typename C, typename... Ts, typename... Fs>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, member_values_t<N, C, Ts...>, Fs...> operator *(const member_values_t<N, C, Ts...>& lhs, const value_product_t<C, Fs...>& rhs)
{
    return _to_value_product(lhs) * rhs;
}
template <std::size_t N, typename C, typename... Ts, typename... Fs>
    MAKESHIFT_NODISCARD constexpr value_product_t<C, Fs..., member_values_t<N, C, Ts...>> operator *(const value_product_t<C, Fs...>& lhs, const member_values_t<N, C, Ts...>& rhs)
{
    return lhs * _to_value_product(rhs);
}
template <typename C, typename T, typename... Fs>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<C, T>& lhs, const value_product_t<C, Fs...>& rhs)
{
    return _to_value_product(lhs) * rhs;
}
template <typename C, typename T, typename... Fs>
    MAKESHIFT_NODISCARD constexpr auto operator *(const value_product_t<C, Fs...>& lhs, const members_t<C, T>& rhs)
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

template <std::size_t... Is, typename C, typename... Fs>
    constexpr auto _members_impl(std::index_sequence<Is...>, const value_product_t<C, Fs...>& product) noexcept
{
    return std::tuple_cat(std::get<Is>(product.factors()).members());
}
template <typename C, typename... Fs>
    constexpr auto _members(const value_product_t<C, Fs...>& product) noexcept
{
    return _members_impl(std::index_sequence_for<Fs...>{ }, product);
}

template <typename C, typename... Fs>
    constexpr auto _num_values_array(const value_product_t<C, Fs...>& product) noexcept
{
    return array_transform2(
        [](const auto& factor) { return std::tuple_size<std::decay_t<decltype(factor.values())>>::value; },
        product.factors());
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

/*template <std::size_t... Is, typename C, typename... Fs>
    constexpr auto _num_values_impl(std::index_sequence<Is...>, const value_product_t<C, Fs...>& product) noexcept
{
    return cmul(std::tuple_size<std::decay_t<decltype(std::get<Is>(product.factors()).values())>>::value...);
}*/
template <typename C, typename... Fs>
    constexpr auto _num_values(const value_product_t<C, Fs...>& product) noexcept
{
    return tuple_reduce(_num_values_array(product), std::size_t(1), std::multiplies<>{ });
}

template <std::size_t... Is, std::size_t N, typename C, typename... Ts>
    constexpr void _apply_value_impl(std::index_sequence<Is...>, C& result, const member_values_t<N, C, Ts...>& memberValues, std::size_t i)
{
    ((result.*(std::get<Is>(memberValues.members())) = std::get<Is>(memberValues.values()[i])), ...);
}
template <std::size_t N, typename C, typename... Ts>
    constexpr void _apply_value(C& result, const member_values_t<N, C, Ts...>& memberValues, std::size_t i)
{
    _apply_value_impl(std::index_sequence_for<Ts...>{ }, result, memberValues, i);
}
template <std::size_t... Is, typename C, typename... Fs>
    constexpr void _apply_values_impl(std::index_sequence<Is...>,
        C& result, const value_product_t<C, Fs...>& product,
        const std::array<std::size_t, sizeof...(Fs)>& shape, const std::array<std::size_t, sizeof...(Fs)>& strides,
        std::size_t i)
{
    (_apply_value(result, std::get<Is>(product.factors()), (i / strides[Is]) % Fs::num_values), ...);
}
template <typename C, typename... Fs>
    constexpr void _apply_values(
        C& result, const value_product_t<C, Fs...>& product,
        const std::array<std::size_t, sizeof...(Fs)>& shape, const std::array<std::size_t, sizeof...(Fs)>& strides,
        std::size_t i)
{
    _apply_values_impl(std::index_sequence_for<Fs...>{ }, result, product, shape, strides, i);
}

template <typename C, typename... Fs>
    constexpr auto _values_in(const value_product_t<C, Fs...>& product) noexcept
{
    auto shape = std::array{ Fs::num_values... };
    auto strides = _shape_to_strides(shape);
    constexpr std::size_t numValues = cmul<std::size_t>(Fs::num_values...);
    return array_transform2<numValues>(
        [&](std::size_t i)
        {
            auto result = C{ };
            _apply_values(result, product, shape, strides, i);
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
