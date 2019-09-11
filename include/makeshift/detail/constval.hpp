
#ifndef INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for size_t
#include <utility>     // for forward<>(), get<>(), integer_sequence<>
#include <type_traits> // for integral_constant<>, declval<>(), is_base_of<>, is_integral<>, is_enum<>, is_member_pointer<>, is_null_pointer<>, is_empty<>, is_default_constructible<>

#include <makeshift/type_traits.hpp> // for constval_tag, can_instantiate<>
#include <makeshift/macros.hpp>      // for MAKESHIFT_NODISCARD


namespace makeshift
{


template <typename T, T... Vs>
    struct array_constant;
template <typename... Cs>
    struct tuple_constant;


namespace detail
{


#if MAKESHIFT_CXX == 17
 // The workaround is not legal in C++14 and no longer needed in C++20.
 // We risk ODR violation by permitting divergent paths here, but it shouldn't matter because it only affects compile-time evaluation.
 #define MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
#endif // MAKESHIFT_CXX == 17


    // Workaround for non-default-constructible lambdas in C++17.
    // Does not rely on UB (as far as I can tell). Works with GCC 8.2, Clang 7.0, MSVC 19.20, and ICC 19.0 (also `constexpr` evaluation).
    // Idea taken from http://pfultz2.com/blog/2014/09/02/static-lambda/ and modified to avoid casts.
    // This construct becomes unnecessary with C++20 which permits stateless lambdas to be default-constructed.
template <typename F>
    struct stateless_functor_wrapper
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

private:
#ifdef MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
    union impl
    {
        F obj;
        char dummy;

        constexpr impl(void) noexcept : dummy(0) { }
    };
#endif // MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_

public:
    template <typename... Ts>
        constexpr decltype(auto) operator ()(Ts&&... args) const
    {
#ifdef MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
        return impl{ }.obj(std::forward<Ts>(args)...); // we can legally use `obj` even if it wasn't initialized because it is empty
#else // MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
        return F{ }.obj(std::forward<Ts>(args)...); // C++14 doesn't support this trick, which is one of the reasons why lambdas cannot be used to make constvals (the other reason being that they cannot be constexpr)
#endif // MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
    }
};
template <typename F, bool IsDefaultConstructible> struct stateless_functor_0_;
template <typename F> struct stateless_functor_0_<F, true> { using type = F; };
template <typename F> struct stateless_functor_0_<F, false> { using type = stateless_functor_wrapper<F>; };
template <typename F> using stateless_functor_t = typename stateless_functor_0_<F, std::is_default_constructible<F>::value>::type;
template <typename F> constexpr stateless_functor_t<F> stateless_functor_v = { };

template <typename T, typename F>
    struct constval_base : constval_tag
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

public:
    using value_type = T;

private:
#ifdef MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
        // We can legally use `obj` even if it wasn't initialized because it is empty. Or at least that's what happens to work for GCC, Clang, and VC++ with /std:c++17.
        // C++14 doesn't support this trick, which is one of the reasons why lambdas cannot be used to make constexpr values (the other reason being that they cannot be constexpr).
        // C++20 obviates the need for this workaround because it makes stateless lambdas default-constructible.
    union impl
    {
        F obj;
        char dummy;

        constexpr impl(void) noexcept : dummy(0) { }
    };
#endif // MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_

public:
    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const
    {
#ifdef MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
        return impl{ }.obj();
#else // MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
        return F{ }();
#endif // MAKESHIFT_NEED_LAMBDA_DEFAULT_CTR_WORKAROUND_
    }
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTELLISENSE__)
    MAKESHIFT_NODISCARD constexpr operator auto(void) const -> value_type // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/149701/c2833-with-operator-decltype.html#reply-152822
#else // defined(_MSC_VER) && !defined(__clang__) && !defined(__INTELLISENSE__)
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const
#endif // defined(_MSC_VER) && !defined(__clang__) && !defined(__INTELLISENSE__)
    {
        return (*this)();
    }
};

    // Returns the canonical constval representation of the given proto-constval.
template <typename C> struct make_constval_;
template <typename C> using make_constval = typename make_constval_<C>::type;

    // Represents an object as a constval.
template <typename T, typename F>
    struct constval : constval_base<T, F>
{
    static constexpr T value = constval_base<T, F>{ }();

private:
#if defined(_MSC_VER) && !defined(NDEBUG)
        // for Natvis support
    static inline const T value_ = value;
#endif // defined(_MSC_VER) && !defined(NDEBUG)
};

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
    MAKESHIFT_NODISCARD constexpr
    make_constval<tuple_accessor_functor<I, tuple_like_constval<T, F>>>
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
    static inline const value_type value_ = value;
#endif // defined(_MSC_VER) && !defined(NDEBUG)

public:
    constexpr operator T(void) const noexcept { return value; }
    constexpr value_type operator ()(void) const noexcept { return value; }
};

    // Determines if a given type makes a valid C++14 non-type template parameter.
template <typename T> struct is_valid_nttp_ : disjunction<std::is_integral<T>, std::is_enum<T>, std::is_member_pointer<T>, std::is_null_pointer<T>> { };

    // Represent constvals of type `std::array<>` as `array_constant<>` of the array values if the array element type is a valid NTTP type,
    // or as an `array_constant<>` of const references to constexpr objects otherwise. This way, syntax for `array_constant<>` users can be
    // agnostic of the NTTP-ness, except perhaps for having to `decay<>` the element type and to use `c<>` to construct a constval for an
    // array element in the general case.
template <std::size_t I, typename C>
    struct array_accessor_functor
{
    constexpr auto operator ()(void) const
    {
        return C{ }()[I];
    }
};
template <bool IsElementValidNTTP, typename T, typename Is, typename C>
    struct make_array_constval_;
