
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <type_traits> // for integral_constant<>

#include <makeshift/utility.hpp> // for type<>


namespace makeshift {


template <typename T, typename> struct values_provider;


namespace detail {


template <typename T> constexpr bool is_std_array_v = false;
template <typename T, std::size_t N> constexpr bool is_std_array_v<std::array<T, N>> = true;

template <typename T, typename ValuesT>
auto reflect_values_check_type(ValuesT const& values)
{
    static_assert(is_std_array_v<ValuesT>, "reflect_values(type<T>) must return a std::array<> of elements of type T");
    return values;
}


template <>
struct default_values<bool>
{
    constexpr std::array<bool, 2> operator ()(void) const
    {
        return { false, true };
    }
};
template <>
struct default_values<std::nullptr_t>
{
    constexpr std::array<std::nullptr_t, 1> operator ()(void) const
    {
        return { nullptr };
    }
};


template <typename T> using reflect_values_r = decltype(detail::reflect_values_check_type<T>(reflect_values(type_c<T>)));
template <typename T> using default_values_r = decltype(default_values<T>{ }());
template <typename T> using have_reflect_values = can_instantiate<detail::reflect_values_r, T>;
template <typename T> using have_default_values = can_instantiate<detail::default_values_r, T>;

template <bool HaveReflectValues, typename T> struct values_of_1_;
template <typename T>
struct values_of_1_<true, T>
{
    constexpr reflect_values_r<T> operator ()(void) const
    {
        return reflect_values(type_c<T>);
    }
};
template <typename T> struct values_of_1_<false, T> : values_provider<T, void> { };
template <bool HaveDefaultValues, typename T> struct values_of_0_;
template <typename T> struct values_of_0_<true, T> : default_values<T> { };
template <typename T> struct values_of_0_<false, T> : values_of_1_<have_reflect_values<T>::value, T> { };
template <typename T> struct values_of_ : values_of_0_<have_default_values<T>::value, T> { };

template <typename T> using values_of_r = decltype(values_of_<T>{ }());


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_REFLECT_HPP_
