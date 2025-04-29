
#ifndef INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_


#include <array>
#include <tuple>
#include <cstddef>      // for size_t, ptrdiff_t
#include <utility>      // for forward<>(), get<>()
#include <type_traits>  // for integral_constant<>, declval<>(), is_base_of<>, is_integral<>, is_enum<>, is_member_pointer<>, is_null_pointer<>, is_empty<>, is_default_constructible<>, common_type<>, make_signed<>, conjunction<>, disjunction<>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, gsl_Assert()

#if gsl_CPP20_OR_GREATER
# include <iterator>
#endif // gsl_CPP20_OR_GREATER

#include <makeshift/type_traits.hpp>  // for constval_tag, can_instantiate<>

#if gsl_CPP20_OR_GREATER
# define MAKESHIFT_CONSTVAL_(...)                                       \
    (makeshift::detail::make_constval(                                  \
        []                                                              \
        {                                                               \
            return __VA_ARGS__;                                         \
        }))
#else  // gsl_CPP20_OR_GREATER ^^^ / vvv !gsl_CPP20_OR_GREATER
# define MAKESHIFT_CONSTVAL_(...)                                       \
    (makeshift::detail::make_constval(                                  \
        []                                                              \
        {                                                               \
            struct R_                                                   \
            {                                                           \
                /* explicit return type is workaround for Clang bug */  \
                constexpr std::decay_t<decltype(__VA_ARGS__)>           \
                operator ()(void) const noexcept                        \
                {                                                       \
                    return __VA_ARGS__;                                 \
                }                                                       \
            };                                                          \
            return R_{ };                                               \
        }()))
#endif  // !gsl_CPP20_OR_GREATER

namespace makeshift {

namespace gsl = ::gsl_lite;


template <typename T, T... Vs>
struct array_constant;

template <typename... Cs>
struct tuple_constant;


namespace detail {


template <typename C> constexpr auto constval_value = C{ }(); // workaround for EDG (TODO: remove?)


    // Returns the canonical constval representation of the given proto-constval.
template <typename C> struct make_constval_;
template <typename C> using constval_t = typename make_constval_<C>::type;

    // Represents an object as a constval.
template <typename T, typename F>
struct constval : constval_tag
{
    using value_type = T;

    [[nodiscard]] constexpr value_type operator ()(void) const
    {
        return F{ }();
    }
#if defined(_MSC_VER) && _MSC_VER < 1927 && !defined(__clang__) && !defined(__NVCC__) && !defined(__INTELLISENSE__)
    [[nodiscard]] constexpr operator auto(void) const -> value_type // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/149701/c2833-with-operator-decltype.html#reply-152822
#else // defined(_MSC_VER) && !defined(__clang__) && !defined(__NVCC__) && !defined(__INTELLISENSE__)
    [[nodiscard]] constexpr operator value_type(void) const
#endif // defined(_MSC_VER) && !defined(__clang__) && !defined(__NVCC__) && !defined(__INTELLISENSE__)
    {
        return (*this)();
    }

    static constexpr T value = constval_value<F>;

private:
#if defined(_MSC_VER) && !defined(NDEBUG)
        // for Natvis support
    static inline const T value_ = value;
#endif // defined(_MSC_VER) && !defined(NDEBUG)
};
template <typename T, typename F>
constexpr T constval<T, F>::value;

template <std::size_t I, typename C>
struct tuple_accessor_functor
{
    constexpr auto operator ()(void) const
    {
        using std::get;
        return get<I>(C{ }());
    }
};

    // Represents an object of tuple-like type as a constval, preserving tuple-likeness.
template <typename T, typename F>
struct tuple_like_constval : constval<T, F>
{
};
template <std::size_t I, typename T, typename F>
[[nodiscard]] constexpr
constval_t<tuple_accessor_functor<I, tuple_like_constval<T, F>>>
get(tuple_like_constval<T, F>) noexcept
{
    static_assert(I < std::tuple_size<T>::value, "index out of range");
    return { };
}

    // Represents a constexpr object captured by const reference as a constval.
template <typename T, T const& Ref>
struct ref_constval : constval_tag
{
public:
    using value_type = T;
    static constexpr value_type value = Ref;

private:
#if defined(_MSC_VER) && !defined(NDEBUG)
        // for Natvis support
    static inline const T value_ = value;
#endif // defined(_MSC_VER) && !defined(NDEBUG)

public:
    constexpr operator T(void) const noexcept { return value; }
    constexpr value_type operator ()(void) const noexcept { return value; }
};
template <typename T, T const& Ref>
constexpr T ref_constval<T, Ref>::value;

    // Determines if a given type makes a valid C++14 non-type template parameter.
template <typename T> struct is_valid_nttp_ : std::disjunction<std::is_integral<T>, std::is_enum<T>, std::is_member_pointer<T>, std::is_null_pointer<T>> { };

