
#ifndef INCLUDED_MAKESHIFT_DETAIL_INDICES_2D_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_INDICES_2D_HPP_


#include <cstddef>  // for size_t


namespace makeshift {

namespace detail {


template <typename R, typename... Ts>
constexpr R cadd(Ts... vs) noexcept
{
    auto term = R{ 0 };
    return (vs + ... + term);
}

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
