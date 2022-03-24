
#ifndef INCLUDED_MAKESHIFT_DETAIL_INDICES_2D_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_INDICES_2D_HPP_


#include <cstddef> // for size_t

#include <gsl-lite/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER


namespace makeshift {

namespace gsl = ::gsl_lite;

namespace detail {


#if gsl_CPP17_OR_GREATER
template <typename R, typename... Ts>
constexpr R cadd(Ts... vs) noexcept
{
    auto term = R{ 0 };
    return (vs + ... + term);
}
#else // gsl_CPP17_OR_GREATER
template <typename R>
constexpr R cadd(void) noexcept
{
    return R{ 0 };
}
template <typename R, typename T0, typename... Ts>
constexpr R cadd(T0 v0, Ts... vs) noexcept
{
    return v0 + cadd<R>(vs...);
}
#endif // gsl_CPP17_OR_GREATER

    // Borrowing the 2-d indexing technique that first appeared in the `tuple_cat()` implementation of Microsoft's STL.
template <std::size_t... Ns>
struct indices_2d_
{
    static constexpr std::size_t size = detail::cadd<std::size_t>(Ns...);
    static constexpr std::size_t row(std::size_t i) noexcept
    {
        std::size_t sizes[] = { Ns... };
        std::size_t r = 0;
        while (i >= sizes[r]) // compiler error if sizeof...(Ns) == 0 or i >= (Ns + ... + 0)
        {
            i -= sizes[r];
            ++r;
        }
        return r;
    }
    static constexpr std::size_t col(std::size_t i) noexcept
    {
        std::size_t sizes[] = { Ns... };
        std::size_t r = 0;
        while (i >= sizes[r]) // compiler error if sizeof...(Ns) == 0 or i >= (Ns + ... + 0)
        {
            i -= sizes[r];
            ++r;
        }
        return i;
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_INDICES_2D_HPP_
