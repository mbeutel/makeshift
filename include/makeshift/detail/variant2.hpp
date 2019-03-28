
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT2_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for size_t
#include <exception>   // for terminate()
#include <utility>     // for move(), forward<>(), integer_sequence<>
#include <type_traits> // for decay<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/constexpr.hpp>    // for constexpr_value<>, constexpr_transform()
#include <makeshift/compound.hpp>     // for compound_hash<>, compound_equal_to<>
#include <makeshift/reflect2.hpp>     // for metadata_of<>
#include <makeshift/type_traits2.hpp> // for type_sequence2<>
#include <makeshift/tuple2.hpp>       // for array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD, MAKESHIFT_CXX17

#include <makeshift/detail/workaround.hpp>   // for cmul<>()
#include <makeshift/detail/unit_variant.hpp>
#include <makeshift/detail/metadata2.hpp>    // for values_t

#ifdef MAKESHIFT_CXX17
 #include <optional>
 #include <variant>
#endif // MAKESHIFT_CXX17


namespace makeshift
{

inline namespace types
{


class unsupported_runtime_value;


} // inline namespace types


namespace detail
{


template <typename T> using is_variant_like_r = std::integral_constant<std::size_t, variant_size<T>::value>;


template <std::size_t N, typename ClassT, typename... Ts>
    class member_values_t
{
private:
    std::tuple<Ts ClassT::*...> members_;
    std::tuple<std::array<Ts, N>...> values_;

public:
    constexpr member_values_t(std::tuple<Ts ClassT::*...> _members, std::tuple<std::array<Ts, N>...> _values)
        : members_(std::move(_members)), values_(std::move(_values))
    {
    }
    constexpr const std::tuple<Ts ClassT::*...>& members(void) const noexcept { return members_; }
    constexpr const std::tuple<std::array<Ts, N>...>& values(void) const noexcept { return values_; }
    static constexpr std::size_t num_values = N;
};

template <typename ClassT, typename... Ts>
    class members_t
{
private:
    std::tuple<Ts ClassT::*...> members_;