template <typename T, std::size_t... Is, typename C>
    struct make_array_constval_<true, T, std::index_sequence<Is...>, C>
{
    using type = array_constant<T, C{ }()[Is]...>;
};
template <typename T, std::size_t... Is, typename C>
    struct make_array_constval_<false, T, std::index_sequence<Is...>, C>
{
        // For types which are not valid NTTP types, we pass constexpr const references instead.
    using type = array_constant<T const&, make_constval<array_accessor_functor<Is, C>>::value...>;
};

    // Represent constvals of type `std::tuple<>` as `tuple_constant<>` of the constval types of the elements.
template <typename Is, typename C>
    struct make_tuple_constval_;
template <std::size_t... Is, typename C>
    struct make_tuple_constval_<std::index_sequence<Is...>, C>
{
    using type = tuple_constant<make_constval<tuple_accessor_functor<Is, C>>...>;
};

    // Normalize constvals of tuple-like type.
template <bool IsTupleLike, typename T, typename C> struct make_constval_3_;
template <typename T, typename C> struct make_constval_3_<true, T, C> { using type = tuple_like_constval<T, C>; };
template <typename T, typename C> struct make_constval_3_<false, T, C> { using type = constval<T, C>; };
template <typename T, T const& Ref> struct make_constval_3_<false, T, ref_constval<T, Ref>> { using type = ref_constval<T, Ref>; }; // shortcut: use ref_constval<> directly if no normalization applies

    // Return constval tag types unaltered.
template <bool IsConstval, typename C> struct make_tag_constval_;
template <typename C> struct make_tag_constval_<true, C> { using type = C; };
template <typename C> struct make_tag_constval_<false, C> : make_constval_3_<is_tuple_like<C>::value, C, C> { };

    // Normalize NTTP constvals.
template <bool IsValidNTTP, typename T, typename C> struct make_constval_2_;
template <typename T, typename C> struct make_constval_2_<true, T, C> { using type = std::integral_constant<T, C{ }()>; };
template <typename T, typename C> struct make_constval_2_<false, T, C> : make_constval_3_<is_tuple_like<T>::value, T, C> { };

    // Normalize constvals of type `std::array<>` and `std::tuple<>` to `array_constant<>` and `tuple_constant<>`, and handle constval tag types.
template <typename T, typename C> struct make_constval_1_ : make_constval_2_<is_valid_nttp_<T>::value, T, C> { };
template <typename C> struct make_constval_1_<C, C> : make_tag_constval_<std::is_base_of<constval_tag, C>::value, C> { };
template <typename T, std::size_t N, typename C> struct make_constval_1_<std::array<T, N>, C> : make_array_constval_<is_valid_nttp_<T>::value, T, std::make_index_sequence<N>, C> { };
template <typename... Ts, typename C> struct make_constval_1_<std::tuple<Ts...>, C> : make_tuple_constval_<std::make_index_sequence<sizeof...(Ts)>, C> { };

    // Return known constval types unaltered.
template <typename C> struct make_constval_ : make_constval_1_<decltype(std::declval<C>()()), C> { };
template <typename T, T V> struct make_constval_<std::integral_constant<T, V>> { using type = std::integral_constant<T, V>; };
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

template <typename C> constexpr auto constval_value = C{ }(); // workaround for EDG (TODO: remove)


    // idea taken from Ben Deane & Jason Turner, "constexpr ALL the things!", C++Now 2017
    // currently not used because VS doesn't reliably support constexpr detection
    // (...and also because we now expect that constvals are explicitly marked as such)
//template <typename F> using is_constexpr_functor_r = std::integral_constant<bool, (std::declval<F>()(), true)>;
template <typename F> using is_constexpr_functor_r = decltype(std::declval<F>()());


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
    return makeshift::detail::get_hvalue_impl(is_constval_<H>{ }, arg);
}


template <typename F, typename... Cs>
    struct constval_transform_functor
{
    constexpr auto operator ()(void) const
    {
        return stateless_functor_v<F>(constval_value<Cs>...);
    }
};

template <typename F, typename... Cs>
    constexpr auto constval_transform_impl(std::true_type /*constvalArgs*/, Cs const&...) noexcept
{
    return make_constval<constval_transform_functor<F, Cs...>>{ };
}
template <typename F, typename... Cs>
    constexpr auto constval_transform_impl(std::false_type /*constvalArgs*/, Cs const&... args)
{
    return stateless_functor_v<F>(makeshift::detail::get_hvalue(args)...);
}


template <typename CF, typename... Cs>
    struct constval_extend_functor
{
    constexpr auto operator ()(void) const
    {
        return stateless_functor_v<CF>(make_constval<Cs>{ }...);
    }
};

template <typename CF, typename... Cs>
    constexpr auto constval_extend_impl(std::true_type /*constvalArgs*/, Cs const&...) noexcept
{
    return make_constval<constval_extend_functor<CF, Cs...>>{ };
}
template <typename CF, typename... Cs>
    constexpr auto constval_extend_impl(std::false_type /*constvalArgs*/, Cs const&... args)
{
    return stateless_functor_v<CF>(args...);
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
    Expects(arg);
}


} // namespace detail

} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for `tuple_like_constval<>`.
template <typename T, typename F> struct tuple_size<makeshift::detail::tuple_like_constval<T, F>> : tuple_size<T> { };
template <std::size_t I, typename T, typename F>
    struct tuple_element<I, makeshift::detail::tuple_like_constval<T, F>>
{
    using type = makeshift::detail::make_constval<makeshift::detail::tuple_accessor_functor<I, makeshift::detail::tuple_like_constval<T, F>>>;
};


} // namespace std


#endif // INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