    // Represent constvals of type `std::array<>` as `array_constant<>` of the array values if the array element type is a valid NTTP type,
    // or as an `array_constant<>` of const references to constexpr objects otherwise. This way, syntax for `array_constant<>` users can be
    // agnostic of the NTTP-ness, except perhaps for having to `decay<>` the element type and to use `c<>` to construct a constval for an
    // array element in the general case.
template <std::size_t I, typename C>
struct array_accessor_functor
{
    constexpr auto operator ()(void) const
    {
        return std::get<I>(C{ }());
    }
};

template <bool IsElementValidNTTP, typename T, typename Is, typename C>
struct make_array_constval_;
template <typename T, std::size_t... Is, typename C>
struct make_array_constval_<true, T, std::index_sequence<Is...>, C>
{
    using type = array_constant<T, std::get<Is>(C{ }())...>;
};
template <typename T, std::size_t... Is, typename C>
struct make_array_constval_<false, T, std::index_sequence<Is...>, C>
{
        // For types which are not valid NTTP types, we pass constexpr const references instead.
    using type = array_constant<T const&, constval_t<array_accessor_functor<Is, C>>::value...>;
};

    // Represent constvals of type `std::tuple<>` as `tuple_constant<>` of the constval types of the elements.
template <typename Is, typename C>
struct make_tuple_constval_;
template <std::size_t... Is, typename C>
struct make_tuple_constval_<std::index_sequence<Is...>, C>
{
    using type = tuple_constant<constval_t<tuple_accessor_functor<Is, C>>...>;
};

    // Normalize constvals of tuple-like type.
template <bool IsTupleLike, typename T, typename C> struct make_constval_4_;
template <typename T, typename C> struct make_constval_4_<true, T, C> { using type = tuple_like_constval<T, C>; };
template <typename T, typename C> struct make_constval_4_<false, T, C> { using type = constval<T, C>; };
template <typename T, T const& Ref> struct make_constval_4_<false, T, ref_constval<T, Ref>> { using type = ref_constval<T, Ref>; }; // shortcut: use `ref_constval<>` directly if no normalization applies

    // Return constval tag types unaltered.
template <bool IsConstval, typename T, typename C> struct make_constval_3_;
template <typename T, typename C> struct make_constval_3_<true, T, C> { using type = T; };
template <typename T, typename C> struct make_constval_3_<false, T, C> : make_constval_4_<is_tuple_like<T>::value, T, C> { };

    // Normalize NTTP constvals.
template <bool IsValidNTTP, typename T, typename C> struct make_constval_2_;
template <typename T, typename C> struct make_constval_2_<true, T, C> { using type = std::integral_constant<T, constval_value<C>>; };
template <typename T, typename C> struct make_constval_2_<false, T, C> : make_constval_3_<std::is_base_of<constval_tag, T>::value, T, C> { };

    // Normalize constvals of type `std::array<>` and `std::tuple<>` to `array_constant<>` and `tuple_constant<>`, and handle constval tag types.
template <typename T, typename C> struct make_constval_1_ : make_constval_2_<is_valid_nttp_<T>::value, T, C> { };
template <typename C> struct make_constval_1_<C, C> : make_constval_3_<std::is_base_of<constval_tag, C>::value, C, C> { }; // shortcut
template <typename T, std::size_t N, typename C> struct make_constval_1_<std::array<T, N>, C> : make_array_constval_<is_valid_nttp_<T>::value, T, std::make_index_sequence<N>, C> { };
template <typename... Ts, typename C> struct make_constval_1_<std::tuple<Ts...>, C> : make_tuple_constval_<std::make_index_sequence<sizeof...(Ts)>, C> { };