    template <std::size_t TI, std::size_t... Is>
        constexpr static std::array<std::tuple_element_t<TI, std::tuple<Ts...>>, sizeof...(Is)> with_values_impl1(std::index_sequence<Is...>, const std::tuple<Ts...> (&vals)[sizeof...(Is)])
    {
        using std::get;
        return { get<TI>(vals[Is])... };
    }
    template <std::size_t... TIs, std::size_t N>
        constexpr static std::tuple<std::array<Ts, N>...> with_values_impl0(std::index_sequence<TIs...>, const std::tuple<Ts...> (&vals)[N])
    {
        return std::make_tuple(members_t::with_values_impl1<TIs>(std::make_index_sequence<N>{ }, vals)...);
    }

public:
    constexpr members_t(Ts ClassT::*... _members) noexcept
        : members_{ _members... }
    {
    }
    MAKESHIFT_NODISCARD constexpr member_values_t<0, ClassT, Ts...> operator =(no_values_tag) const
    {
        return { members_, { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr member_values_t<N, ClassT, Ts...> operator =(std::tuple<Ts...> (&&vals)[N]) const &&
    {
        return { members_, members_t::with_values_impl0(std::index_sequence_for<Ts...>{ }, vals) };
    }
    constexpr std::tuple<Ts ClassT::*...> members(void) const noexcept { return members_; }
};
template <typename ClassT, typename T>
    class members_t<ClassT, T>
{
private:
    std::tuple<T ClassT::*> member_;

public:
    constexpr members_t(T ClassT::* _member) noexcept
        : member_{ _member }
    {
    }
    MAKESHIFT_NODISCARD constexpr member_values_t<0, ClassT, T> operator =(no_values_tag) const
    {
        return { member_, { } };
    }
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr member_values_t<N, ClassT, T> operator =(T (&&vals)[N]) const &&
    {
        return { member_, std::make_tuple(makeshift::detail::to_array2(vals)) };
    }
    constexpr std::tuple<T ClassT::*> members(void) const noexcept { return member_; }
};

template <typename ClassT, typename T>
    constexpr auto default_values(const members_t<ClassT, T>& member)
{
    auto lvalues = metadata_of_v<T>.values();
    constexpr std::size_t n = std::tuple_size<std::decay_t<decltype(lvalues)>>::value;
    return member_values_t<n, ClassT, T>{ member.members(), std::make_tuple(lvalues) };
}

struct member_values_initializer_t
{
    template <typename ClassT, typename... Ts>
        MAKESHIFT_NODISCARD constexpr makeshift::detail::members_t<ClassT, Ts...> operator ()(Ts ClassT::*... members) const
    {
        // TODO: we should raise an assertion if a member appears more than once
        return { members... };
    }
};

template <bool Exhaustive, typename ClassT, typename... FactorsT>
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

template <typename ClassT, std::size_t N, typename... Ts>
    constexpr value_product_t<true, ClassT, member_values_t<N, ClassT, Ts...>> default_values_to_value_product(const member_values_t<N, ClassT, Ts...>& values)
{
    return { { values } };
}

template <typename ClassT, std::size_t N, typename... Ts>
    constexpr value_product_t<false, ClassT, member_values_t<N, ClassT, Ts...>> to_value_product(const member_values_t<N, ClassT, Ts...>& values)
{
    return { { values } };
}
template <typename ClassT, typename T>
    constexpr auto to_value_product(const members_t<ClassT, T>& member)
{
    return makeshift::detail::default_values_to_value_product(makeshift::detail::default_values(member));
}

template <typename ClassT, bool LExhaustive, typename... LFs, bool RExhaustive, typename... RFs>
    MAKESHIFT_NODISCARD constexpr value_product_t<LExhaustive && RExhaustive, ClassT, LFs..., RFs...> operator *(const value_product_t<LExhaustive, ClassT, LFs...>& lhs, const value_product_t<RExhaustive, ClassT, RFs...>& rhs)
{
    // TODO: we should raise an assertion if a member appears more than once
    return { std::tuple_cat(lhs.factors(), rhs.factors()) };
}
template <std::size_t N, typename ClassT, typename... Ts, bool RExhaustive, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr value_product_t<false, ClassT, member_values_t<N, ClassT, Ts...>, FactorsT...> operator *(const member_values_t<N, ClassT, Ts...>& lhs, const value_product_t<RExhaustive, ClassT, FactorsT...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * rhs;
}
template <std::size_t N, typename ClassT, typename... Ts, bool LExhaustive, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr value_product_t<LExhaustive, ClassT, FactorsT..., member_values_t<N, ClassT, Ts...>> operator *(const value_product_t<LExhaustive, ClassT, FactorsT...>& lhs, const member_values_t<N, ClassT, Ts...>& rhs)
{
    return lhs * makeshift::detail::to_value_product(rhs);
}
template <typename ClassT, typename T, bool RExhaustive, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<ClassT, T>& lhs, const value_product_t<RExhaustive, ClassT, FactorsT...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * rhs;
}
template <typename ClassT, typename T, bool LExhaustive, typename... FactorsT>
    MAKESHIFT_NODISCARD constexpr auto operator *(const value_product_t<LExhaustive, ClassT, FactorsT...>& lhs, const members_t<ClassT, T>& rhs)
{
    return lhs * makeshift::detail::to_value_product(rhs);
}
template <std::size_t N1, std::size_t N2, typename ClassT, typename... T1s, typename... T2s>
    MAKESHIFT_NODISCARD constexpr auto operator *(const member_values_t<N1, ClassT, T1s...>& lhs, const member_values_t<N2, ClassT, T2s...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}
template <std::size_t N, typename ClassT, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<ClassT, T>& lhs, const member_values_t<N, ClassT, Ts...>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}
template <std::size_t N, typename ClassT, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto operator *(const member_values_t<N, ClassT, Ts...>& lhs, const members_t<ClassT, T>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}
template <typename ClassT, typename T1, typename T2>
    MAKESHIFT_NODISCARD constexpr auto operator *(const members_t<ClassT, T1>& lhs, const members_t<ClassT, T2>& rhs)
{
    return makeshift::detail::to_value_product(lhs) * makeshift::detail::to_value_product(rhs);
}

// TODO: we could also implement operator |(), but it is a lot of tedious work, and YAGNI

template <typename T, typename ClassT>
    struct member_functor
{
private:
    T ClassT::* member_;

public:
    constexpr member_functor(T ClassT::* _member) noexcept : member_(_member) { }
    constexpr const T& operator ()(const ClassT& obj) noexcept { return obj.*member_; }
};

struct member_transform_functor
{
    template <typename T, typename ClassT>
        constexpr member_functor<T, ClassT> operator ()(T ClassT::* member) noexcept
    {
        return { member };
    }
};
template <std::size_t... Is, bool Exhaustive, typename ClassT, typename... FactorsT>
    constexpr auto members_impl(std::index_sequence<Is...>, const value_product_t<Exhaustive, ClassT, FactorsT...>& product) noexcept
{
    using std::get;
    return std::tuple_cat(get<Is>(product.factors()).members()...);
}
template <bool Exhaustive, typename ClassT, typename... FactorsT>
    constexpr auto members(const value_product_t<Exhaustive, ClassT, FactorsT...>& product) noexcept
{
    return makeshift::tuple_transform2(member_transform_functor{ }, makeshift::detail::members_impl(std::index_sequence_for<FactorsT...>{ }, product));
}
template <typename ClassT, typename T>
    constexpr std::tuple<member_functor<T, ClassT>> members(const members_t<ClassT, T>& product) noexcept
{
    using std::get;
    return { { get<0>(product.members()) } };
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


template <std::size_t... Is, std::size_t N, typename ClassT, typename... Ts>
    constexpr void apply_value_impl(std::index_sequence<Is...>, ClassT& result, const member_values_t<N, ClassT, Ts...>& memberValues, std::size_t i)
{
    using std::get;
    ((result.*(get<Is>(memberValues.members())) = get<Is>(memberValues.values())[i]), ...);
}
template <std::size_t N, typename ClassT, typename... Ts>
    constexpr void apply_value(ClassT& result, const member_values_t<N, ClassT, Ts...>& memberValues, std::size_t i)
{
    makeshift::detail::apply_value_impl(std::index_sequence_for<Ts...>{ }, result, memberValues, i);
}
template <std::size_t... Is, bool Exhaustive, typename ClassT, typename... FactorsT>
    constexpr void apply_values_impl(std::index_sequence<Is...>,
        ClassT& result, const value_product_t<Exhaustive, ClassT, FactorsT...>& product,
        const std::array<std::size_t, sizeof...(FactorsT)>& strides,
        std::size_t i)
{
    using std::get;
    (makeshift::detail::apply_value(result, get<Is>(product.factors()), (i / strides[Is]) % FactorsT::num_values), ...);
}
template <bool Exhaustive, typename ClassT, typename... FactorsT>
    constexpr void apply_values(
        ClassT& result, const value_product_t<Exhaustive, ClassT, FactorsT...>& product,
        const std::array<std::size_t, sizeof...(FactorsT)>& strides,
        std::size_t i)
{
    makeshift::detail::apply_values_impl(std::index_sequence_for<FactorsT...>{ }, result, product, strides, i);
}

template <bool Exhaustive, typename ClassT, typename... FactorsT>
    struct make_value_functor
{
    const value_product_t<Exhaustive, ClassT, FactorsT...>& product;
    const std::array<std::size_t, sizeof...(FactorsT)>& strides;

    constexpr ClassT operator ()(std::size_t i) const
    {
        auto result = ClassT{ };
        makeshift::detail::apply_values(result, product, strides, i);
        return result;
    }
};

template <bool Exhaustive, typename ClassT, typename... FactorsT>
    constexpr auto to_array(const value_product_t<Exhaustive, ClassT, FactorsT...>& product) noexcept
{
    std::array<std::size_t, sizeof...(FactorsT)> strides = makeshift::detail::shape_to_strides(std::array{ FactorsT::num_values... });
    constexpr std::size_t numValues = cmul<std::size_t>(FactorsT::num_values...);
    return makeshift::array_transform2<numValues>(
        make_value_functor<Exhaustive, ClassT, FactorsT...>{ product, strides },
        tuple_index);
}
template <std::size_t N, typename ClassT, typename... Ts>
    constexpr auto to_array(const member_values_t<N, ClassT, Ts...>& memberValues) noexcept
{
    return to_array(makeshift::detail::to_value_product(memberValues));
}
template <typename ClassT, typename T>
    constexpr auto to_array(const members_t<ClassT, T>& member) noexcept
{
    return to_array(makeshift::detail::to_value_product(member));
}


template <typename T, typename C, std::size_t I>
    struct value_functor
{
    constexpr T operator ()(void) const
    {
        constexpr auto lvalues = makeshift::constexpr_value<C>();
        return lvalues[I];
    }
};

template <typename T, typename C, typename Is>
    struct expand_type_;
template <typename T, typename C, std::size_t... Is>
    struct expand_type_<T, C, std::index_sequence<Is...>>
{
    using type = unit_variant<value_functor<T, C, Is>...>;
};

template <typename T, typename D, typename... ArgsT>
    T make_dependent(ArgsT&&... args) // to defeat first-phase name lookup
{
    return T(std::forward<ArgsT>(args)...);
}

#ifdef MAKESHIFT_CXX17
struct result_handler_optional
{
    template <typename T>
        static constexpr std::optional<std::decay_t<T>> succeed(T&& value) noexcept
    {
        return std::forward<T>(value);
    }
    template <typename T>
        static constexpr std::optional<T> fail(void) noexcept
    {
        return std::nullopt;
    }
};
#endif // MAKESHIFT_CXX17
struct result_handler_terminate
{
    template <typename T>
        static constexpr T&& succeed(T&& value) noexcept
    {
        return std::forward<T>(value);
    }
    template <typename T>
        [[noreturn]] static constexpr T fail(void) noexcept
    {
        std::terminate();
    }
};
struct result_handler_throw
{
    template <typename T>
        static constexpr T&& succeed(T&& value) noexcept
    {
        return std::forward<T>(value);
    }
    template <typename T>
        [[noreturn]] static constexpr T fail(void) noexcept
    {
        throw make_dependent<unsupported_runtime_value, T>("unsupported runtime value");
    }
};

template <typename ResultHandlerT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto value_to_variant(const T& value, C valueArrayC, HashT&& /*hash*/, EqualToT&& equal)
{
    constexpr auto lvalues = valueArrayC();
    constexpr std::size_t numValues = std::tuple_size<decltype(lvalues)>::value;
    using ExpandType = typename expand_type_<T, C, std::make_index_sequence<numValues>>::type;

    for (std::size_t i = 0; i != numValues; ++i)
        if (equal(value, lvalues[i]))
            return ResultHandlerT::succeed(ExpandType(index_value, i));
    return ResultHandlerT::template fail<ExpandType>();
}

struct value_array_functor
{
    template <typename T>
        constexpr auto operator ()(T&& arg) const
    {
        return std::forward<T>(arg).values();
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
    struct metadata_values_retriever
{
    constexpr auto operator ()(void) const
    {
        return metadata_of_v<T>;
    }
};

template <typename ResultHandlerT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl3(const T& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    auto valueArrayC = makeshift::constexpr_transform(value_array_functor{ }, valuesC);
    
        // If hash and equal are empty, we can build a hash table at compile time.
        // We won't bother in the case where they are not because we need to traverse all elements at runtime anyway.
        // (TODO: implement)
        // (TODO: is there an acceptable way of checking for constexpr-ness?)

    return makeshift::detail::value_to_variant<ResultHandlerT>(value, valueArrayC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename ResultHandlerT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl3_compound(const T& value, C productC, HashT&& hash, EqualToT&& equal)
{
    constexpr auto product = productC();
    constexpr auto valueArrayC = makeshift::constexpr_transform(to_array_functor{ }, productC);

        // If hash and equal are empty, we can build a hash table at compile time.
        // We won't bother in the case where they are not because we need to traverse all elements at runtime anyway.
        // (TODO: implement)
        // (TODO: is there an acceptable way of checking for constexpr-ness?)

    auto memberAccessor = [members = makeshift::detail::members(product)](type<T>) { return members; };
    auto compoundHash = compound_hash<decltype(memberAccessor), std::decay_t<HashT>>{ std::forward<HashT>(hash), memberAccessor };
    auto compoundEqual = compound_equal_to<decltype(memberAccessor), std::decay_t<EqualToT>>{ std::forward<EqualToT>(equal), memberAccessor };

    return makeshift::detail::value_to_variant<ResultHandlerT>(value, valueArrayC, std::move(compoundHash), std::move(compoundEqual));
}

template <typename ResultHandlerT, bool Exhaustive, typename ClassT, typename... FactorsT, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2(type<value_product_t<Exhaustive, ClassT, FactorsT...>>, const ClassT& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl3_compound<ResultHandlerT>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename ResultHandlerT, typename ClassT, std::size_t N, typename... Ts, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2(type<member_values_t<N, ClassT, Ts...>>, const ClassT& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl3_compound<ResultHandlerT>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename ResultHandlerT, typename ClassT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2(type<members_t<ClassT, T>>, const ClassT& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl3_compound<ResultHandlerT>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename ResultHandlerT, typename T, std::size_t N, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl2(type<values_t<T, N>>, const T& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl3<ResultHandlerT>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename ResultHandlerT, typename MetadataT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(std::true_type /*isMetadata*/, const T& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl3<ResultHandlerT>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}
template <typename ResultHandlerT, typename MetadataT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl1(std::false_type /*isMetadata*/, const T& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    return expand2_impl2<ResultHandlerT>(type_v<MetadataT>, value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename ResultHandlerT, typename T, typename C, typename HashT, typename EqualToT>
    constexpr auto expand2_impl0(const T& value, C valuesC, HashT&& hash, EqualToT&& equal)
{
    using Metadata = decltype(valuesC());
    return expand2_impl1<ResultHandlerT, Metadata>(std::is_base_of<metadata_tag, Metadata>{ }, value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

template <typename T> struct is_exhaustive_1_;
template <bool Exhaustive, typename T, typename... FactorsT> struct is_exhaustive_1_<value_product_t<Exhaustive, T, FactorsT...>> : std::integral_constant<bool, Exhaustive> { };
template <typename T, std::size_t N, typename... Ts> struct is_exhaustive_1_<member_values_t<N, T, Ts...>> : std::false_type { };
template <typename C, typename T> struct is_exhaustive_1_<members_t<C, T>> : std::true_type { };
template <typename T, std::size_t N> struct is_exhaustive_1_<values_t<T, N>> : std::false_type { };
template <typename T> using is_exhaustive_0_ = std::disjunction<std::is_base_of<metadata_tag, T>, is_exhaustive_1_<T>>;
template <typename C> constexpr bool is_exhaustive_v = is_exhaustive_0_<decltype(std::declval<C>()())>::value;

template <typename T> struct decay_to_args;
template <template <typename...> class T, typename... Ts> struct decay_to_args<T<Ts...>> { using type = T<Ts...>; };
template <template <typename...> class T, typename... Ts> struct decay_to_args<T<Ts...>&> { using type = T<Ts&...>; };
template <template <typename...> class T, typename... Ts> struct decay_to_args<const T<Ts...>&> { using type = T<const Ts&...>; };
template <template <typename...> class T, typename... Ts> struct decay_to_args<T<Ts...>&&> { using type = T<Ts&&...>; };
template <typename T> using decay_to_args_t = typename decay_to_args<T>::type;

#ifdef MAKESHIFT_CXX17
// TODO: this should also be extended to support variant-like types and expandable arguments

template <typename F, typename... ArgsT> using call_result_t = decltype(std::declval<F>()(std::declval<ArgsT>()...));

template <typename C, typename F, typename L, typename... Vs> struct visit_many_result_0_;
template <typename C, typename F, typename... Ls>
    struct visit_many_result_0_<C, F, type_sequence2<Ls...>>
{
    using type = type_sequence2<call_result_t<F, Ls...>>;
};
template <typename C, typename F, typename... Ls, template <typename...> class V, typename... V0s, typename... Vs>
    struct visit_many_result_0_<C, F, type_sequence2<Ls...>, V<V0s...>, Vs...>
        : type_sequence2_cat<typename visit_many_result_0_<C, F, type_sequence2<Ls..., V0s>, Vs...>::type...>
{
};

template <typename F, typename... Vs> struct visit_many_result_ : apply_<std::variant, typename unique_sequence_<typename visit_many_result_0_<type_sequence2<>, F, type_sequence2<>, decay_to_args_t<Vs>...>::type>::type> { };
#endif // MAKESHIFT_CXX17


#ifdef MAKESHIFT_CXX17
template <typename V>
    constexpr bool is_valueless_by_exception(const V&) noexcept
{
    return false;
}
template <typename... Ts>
    bool is_valueless_by_exception(const std::variant<Ts...>& variant)
{
    return variant.valueless_by_exception();
}
#endif // MAKESHIFT_CXX17


template <std::size_t CurStride, typename StridesT, typename ShapeT> struct compute_strides_0_;
template <std::size_t CurStride, std::size_t... Strides> struct compute_strides_0_<CurStride, std::index_sequence<Strides...>, std::index_sequence<>> { using type = std::index_sequence<Strides...>; };
template <std::size_t CurStride, std::size_t... Strides, std::size_t NextDim, std::size_t... Dims> struct compute_strides_0_<CurStride, std::index_sequence<Strides...>, std::index_sequence<NextDim, Dims...>>
    : compute_strides_0_<CurStride * NextDim, std::index_sequence<Strides..., CurStride>, std::index_sequence<Dims...>> { };
template <typename ShapeT> using compute_strides_ = compute_strides_0_<1, std::index_sequence<>, ShapeT>;
template <typename ShapeT> using compute_strides_t = typename compute_strides_<ShapeT>::type;

template <std::size_t LinearIndex, std::size_t I, std::size_t Stride, typename TupleT>
    constexpr decltype(auto) visit_get_element(TupleT&& args)
{
    using std::get;
    using Variant = std::remove_reference_t<std::tuple_element_t<I, std::decay_t<TupleT>>>;
    constexpr std::size_t variantIndex = (LinearIndex / Stride) % variant_size<Variant>::value;
    return get<variantIndex>(get<I>(std::forward<TupleT>(args)));
}

template <std::size_t LinearIndex, std::size_t... Strides, std::size_t... Is, typename F, typename TupleT>
    constexpr decltype(auto) visitor_impl(std::index_sequence<Strides...>, std::index_sequence<Is...>, F&& func, TupleT&& args)
{
    return func(visit_get_element<LinearIndex, Is, Strides>(std::forward<TupleT>(args))...);
}
template <std::size_t LinearIndex, typename StridesT, typename F, typename TupleT>
    constexpr decltype(auto) visitor(F&& func, TupleT&& args)
{
    return makeshift::detail::visitor_impl<LinearIndex>(StridesT{ }, std::make_index_sequence<StridesT::size()>{ }, std::forward<F>(func), std::forward<TupleT>(args));
}

template <std::size_t NumOptions, typename StridesT, typename F, typename TupleT>
    [[noreturn]] constexpr decltype(auto) visit_impl_2_small(std::integral_constant<std::size_t, NumOptions>, std::integral_constant<std::size_t, NumOptions>, StridesT, std::size_t, F&&, TupleT&&)
{
    std::terminate();
}
template <std::size_t LinearIndex, std::size_t NumOptions, typename StridesT, typename F, typename TupleT>
    constexpr decltype(auto) visit_impl_2_small(std::integral_constant<std::size_t, LinearIndex>, std::integral_constant<std::size_t, NumOptions>, StridesT, std::size_t linearIndex, F&& func, TupleT&& args)
{
    if (linearIndex == LinearIndex)
        return makeshift::detail::visitor<LinearIndex, StridesT>(std::forward<F>(func), std::forward<TupleT>(args));
    else
        return makeshift::detail::visit_impl_2_small(std::integral_constant<std::size_t, LinearIndex + 1>{ }, std::integral_constant<std::size_t, NumOptions>{ }, StridesT{ }, linearIndex, std::forward<F>(func), std::forward<TupleT>(args));
}
template <typename LinearIndicesT, typename StridesT, typename F, typename TupleT>
    struct visitor_table;
template <std::size_t... LinearIndices, typename StridesT, typename F, typename TupleT>
    struct visitor_table<std::index_sequence<LinearIndices...>, StridesT, F, TupleT>
{
    using Visitor = decltype(&visitor<0, StridesT, F, TupleT>);
    static constexpr Visitor visitors[] = {
        visitor<LinearIndices, StridesT, F, TupleT>...
    };
};
template <std::size_t... LinearIndices, typename StridesT, typename F, typename TupleT>
    constexpr typename visitor_table<std::index_sequence<LinearIndices...>, StridesT, F, TupleT>::Visitor visitor_table<std::index_sequence<LinearIndices...>, StridesT, F, TupleT>::visitors[];

template <std::size_t... LinearIndices, typename StridesT, typename F, typename TupleT>
    constexpr decltype(auto) visit_impl_2_large(std::index_sequence<LinearIndices...>, StridesT, std::size_t linearIndex, F&& func, TupleT&& args)
{
    Expects(linearIndex < sizeof...(LinearIndices));
    return visitor_table<std::index_sequence<LinearIndices...>, StridesT, F, TupleT>::visitors[linearIndex](std::forward<F>(func), std::forward<TupleT>(args));
}
template <std::size_t NumOptions, typename StridesT, typename F, typename TupleT>
    constexpr decltype(auto) visit_impl_1(std::true_type /*smallNumberOptimization*/, std::size_t linearIndex, F&& func, TupleT&& args)
{
    return makeshift::detail::visit_impl_2_small(std::integral_constant<std::size_t, 0>{ }, std::integral_constant<std::size_t, NumOptions>{ }, StridesT{ }, linearIndex, std::forward<F>(func), std::forward<TupleT>(args));
}
template <std::size_t NumOptions, typename StridesT, typename F, typename TupleT>
    constexpr decltype(auto) visit_impl_1(std::false_type /*smallNumberOptimization*/, std::size_t linearIndex, F&& func, TupleT&& args)
{
    return makeshift::detail::visit_impl_2_large(std::make_index_sequence<NumOptions>{ }, StridesT{ }, linearIndex, std::forward<F>(func), std::forward<TupleT>(args));
}

template <std::size_t... Strides, std::size_t... Is, typename TupleT>
    constexpr std::size_t visit_compute_linear_index(std::index_sequence<Strides...>, std::index_sequence<Is...>, const TupleT& args)
{
    using std::get;
    return makeshift::detail::cadd<std::size_t>((Strides * get<Is>(args).index())...);
}

template <typename F, typename... Vs>
    constexpr decltype(auto) visit_impl_0(F&& func, Vs&&... args)
{
#ifdef MAKESHIFT_CXX17
    if (makeshift::detail::cor(makeshift::detail::is_valueless_by_exception(args)...))
        throw std::bad_variant_access();
#endif // MAKESHIFT_CXX17

    constexpr std::size_t smallNumberLimit = 5;

    using Strides = compute_strides_t<std::index_sequence<variant_size<std::remove_reference_t<Vs>>::value...>>;
    constexpr std::size_t numOptions = makeshift::detail::cmul<std::size_t>(variant_size<std::remove_reference_t<Vs>>::value...);

    auto argsTuple = std::tuple<Vs&&...>(std::forward<Vs>(args)...);
    std::size_t linearIndex = makeshift::detail::visit_compute_linear_index(Strides{ }, std::index_sequence_for<Vs...>{ }, argsTuple);
    visit_impl_1<numOptions, Strides>(std::integral_constant<bool, (numOptions < smallNumberLimit)>{ }, linearIndex, std::forward<F>(func), std::move(argsTuple));
}


template <typename V>
    constexpr decltype(auto) maybe_expand_impl(std::true_type /*isVariant*/, V&& variant)
{
    return std::forward<V>(variant);
}
template <typename T>
    constexpr decltype(auto) maybe_expand_impl(std::false_type /*isVariant*/, T&& expandable)
{
    return makeshift::detail::expand2_impl0<result_handler_terminate>(expandable, metadata_values_retriever<std::decay_t<T>>{ }, hash2<>{ }, std::equal_to<>{ });
}
template <typename T>
    constexpr decltype(auto) maybe_expand(T&& arg)
{
    constexpr bool isVariantLike = can_apply_v<is_variant_like_r, std::decay_t<T>>;
    constexpr bool isExpandable = std::is_base_of<values_tag, metadata_of<std::decay_t<T>>>::value;
    static_assert(isVariantLike || isExpandable, "arguments of visit() must be either variants or implicitly expandable");

    return makeshift::detail::maybe_expand_impl(std::integral_constant<bool, isVariantLike>{ }, std::forward<T>(arg));
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT2_HPP_
