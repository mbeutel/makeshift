
#ifndef INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for is_empty<>, is_default_constructible<>, integral_constant<>, declval<>(), is_base_of<>, disjunction<>

#include <makeshift/type_traits2.hpp> // for constval_tag
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD


namespace makeshift
{

namespace detail
{


template <typename T> using type_member_r = typename T::type;

template <typename C> struct is_integral_constant_ : std::false_type { };
template <typename T, T V> struct is_integral_constant_<std::integral_constant<T, V>> : std::true_type { };


    // Workaround for non-default-constructible lambdas in C++17.
    // Does not rely on UB (as far as I can tell). Works with GCC 8.2, Clang 7.0, MSVC 19.20, and ICC 19.0 (also `constexpr` evaluation).
    // Idea taken from http://pfultz2.com/blog/2014/09/02/static-lambda/ and modified to avoid casts.
    // This construct becomes unnecessary with C++20 which permits stateless lambdas to be default-constructed.
template <typename F>
    struct stateless_functor_wrapper
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

private:
#ifdef MAKESHIFT_CXX17
    union impl
    {
        F obj;
        char dummy;

        constexpr impl(void) noexcept : dummy(0) { }
    };
#endif // MAKESHIFT_CXX17

public:
    template <typename... Ts>
        constexpr decltype(auto) operator ()(Ts&&... args) const
    {
#ifdef MAKESHIFT_CXX17
        return impl{ }.obj(std::forward<Ts>(args)...); // we can legally use `obj` even if it wasn't initialized because it is empty
#else // MAKESHIFT_CXX17
        return F{ }.obj(std::forward<Ts>(args)...); // C++14 doesn't support this trick, which is one of the reasons why lambdas cannot be used to make constexpr values (the other reason being that they cannot be constexpr)
#endif // MAKESHIFT_CXX17
    }
};
template <typename F, bool IsDefaultConstructible> struct stateless_functor_0_;
template <typename F> struct stateless_functor_0_<F, true> { using type = F; };
template <typename F> struct stateless_functor_0_<F, false> { using type = stateless_functor_wrapper<F>; };
template <typename F> using stateless_functor_t = typename stateless_functor_0_<F, std::is_default_constructible<F>::value>::type;
template <typename F> constexpr stateless_functor_t<F> stateless_functor_v = { };

template <typename T> struct is_constval_ : std::disjunction<std::is_base_of<constval_tag, T>, is_integral_constant_<T>> { };

template <typename F>
    struct constval_functor_wrapper : constval_tag
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

private:
#ifdef MAKESHIFT_CXX17
    union impl
    {
        F obj;
        char dummy;

        constexpr impl(void) noexcept : dummy(0) { }
    };
#endif // MAKESHIFT_CXX17

public:
    using value_type = decltype(std::declval<F>()());
    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const
    {
#ifdef MAKESHIFT_CXX17
        return impl{ }.obj(); // we can legally use `obj` even if it wasn't initialized because it is empty
#else // MAKESHIFT_CXX17
        return F{ }.obj(); // C++14 doesn't support this trick, which is one of the reasons why lambdas cannot be used to make constexpr values (the other reason being that they cannot be constexpr)
#endif // MAKESHIFT_CXX17
    }
#if defined(_MSC_VER ) && !defined(__INTELLISENSE__)
    MAKESHIFT_NODISCARD constexpr operator auto(void) const -> value_type // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/149701/c2833-with-operator-decltype.html#reply-152822
#else // defined(_MSC_VER ) && !defined(__INTELLISENSE__)
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const
#endif // defined(_MSC_VER ) && !defined(__INTELLISENSE__)
    {
        return (*this)();
    }
};

template <typename C, bool IsConstval> struct constval_functor_0_;
template <typename C> struct constval_functor_0_<C, true> { using type = C; };
template <typename C> struct constval_functor_0_<C, false> { using type = constval_functor_wrapper<C>; };
template <typename C> using constval_functor_t = typename constval_functor_0_<C, is_constval_<C>::value>::type;

    // determines if a given type makes a valid non-type template parameter
template <typename T> struct is_valid_nttp_ : std::disjunction<std::is_integral<T>, std::is_enum<T>, std::is_member_pointer<T>, std::is_null_pointer<T>> { };

template <typename C, bool IsValidNTTP> struct make_constval_0_;
template <typename C> struct make_constval_0_<C, true>
{
        // T makes a valid non-type template parameter; substitute the constval type with `std::integral_constant<>`
    using T = decltype(C{ }());
    using type = std::integral_constant<T, C{ }()>;
};
template <typename C> struct make_constval_0_<C, false> { using type = C; };
template <typename C> struct make_constval_ : make_constval_0_<constval_functor_t<C>, is_valid_nttp_<C>::value> { };
template <typename T, T V> struct make_constval_<std::integral_constant<T, V>> { using type = std::integral_constant<T, V>; }; // shortcut
template <typename C> using make_constval_t = typename make_constval_<C>::type;

template <typename C> constexpr auto eval_constval_v = C{ }();


    // idea taken from Ben Deane & Jason Turner, "constexpr ALL the things!", C++Now 2017
    // currently not used because VS doesn't reliably support constexpr detection
    // (...and also because we now expect that constexpr values are explicitly marked as such)
//template <typename F> using is_constexpr_functor_r = std::integral_constant<bool, (std::declval<F>()(), true)>;
template <typename F> using is_constexpr_functor_r = decltype(std::declval<F>()());


template <typename C>
    constexpr auto get_hvalue_impl(std::true_type /*constvalArg*/, const C&)
{
    return eval_constval_v<C>;
}
template <typename V>
    constexpr V get_hvalue_impl(std::false_type /*constvalArg*/, const V& arg)
{
    return arg;
}
template <typename H>
    constexpr auto get_hvalue(const H& arg)
{
    return get_hvalue_impl(is_constval_<H>{ }, arg);
}


template <typename F, typename... Cs>
    struct constval_transform_functor
{
    constexpr auto operator ()(void) const
    {
        return stateless_functor_v<F>(eval_constval_v<Cs>...);
    }
};

template <typename F, typename... Cs>
    constexpr make_constval_t<constval_transform_functor<F, Cs...>> constval_transform_impl(std::true_type /*constvalArgs*/, const Cs&...) noexcept
{
    return { };
}
template <typename F, typename... Cs>
    constexpr auto constval_transform_impl(std::false_type /*constvalArgs*/, const Cs&... args)
{
    return stateless_functor_v<F>(makeshift::detail::get_hvalue(args)...);
}


template <typename CF, typename... Cs>
    struct constval_extend_functor
{
    constexpr auto operator ()(void) const
    {
        return stateless_functor_v<CF>(make_constval_t<Cs>{ }...);
    }
};

template <typename CF, typename... Cs>
    constexpr make_constval_t<constval_extend_functor<CF, Cs...>> constval_extend_impl(std::true_type /*constvalArgs*/, const Cs&...) noexcept
{
    return { };
}
template <typename CF, typename... Cs>
    constexpr auto constval_extend_impl(std::false_type /*constvalArgs*/, const Cs&... args)
{
    return stateless_functor_v<CF>(args...);
}


template <typename C>
    constexpr auto constval_extract_impl(std::true_type /*isConstval*/, const C&) noexcept
{
    return eval_constval_v<C>;
}
template <typename C>
    constexpr C constval_extract_impl(std::false_type /*isConstval*/, const C& value)
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


#endif // INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
