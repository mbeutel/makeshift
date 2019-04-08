
#ifndef INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for is_empty<>, is_default_constructible<>, integral_constant<>, declval<>(), is_base_of<>, disjunction<>

#include <makeshift/type_traits2.hpp> // for constval_tag


namespace makeshift
{

namespace detail
{


template <typename T> using type_member_r = typename T::type;

template <typename T> struct is_integral_constant_1 : std::false_type { };
template <typename T, T Value> struct is_integral_constant_1<std::integral_constant<T, Value>> : std::true_type { };
template <typename T, bool HasTypeMember> struct is_integral_constant_0;
template <typename T> struct is_integral_constant_0<T, true> : is_integral_constant_1<typename T::type> { };
template <typename T> struct is_integral_constant_0<T, false> : std::false_type { };
template <typename T> struct is_integral_constant : is_integral_constant_0<T, can_apply_v<type_member_r, T>> { };


    // Workaround for non-default-constructible lambdas in C++17.
    // Does not rely on UB (as far as I can tell). Works with GCC 8.2, Clang 7.0, MSVC 19.20, and ICC 19.0 (also `constexpr` evaluation).
    // Idea taken from http://pfultz2.com/blog/2014/09/02/static-lambda/ and modified to avoid casts.
    // This construct becomes unnecessary with C++20 which permits stateless lambdas to be default-constructed.
template <typename F>
    struct stateless_lambda : constval_tag
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

private:
    union
    {
        F obj;
        char dummy;
    };

public:
    constexpr stateless_lambda(void) noexcept : dummy(0) { }
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args) const
    {
        return obj(std::forward<Ts>(args)...); // we can legally use `obj` even if it wasn't initialized because it is empty
    }
};

template <typename T> struct is_constval_ : std::disjunction<std::is_base_of<constval_tag, T>, makeshift::detail::is_integral_constant<T>> { };

template <typename F, bool IsDefaultConstructible> struct stateless_functor_0_;
template <typename F> struct stateless_functor_0_<F, true> { using type = F; };
template <typename F> struct stateless_functor_0_<F, false> { using type = stateless_lambda<F>; };
template <typename C> using as_constval_t = typename stateless_functor_0_<C, std::is_default_constructible<C>::value && is_constval_<C>::value>::type;


    // idea taken from Ben Deane & Jason Turner, "constexpr ALL the things!", C++Now 2017
    // currently not used because VS doesn't reliably support constexpr detection
    // (...and also because we now expect that constexpr values are explicitly marked as such)
//template <typename F> using is_constexpr_functor_r = std::integral_constant<bool, (std::declval<F>()(), true)>;
template <typename F> using is_constexpr_functor_r = decltype(std::declval<F>()());


template <typename C>
    constexpr auto get_hvalue_impl(std::true_type /*constvalArg*/, const C&)
{
    return as_constval_t<C>{ }();
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
    struct constval_transform_functor : constval_tag
{
    constexpr auto operator ()(void) const
    {
        return as_constval_t<F>{ }(as_constval_t<Cs>{ }()...);
    }
};

template <typename F, typename... Cs>
    constexpr constval_transform_functor<F, Cs...> constval_transform_impl(std::true_type /*constvalArgs*/, const Cs&...) noexcept
{
    return { };
}
template <typename F, typename... Cs>
    constexpr auto constval_transform_impl(std::false_type /*constvalArgs*/, const Cs&... args)
{
    return as_constval_t<F>{ }(makeshift::detail::get_hvalue(args)...);
}


template <typename CF, typename... Cs>
    struct constval_extend_functor : constval_tag
{
    constexpr auto operator ()(void) const
    {
        return as_constval_t<CF>{ }(as_constval_t<Cs>{ }...);
    }
};

template <typename CF, typename... Cs>
    constexpr constval_extend_functor<CF, Cs...> constval_extend_impl(std::true_type /*constvalArgs*/, const Cs&...) noexcept
{
    return { };
}
template <typename CF, typename... Cs>
    constexpr auto constval_extend_impl(std::false_type /*constvalArgs*/, const Cs&... args)
{
    return as_constval_t<CF>{ }(args...);
}


template <typename C>
    constexpr auto constval_extract_impl(std::true_type /*isConstval*/, const C&) noexcept
{
    return as_constval_t<C>{ }();
}
template <typename C>
    constexpr C constval_extract_impl(std::false_type /*isConstval*/, const C& value) noexcept
{
    return value;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_CONSTVAL_HPP_
