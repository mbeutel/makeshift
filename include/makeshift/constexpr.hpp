
#ifndef INCLUDED_MAKESHIFT_CONSTEXPR_HPP_
#define INCLUDED_MAKESHIFT_CONSTEXPR_HPP_


#include <type_traits> // for invoke_result<>, decay<>

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/constexpr.hpp>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Obtains the type of the value returned by a retriever
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    using Value = retrieved_t<decltype(valueR)>; // int
    //
template <typename R>
    using retrieved_t = std::invoke_result_t<R>;


    //ᅟ
    // Extracts the value of a retriever.
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    constexpr int value = retrieve(valueR); // 42
    //
template <typename R>
    MAKESHIFT_NODISCARD constexpr std::invoke_result_t<R>
    retrieve(const R&) noexcept
{
    return makeshift::detail::stateless_functor<R>{ }();
}


    //ᅟ
    // Extracts the value of a retriever.
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    using ValueR = decltype(valueR);
    //ᅟ    constexpr int value = retrieve<ValueR>(); // 42
    //
template <typename R>
    MAKESHIFT_NODISCARD constexpr std::invoke_result_t<R>
    retrieve(void) noexcept
{
    return makeshift::detail::stateless_functor<R>{ }();
}


    //ᅟ
    // Returns a retriever for the value of the function applied to the retriever's values.
    //ᅟ
    //ᅟ    auto baseIndexR = []{ return 42; };
    //ᅟ    auto offsetR = []{ return 3; };
    //ᅟ    auto indexR = retriever_transform(std::plus<>, baseIndexR, offsetR); // equivalent to `[]{ return 45; }`
    //
template <typename F, typename... Rs>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::constexpr_transform_functor<F, Rs...>
    retriever_transform(F, Rs...) noexcept
{
    return { };
}



} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_CONSTEXPR_HPP_
