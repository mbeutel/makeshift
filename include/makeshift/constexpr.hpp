
#ifndef INCLUDED_MAKESHIFT_CONSTEXPR_HPP_
#define INCLUDED_MAKESHIFT_CONSTEXPR_HPP_


#include <type_traits> // for invoke_result<>, decay<>, is_empty<>, enable_if<>

#include <makeshift/type_traits2.hpp> // for can_apply<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/constexpr.hpp>
#include <makeshift/detail/workaround.hpp> // for cand()


namespace makeshift
{

inline namespace types
{


    // For a reference to the general idea behind constexpr values, cf.
    // https://mpark.github.io/programming/2017/05/26/constexpr-function-parameters/ .


template <typename T> constexpr makeshift::detail::constexpr_t<T> constexpr_value = { };


    //ᅟ
    // Determines whether the given type is a constexpr value, i.e. a stateless functor type with constexpr nullary `operator ()`.
    //
template <typename F> struct is_constexpr_value : std::conjunction<std::is_empty<F>, can_apply<makeshift::detail::is_constexpr_value_r, F>> { };

    //ᅟ
    // Determines whether the given type is a constexpr value, i.e. a stateless functor type with constexpr nullary `operator ()`.
    //
template <typename F> constexpr bool is_constexpr_value_v = is_constexpr_value<F>::value;


    //ᅟ
    // Returns the result of the function applied to the values of the given constexpr values as a constexpr value.
    //ᅟ
    //ᅟ    auto baseIndexR = []{ return 42; };
    //ᅟ    auto offsetR = []{ return 3; };
    //ᅟ    auto indexR = constexpr_transform(std::plus<>, baseIndexR, offsetR); // equivalent to `[]{ return 45; }`
    //
template <typename F, typename... Rs>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::constexpr_transform_functor<F, Rs...>
    constexpr_transform(const F&, const Rs&...) noexcept
{
    static_assert(std::is_empty<F>::value, "transformer must be stateless");
    static_assert(makeshift::detail::cand(is_constexpr_value_v<Rs>...), "arguments must be constexpr values");
    return { };
}


    //ᅟ
    // Returns the result of the function applied to the given constexpr values as a constexpr value.
    //ᅟ
    //ᅟ    auto variantR = []{ return std::variant<int, float>{ 42 }; };
    //ᅟ    auto elementR = constexpr_extend(
    //ᅟ        [](auto _variantR)
    //ᅟ        {
    //ᅟ            constexpr auto variant = _variantR();
    //ᅟ            return std::get<variant.index()>(variant);
    //ᅟ        },
    //ᅟ        variantR);
    //ᅟ    // equivalent to `[]{ return 42; }`
    //
template <typename RF, typename... Rs>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::constexpr_extend_functor<RF, Rs...>
    constexpr_extend(const RF&, const Rs&...) noexcept
{
    static_assert(std::is_empty<RF>::value, "extender must be stateless");
    static_assert(makeshift::detail::cand(is_constexpr_value_v<Rs>...), "arguments must be constexpr values");
    return { };
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_CONSTEXPR_HPP_
