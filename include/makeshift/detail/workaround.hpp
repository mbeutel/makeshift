
#ifndef INCLUDED_MAKESHIFT_DETAIL_WORKAROUND_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_WORKAROUND_HPP_


namespace makeshift::detail
{


template <typename R, typename... Ts>
    constexpr inline R csum(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    auto term = R{ };
    return (vs + ... + term);
}
template <typename R, typename... Ts>
    constexpr inline R cmul(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    auto term = R{ 1 };
    return (vs * ... * term);
}
template <typename... Ts>
    constexpr inline bool cor(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    return (vs || ...);
}
template <typename... Ts>
    constexpr inline bool cand(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    return (vs && ...);
}


} // namespace makeshift::detail


#endif // INCLUDED_MAKESHIFT_DETAIL_WORKAROUND_HPP_
