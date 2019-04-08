
#ifndef INCLUDED_MAKESHIFT_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_CONSTVAL_HPP_


#include <type_traits> // for is_empty<>, conjunction<>

#include <makeshift/type_traits2.hpp> // for can_apply<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/constval.hpp>


namespace makeshift
{

inline namespace types
{


    // For a reference to the general idea behind constexpr values, cf.
    // https://mpark.github.io/programming/2017/05/26/constexpr-function-parameters/ .


    //ᅟ
    // Determines whether the given type is a constexpr value.
    //
template <typename T> struct is_constval : makeshift::detail::is_constval_<T> { };

    //ᅟ
    // Determines whether the given type is a constexpr value.
    //
template <typename T> constexpr bool is_constval_v = is_constval<T>::value;


    //ᅟ
    // A constexpr value type representing the given constexpr function object type.
    //
template <typename C> using constval_t = makeshift::detail::as_constval_t<C>;


    //ᅟ
    // A constexpr value representing the given constexpr function object type.
    //
template <typename C> constexpr constval_t<C> constval = { };


    //ᅟ
    // Returns a constexpr value representing the given constexpr function object type.
    //
template <typename C>
    constexpr constval_t<C> make_constval(const C&) noexcept
{
    static_assert(std::is_empty<C>::value, "argument must be stateless");
    static_assert(can_apply_v<makeshift::detail::is_constexpr_functor_r, C>, "argument must be constexpr function object");

    return { };
}


    //ᅟ
    // Returns the value of a constexpr value, or passes through the argument if it is not a constexpr value.
    //
template <typename C>
    constexpr auto constval_extract(const C& value) noexcept
{
    return makeshift::detail::constval_extract_impl(is_constval<C>{ }, value);
}


    //ᅟ
    // Returns the result of the function applied to the values of the given constexpr values as a constexpr value, or the result value itself if one of the arguments is not a constexpr value.
    //ᅟ
    //ᅟ    auto baseIndexR = make_constval([]{ return 42; });
    //ᅟ    auto offsetR = make_constval([]{ return 3; });
    //ᅟ    auto indexR = constval_transform(std::plus<>, baseIndexR, offsetR); // equivalent to `make_constval([]{ return 45; })`
    //
template <typename F, typename... Cs>
    MAKESHIFT_NODISCARD constexpr auto
    constval_transform(const F&, const Cs&... args) noexcept
{
    static_assert(std::is_empty<F>::value, "transformer must be stateless");
    return makeshift::detail::constval_transform_impl<F>(std::conjunction<is_constval<Cs>...>{ }, args...);
}


    //ᅟ
    // Returns the result of the function applied to the given constexpr values as a constexpr value, or the result value itself if one of the arguments is not a constexpr value.
    //ᅟ
    //ᅟ    auto variantR = make_constval([]{ return std::variant<int, float>{ 42 }; });
    //ᅟ    auto elementR = constval_extend(
    //ᅟ        [](auto _variantR)
    //ᅟ        {
    //ᅟ            constexpr auto variant = _variantR();
    //ᅟ            return std::get<variant.index()>(variant);
    //ᅟ        },
    //ᅟ        variantR);
    //ᅟ    // equivalent to `make_constval([]{ return 42; })`
    //
template <typename CF, typename... Cs>
    MAKESHIFT_NODISCARD constexpr auto
    constval_extend(const CF&, const Cs&... args) noexcept
{
    static_assert(std::is_empty<CF>::value, "extender must be stateless");
    return makeshift::detail::constval_extend_impl<CF>(std::conjunction<is_constval<Cs>...>{ }, args...);
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_CONSTVAL_HPP_
