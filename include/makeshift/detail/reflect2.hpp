
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for size_t
#include <utility>     // for integer_sequence<>
#include <type_traits> // for is_base_of<>, conjunction<>, negation<>, enable_if<>

#include <makeshift/type_traits.hpp>  // for can_apply<>
#include <makeshift/type_traits2.hpp> // for type<>, is_iterable<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/metadata2.hpp> // for raw_value_metadata<>, raw_class_metadata<>


namespace makeshift
{

namespace detail
{


template <typename T> using raw_metadata_of_r = decltype(reflect(type<T>{ }));
template <typename T> struct have_raw_metadata : can_apply<raw_metadata_of_r, T> { };
template <typename T> struct is_value_metadata : std::is_base_of<value_metadata_base, raw_metadata_of_r<T>> { };
template <typename T> struct is_compound_metadata : std::is_base_of<compound_metadata_base, raw_metadata_of_r<T>> { };


template <typename T, typename ValuesT>
    constexpr const ValuesT& get_metadata(const raw_value_metadata<ValuesT>& md) noexcept
{
    return { md.values };
}
template <typename T>
    constexpr std::array<named2<T>, 0> get_metadata(raw_value_metadata<void> md) noexcept
{
    return { };
}


template <typename T, typename MembersT>
    constexpr const MembersT& get_metadata(const raw_compound_metadata<MembersT>& md) noexcept
{
    return { md.members };
}


template <typename TupleT, typename Is>
    struct tuple_reflector;
template <typename TupleT, std::size_t... Is>
    struct tuple_reflector<TupleT, std::index_sequence<Is...>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return std::make_tuple(
            with_name(
                [](const TupleT& t)
                {
                    return std::get<Is>(t);
                },
                { })...
        );
    }
};

template <typename T, typename = void>
    struct reflector;
template <>
    struct reflector<bool>
{
    constexpr std::array<named2<bool>, 2> operator ()(void) const noexcept
    {
        return {
            with_name(false, "false"),
            with_name(true, "true")
        };
    }
};
template <typename T>
    struct reflector<T, std::enable_if_t<std::conjunction_v<is_tuple_like2<T>, std::negation<is_iterable<T>>>>>
        : tuple_reflector<T, std::make_index_sequence<std::tuple_size<T>::value>>
{
};
template <typename T>
    struct reflector<T, std::enable_if_t<have_raw_metadata<T>::value>>
{
    constexpr decltype(auto) operator ()(void) const noexcept
    {
        return makeshift::detail::get_metadata<T>(reflect(type<T>{ }));
    }
};

} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
