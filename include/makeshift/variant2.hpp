
#ifndef INCLUDED_MAKESHIFT_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_VARIANT2_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for size_t
#include <variant>
#include <optional>
#include <utility>     // for move(), forward<>(), integer_sequence<>
#include <functional>  // for equal_to<>
#include <type_traits> // for decay<>, is_class<>, is_empty<>, is_default_constructible<>

#include <makeshift/reflect2.hpp> // for values_of()
#include <makeshift/tuple2.hpp>   // for array_transform2()
#include <makeshift/compound.hpp> // for compound_hash<>, compound_equal_to<>
#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/workaround.hpp> // for cmul<>()


namespace makeshift
{

namespace detail
{


template <typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2(const T (&array)[N])
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
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr values_t<N, C> operator =(const std::array<C, N>& vals) const
    {
        return { vals };
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
template <std::size_t N, typename C, typename... Ts>
    constexpr auto _values_in(const member_values_t<N, C, Ts...>& memberValues) noexcept
{
    return _values_in(_to_value_product(memberValues));
}
template <std::size_t N, typename C>
    constexpr auto _values_in(const values_t<N, C>& values) noexcept
{
    return values.values;
}

    // Workaround for non-default-constructible lambdas in C++17.
    // Does not rely on UB (as far as I can tell). Works with GCC 8.2, Clang 7.0, MSVC 19.20, and ICC 19.0 (also `constexpr` evaluation).
    // Idea taken from http://pfultz2.com/blog/2014/09/02/static-lambda/ and modified to avoid casts.
    // TODO: look at https://www.boost.org/doc/libs/1_65_1/libs/type_traits/doc/html/boost_typetraits/reference/is_stateless.html
template <typename F>
    struct stateless_lambda
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

private:
    union
    {
        F obj; // we can legally use `obj` even if it wasn't initialized because it is empty
        char dummy;
    };

public:
    constexpr stateless_lambda(void) noexcept : dummy(0) { }
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args) const
    {
        return obj(std::forward<Ts>(args)...);
    }
};

template <typename F, bool IsDefaultConstructible> struct stateless_functor_0_;
template <typename F> struct stateless_functor_0_<F, true> { using type = F; };
template <typename F> struct stateless_functor_0_<F, false> { using type = stateless_lambda<F>; };
template <typename F> using stateless_functor = typename stateless_functor_0_<F, std::is_default_constructible_v<F>>::type;

template <typename T, typename F, std::size_t I>
    struct value_functor
{
    /*constexpr T operator ()(void) const
    {
        constexpr auto value = _values_in(F{ }(object_t<T>{ }))[I];
        return value;
    }*/
    friend constexpr T get(const value_functor&)
    {
        constexpr auto value = F{ }()[I];
        return value;
    }
};

template <typename T, typename F, typename Is>
    struct expand_type_;
template <typename T, typename F, std::size_t... Is>
    struct expand_type_<T, F, std::index_sequence<Is...>>
{
    using type = std::variant<value_functor<T, F, Is>...>;
};
template <typename ExpandTypeT>
    using expand_type_factory = ExpandTypeT (*)(void);
template <typename ExpandTypeT, std::size_t N>
    constexpr auto _make_expand_factories(void)
{
    return array_transform2<N>(
        [](auto i) -> expand_type_factory<ExpandTypeT>
        {
            return +[]
            {
                constexpr std::size_t I = decltype(i)::value;
                return ExpandTypeT{ std::in_place_index<I> };
            };
        },
        tuple_index);
}
template <typename T, typename F, typename HashT, typename EqualToT>
     constexpr auto _value_to_variant(const T& value, F&&, HashT&& /*hash*/, EqualToT&& equal)
{
    using ValuesFunc = stateless_functor<std::decay_t<F>>;
    constexpr auto values = ValuesFunc{ }();
    constexpr std::size_t numValues = std::tuple_size<decltype(values)>::value;
    using ExpandType = typename expand_type_<T, ValuesFunc, std::make_index_sequence<numValues>>::type;

    constexpr auto factories = _make_expand_factories<ExpandType, numValues>();

    for (std::size_t i = 0; i != numValues; ++i)
        if (equal(value, values[i]))
            return std::optional<ExpandType>{ ExpandType{ factories[i]() } };
    return std::optional<ExpandType>{ std::nullopt };
}

template <typename T, typename F, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2(const T& value, F&&, HashT&& hash, EqualToT&& equal)
{
    using ValuesFunc = stateless_functor<std::decay_t<F>>;
    constexpr auto valuesFunc2 = [] { return ValuesFunc{ }(object_t<T>{ }).values; };

        // If hash and equal are empty, we can build a hash table at compile time.
        // We won't bother in the case where they are not because we need to traverse all elements at runtime anyway.
        // (TODO: implement)
        // (TODO: is there an acceptable way of checking for constexpr-ness?)

    return _value_to_variant(value, std::move(valuesFunc2), std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename T, typename F, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2_compound(const T& value, F&&, HashT&& hash, EqualToT&& equal)
{
    using ProductFunc = stateless_functor<std::decay_t<F>>;
    constexpr auto product = ProductFunc{ }(object_t<T>{ });
    constexpr auto valuesFunc = [] { return _values_in(ProductFunc{ }(object_t<T>{ })); };

        // If hash and equal are empty, we can build a hash table at compile time.
        // We won't bother in the case where they are not because we need to traverse all elements at runtime anyway.
        // (TODO: implement)
        // (TODO: is there an acceptable way of checking for constexpr-ness?)

    auto memberAccessor = [members = _members(product)](type<T>) { return members; };
    auto compoundHash = compound_hash<std::decay_t<HashT>, decltype(memberAccessor)>{ std::forward<HashT>(hash), memberAccessor };
    auto compoundEqual = compound_equal_to<std::decay_t<EqualToT>, decltype(memberAccessor)>{ std::forward<EqualToT>(equal), memberAccessor };

    return _value_to_variant(value, std::move(valuesFunc), std::move(compoundHash), std::move(compoundEqual));
}

template <typename T, typename... FactorsT, typename F, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(type<value_product_t<T, FactorsT...>>, const T& value, F&& valuesFunc, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2_compound(value, std::forward<F>(valuesFunc), std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename T, std::size_t N, typename... Ts, typename F, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(type<member_values_t<N, T, Ts...>>, const T& value, F&& valuesFunc, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2_compound(value, std::forward<F>(valuesFunc), std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename T, std::size_t N, typename F, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(type<values_t<N, T>>, const T& value, F&& valuesFunc, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2(value, std::forward<F>(valuesFunc), std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename T, typename F, typename HashT, typename EqualToT>
    constexpr auto expand2_impl0(const T& value, F&& valuesFunc, HashT&& hash, EqualToT&& equal)
{
    using ValuesT = decltype(valuesFunc(object_t<T>{ }));
    return expand2_impl1(type_v<ValuesT>, value, std::forward<F>(valuesFunc), std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}


} // namespace detail


inline namespace types
{


    //ᅟ
    // Given a runtime value, a retriever of known values, a hasher, and an equality comparer, returns an optional variant of retrievers
    // of the known values.
    //ᅟ
    //ᅟ    auto bitValuesR = [](auto value) { return value = { 16, 32, 64 }; };
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = try_expand(bits, bitValuesR, hash2<>{ }, std::equal_to<>{ });
    //ᅟ
    //ᅟ    std::visit([](auto bitsR)
    //ᅟ               {
    //ᅟ                   constexpr int bitsC = get(bitsR);
    //ᅟ                   ...
    //ᅟ               },
    //ᅟ        bitsV.value());
    //
template <typename T, typename F, typename HashT, typename EqualToT>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, F&& valuesFunc, HashT&& hash, EqualToT&& equal)
{
    return makeshift::detail::expand2_impl0(value, std::forward<F>(valuesFunc), std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

    //ᅟ
    // Given a runtime value and a retriever of known values, returns an optional variant of retrievers of the known values.
    //ᅟ
    //ᅟ    auto bitValuesR = [](auto value) { return value = { 16, 32, 64 }; };
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = try_expand(bits, bitValuesR);
    //ᅟ
    //ᅟ    std::visit([](auto bitsR)
    //ᅟ               {
    //ᅟ                   constexpr int bitsC = get(bitsR);
    //ᅟ                   ...
    //ᅟ               },
    //ᅟ        bitsV.value());
    //
template <typename T, typename F>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, F&& valuesFunc)
{
    return try_expand2(value, std::forward<F>(valuesFunc), hash2<>{ }, std::equal_to<>{ });
}

    //ᅟ
    // Given a runtime value with metadata, a hasher, and an equality comparer, returns an optional variant of retrievers
    // of the known values.
    //ᅟ
    //ᅟ    enum class Precision { single, double_ };
    //ᅟ    constexpr static auto reflect(mk::type<Precision>) { ... }
    //ᅟ
    //ᅟ    Precision precision = ...;
    //ᅟ    auto precisionV = try_expand(precision, hash2<>{ }, std::equal_to<>{ });
    //ᅟ
    //ᅟ    std::visit([](auto precisionR)
    //ᅟ               {
    //ᅟ                   constexpr int precisionC = get(precisionR);
    //ᅟ                   ...
    //ᅟ               },
    //ᅟ        precisionV.value());
    //
template <typename T, typename HashT, typename EqualToT>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, HashT&& hash, EqualToT&& equal)
{
    return try_expand2(value,
        [](auto values)
        {
            return values = values_of(type_v<T>);
        },
        std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

    //ᅟ
    // Given a runtime value with metadata, returns an optional variant of retrievers of the known values.
    //ᅟ
    //ᅟ    enum class Precision { single, double_ };
    //ᅟ    constexpr static auto reflect(mk::type<Precision>) { ... }
    //ᅟ
    //ᅟ    Precision precision = ...;
    //ᅟ    auto precisionV = try_expand(precision);
    //ᅟ
    //ᅟ    std::visit([](auto precisionR)
    //ᅟ               {
    //ᅟ                   constexpr int precisionC = get(precisionR);
    //ᅟ                   ...
    //ᅟ               },
    //ᅟ        precisionV.value());
    //
template <typename T>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value)
{
    return try_expand2(value,
        [](auto values) constexpr
        {
            return values = values_of(type_v<T>);
        });
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_VARIANT2_HPP_