    // Return known constval types unaltered.
template <typename C> struct make_constval_ : make_constval_1_<decltype(std::declval<C>()()), C> { };
template <typename T, T V> struct make_constval_<std::integral_constant<T, V>> { using type = std::integral_constant<T, V>; };
template <typename T> struct make_constval_<gsl::type_identity<T>> { using type = gsl::type_identity<T>; };
template <typename T, T... Vs> struct make_constval_<array_constant<T, Vs...>> { using type = array_constant<T, Vs...>; };
template <typename... Cs> struct make_constval_<tuple_constant<Cs...>> { using type = tuple_constant<Cs...>; };
template <typename T, typename C> struct make_constval_<constval<T, C>> { using type = constval<T, C>; };
template <typename T, T const& Ref> struct make_constval_<ref_constval<T, Ref>> { using type = ref_constval<T, Ref>; };
template <typename T, typename C> struct make_constval_<tuple_like_constval<T, C>> { using type = tuple_like_constval<T, C>; };

template <bool IsRef, typename ConstRefT, ConstRefT Ref>
struct constant_0_
{
         // We use `make_constval_1_<>` directly to skip the short-circuiting of `ref_constval<>` here.
    using T = std::remove_const_t<std::remove_reference_t<ConstRefT>>;
    using type = typename make_constval_1_<T, ref_constval<T, Ref>>::type;
};
template <typename T, T V>
struct constant_0_<false, T, V>
{
        // Currently, "`T` being a non-ref type" is equivalent to "`T` is a valid NTTP".
    using type = std::integral_constant<T, V>;
};
template <typename T, T V> struct constant_ : constant_0_<std::is_reference<T>::value, T, V> { };

template <bool IsValidNTTP, typename T> struct array_constant_element_type_0_;
template <typename T> struct array_constant_element_type_0_<true, T> { using type = T; };
template <typename T> struct array_constant_element_type_0_<false, T> { using type = T const&; };
template <typename T> struct array_constant_element_type_ : array_constant_element_type_0_<is_valid_nttp_<T>::value, T> { };


template <typename C>
constexpr auto get_hvalue_impl(std::true_type /*constvalArg*/, C const&)
{
    return constval_value<C>;
}
template <typename V>
constexpr V get_hvalue_impl(std::false_type /*constvalArg*/, V const& arg)
{
    return arg;
}
template <typename H>
constexpr auto get_hvalue(H const& arg)
{
    return detail::get_hvalue_impl(is_constval_<H>{ }, arg);
}


template <typename F, typename... Cs>
struct constval_transform_functor
{
    constexpr auto operator ()(void) const
    {
        return F{ }(Cs{ }()...);
    }
};

template <typename F, typename... Cs>
constexpr auto constval_transform_impl(std::true_type /*constvalArgs*/, Cs const&...) noexcept
{
    return constval_t<constval_transform_functor<F, Cs...>>{ };
}
template <typename F, typename... Cs>
constexpr auto constval_transform_impl(std::false_type /*constvalArgs*/, Cs const&... args)
{
    return F{ }(detail::get_hvalue(args)...);
}


template <typename CF, typename... Cs>
struct constval_extend_functor
{
    constexpr auto operator ()(void) const
    {
        return CF{ }(Cs{ }...);
    }
};

template <typename CF, typename... Cs>
constexpr auto constval_extend_impl(std::true_type /*constvalArgs*/, Cs const&...) noexcept
{
    return constval_t<constval_extend_functor<CF, Cs...>>{ };
}
template <typename CF, typename... Cs>
constexpr auto constval_extend_impl(std::false_type /*constvalArgs*/, Cs const&... args)
{
    return CF{ }(args...);
}


template <typename C>
constexpr auto constval_extract_impl(std::true_type /*isConstval*/, C const&) noexcept
{
    return constval_value<C>;
}
template <typename C>
constexpr C constval_extract_impl(std::false_type /*isConstval*/, C const& value)
{
    return value;
}


template <typename BoolC>
constexpr void constval_assert_impl(std::true_type /*isConstval*/, BoolC arg) noexcept
{
    static_assert(arg(), "constval assertion failed");
}
static constexpr void constval_assert_impl(std::false_type /*isConstval*/, bool arg)
{
    gsl_Assert(arg);
}


template <std::size_t N, typename R>
constexpr auto
constval_range_to_array_impl(R&& range)
{
    using IteratorT = decltype(std::ranges::begin(std::declval<R&>()));
    using T = std::iter_value_t<IteratorT>;
    auto result = std::array<T, N>{ };
    std::ranges::copy(range, result.begin());
    return result;
}


template <typename C>
constexpr constval_t<C> make_constval(C const&)
{
    return { };
}


template <typename ContainerT>
constexpr std::integral_constant<std::size_t, std::tuple_size<ContainerT>::value>
csize_impl(std::true_type /*isConstval*/, ContainerT const&)
{
    return { };
}
template <typename ContainerT>
constexpr auto csize_impl(std::false_type /*isConstval*/, ContainerT const& c)
    -> decltype(c.size())
{
    return c.size();
}

template <typename ContainerT>
constexpr std::integral_constant<std::ptrdiff_t, std::tuple_size<ContainerT>::value> cssize_impl(std::true_type /*isConstval*/, ContainerT const&)
{
    return { };
}
template <typename ContainerT>
constexpr auto cssize_impl(std::false_type /*isConstval*/, ContainerT const& c)
    -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>
{
    using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
    return static_cast<R>(c.size());
}


} // namespace detail

} // namespace makeshift


    // Implement tuple-like protocol for `tuple_like_constval<>`.
template <typename T, typename F> class std::tuple_size<makeshift::detail::tuple_like_constval<T, F>> : public std::tuple_size<T> { };
template <std::size_t I, typename T, typename F>
class std::tuple_element<I, makeshift::detail::tuple_like_constval<T, F>>
{
public:
    using type = makeshift::detail::constval_t<makeshift::detail::tuple_accessor_functor<I, makeshift::detail::tuple_like_constval<T, F>>>;
};


#endif // INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
