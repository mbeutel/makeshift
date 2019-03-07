
#ifndef INCLUDED_MAKESHIFT_CONSTEXPR_HPP_
#define INCLUDED_MAKESHIFT_CONSTEXPR_HPP_


#include <type_traits> // for invoke_result<>, decay<>, is_empty<>, enable_if<>

#include <makeshift/type_traits.hpp> // for can_apply<>
#include <makeshift/version.hpp>     // for MAKESHIFT_NODISCARD

#include <makeshift/detail/constexpr.hpp>
#include <makeshift/detail/workaround.hpp> // for cand()


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Determines whether the given type is a retriever, i.e. a stateless functor type with constexpr nullary `operator ()`.
    //
//template <typename F> struct is_retriever : std::conjunction<std::is_empty<F>, can_apply<makeshift::detail::is_constexpr_retriever_r, F>> { };
template <typename F> struct is_retriever : std::conjunction<std::is_empty<F>, can_apply<makeshift::detail::is_retriever_r, F>> { };

    //ᅟ
    // Determines whether the given type is a retriever, i.e. a stateless functor type with constexpr nullary `operator ()`.
    //
template <typename F> constexpr bool is_retriever_v = is_retriever<F>::value;


    //ᅟ
    // Obtains the type of the value returned by a retriever.
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    using Value = retrieved_t<decltype(valueR)>; // int
    //
template <typename R>
    struct retrieved
{
    static_assert(is_retriever_v<R>, "type must be a retriever");

    using type = decltype(makeshift::detail::retriever<R>{ }());
};

    //ᅟ
    // Obtains the type of the value returned by a retriever.
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    using Value = retrieved_t<decltype(valueR)>; // int
    //
template <typename R> using retrieved_t = typename retrieved<R>::type;


    //ᅟ
    // Extracts the value of a retriever.
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    constexpr int value = retrieve(valueR); // 42
    //
template <typename R>
    MAKESHIFT_NODISCARD constexpr typename std::enable_if_t<is_retriever_v<R>, retrieved<R>>::type
    retrieve(const R&) noexcept
{
    return makeshift::detail::retriever<R>{ }();
}


    //ᅟ
    // Extracts the value of a retriever.
    //ᅟ
    //ᅟ    auto valueR = []{ return 42; };
    //ᅟ    using ValueR = decltype(valueR);
    //ᅟ    constexpr int value = retrieve<ValueR>(); // 42
    //
template <typename R>
    MAKESHIFT_NODISCARD constexpr typename std::enable_if_t<is_retriever_v<R>, retrieved<R>>::type
    retrieve(void) noexcept
{
    return makeshift::detail::retriever<R>{ }();
}


    //ᅟ
    // Returns a retriever for the value of the function applied to the retrievers' values.
    //ᅟ
    //ᅟ    auto baseIndexR = []{ return 42; };
    //ᅟ    auto offsetR = []{ return 3; };
    //ᅟ    auto indexR = retriever_transform(std::plus<>, baseIndexR, offsetR); // equivalent to `[]{ return 45; }`
    //
template <typename F, typename... Rs>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::retriever_transform_functor<F, Rs...>
    retriever_transform(F, Rs...) noexcept
{
    static_assert(std::is_empty<F>::value, "transformer must be stateless");
    static_assert(makeshift::detail::cand(is_retriever_v<Rs>...), "arguments must be retrievers");
    return { };
}


    //ᅟ
    // Returns a retriever for the value of the function applied to the retrievers' values.
    //ᅟ
    //ᅟ    auto getF = [](auto indexR)
    //ᅟ    {
    //ᅟ        auto tuple = ...;
    //ᅟ        return std::get<retrieve(indexR)>(tuple);
    //ᅟ    };
    //ᅟ    auto indexR = retriever_extend(getF, []{ return 1; });
    //
template <typename RF, typename... Rs>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::retriever_extend_functor<RF, Rs...>
    retriever_extend(RF, Rs...) noexcept
{
    static_assert(std::is_empty<RF>::value, "extender must be stateless");
    static_assert(makeshift::detail::cand(is_retriever_v<Rs>...), "arguments must be retrievers");
    return { };
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_CONSTEXPR_HPP_
