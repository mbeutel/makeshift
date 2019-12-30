
#ifndef INCLUDED_MAKESHIFT_DETAIL_ENUM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ENUM_HPP_


namespace makeshift
{

namespace detail
{


template <typename T>
constexpr bool is_flag_power_of_2(T value) noexcept
{
    return value > 0
        && (value & (value - 1)) == 0;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ENUM_HPP_
