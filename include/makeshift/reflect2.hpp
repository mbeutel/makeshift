
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <type_traits> // for is_class<>, is_enum<>, is_same<>, integral_constant<>, enable_if<>, is_default_constructible<>, declval<>()
#include <tuple>
#include <utility>     // for integer_sequence<>

#include <makeshift/type_traits.hpp>  // for sequence<>, can_apply<>
#include <makeshift/type_traits2.hpp> // for type<>, is_iterable<>
#include <makeshift/metadata2.hpp>
#include <makeshift/tuple2.hpp>       // for tuple_transform2(), array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD


namespace makeshift
{

namespace detail
{


template <typename T> using raw_metadata_of_r = decltype(reflect(type<T>{ }));
template <typename T> struct have_raw_metadata : can_apply<makeshift::detail::raw_metadata_of_r, T> { };
template <typename T> struct is_value_metadata : std::is_base_of<value_metadata_base, raw_metadata_of_r<T>> { };
template <typename T> struct is_compound_metadata : std::is_base_of<compound_metadata_base, raw_metadata_of_r<T>> { };


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // Determines whether values can be enumerated for the given type.
    //
template <typename T> struct have_value_metadata : std::conjunction<makeshift::detail::have_raw_metadata<T>, makeshift::detail::is_value_metadata<T>> { };
template <> struct have_value_metadata<bool> : std::true_type { };

    //ᅟ
    // Determines whether values can be enumerated for the given type.
    //
template <typename T> constexpr bool have_value_metadata_v = have_value_metadata<T>::value;


    //ᅟ
    // Determines whether members can be enumerated for the given type.
    //
template <typename T> struct have_compound_metadata : std::disjunction<
    std::conjunction<is_tuple_like2<T>, std::negation<is_iterable<T>>>,
    std::conjunction<makeshift::detail::have_raw_metadata<T>, makeshift::detail::is_compound_metadata<T>>> { };

    //ᅟ
    // Determines whether members can be enumerated for the given type.
    //
template <typename T> constexpr bool have_compound_metadata_v = have_compound_metadata<T>::value;


} // inline namespace metadata


namespace detail
{


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


inline namespace metadata
{


    //ᅟ
    // Returns an array of the enumerators of a given enumeration type, retrieved from metadata.
    // Returns `std::array{ false, true }` if the type argument is `bool`.
    //
struct values_t
{
        //ᅟ
        // Returns an array of the enumerators of a given enumeration type, retrieved from metadata.
        // Returns `std::array{ false, true }` if the type argument is `bool`.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(have_value_metadata_v<T>, "cannot enumerate values without metadata");

        return array_transform2(
            [](auto namedValue) { return namedValue.value; },
            makeshift::detail::reflector<T>{ }());
    }
};

    //ᅟ
    // Returns an array of the enumerators of a given enumeration type, retrieved from metadata.
    // Returns `std::array{ false, true }` if the type argument is `bool`.
    //
static inline constexpr values_t values = { };


    //ᅟ
    // Returns an array of the names and values of the enumerators of a given enumeration type, retrieved from metadata.
    // Returns `std::array{ with_name(false, "false"), with_name(true, "true") }` if the type argument is `bool`.
    //
struct named_values_t
{
        //ᅟ
        // Returns an array of the names and values of the enumerators of a given enumeration type, retrieved from metadata.
        // Returns `std::array{ with_name(false, "false"), with_name(true, "true") }` if the type argument is `bool`.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(have_value_metadata_v<T>, "cannot enumerate values without metadata");
    
        return makeshift::detail::reflector<T>{ }();
    }
};

    //ᅟ
    // Returns an array of the names and values of the enumerators of a given enumeration type, retrieved from metadata.
    // Returns `std::array{ with_name(false, "false"), with_name(true, "true") }` if the type argument is `bool`.
    //
static inline constexpr named_values_t named_values = { };


    //ᅟ
    // Returns a tuple of the accessors of the members in a given compound type, retrieved from metadata.
    //
struct compound_members_t
{
        //ᅟ
        // Returns a tuple of the accessors of the members in a given compound type, retrieved from metadata.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(std::is_class<T>::value, "cannot enumerate members of non-class types");
        static_assert(have_compound_metadata_v<T>, "cannot enumerate members of classes without metadata");
    
        return tuple_transform2(
            [](auto namedMember) { return namedMember.value; },
            makeshift::detail::reflector<T>{ }());
    }
};

    //ᅟ
    // Returns a tuple of the accessors of the members in a given compound type, retrieved from metadata.
    //
static inline constexpr compound_members_t compound_members = { };


    //ᅟ
    // Returns a tuple of the names and accessors of the members in a given compound type, retrieved from metadata.
    //
struct named_compound_members_t
{
        //ᅟ
        // Returns a tuple of the names and accessors of the members in a given compound type, retrieved from metadata.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto named_compound_members(type<T> = { })
    {
        static_assert(std::is_class<T>::value, "cannot enumerate members of non-class types");
        static_assert(have_compound_metadata_v<T>, "cannot enumerate members of classes without metadata");
    
        return makeshift::detail::reflector<T>{ }();
    }
};

    //ᅟ
    // Returns a tuple of the names and accessors of the members in a given compound type, retrieved from metadata.
    //
static inline constexpr named_compound_members_t named_compound_members = { };


    //ᅟ
    // Returns the value of a member of an object given an object reference and a member accessor.
    //
template <typename C, typename T>
    MAKESHIFT_NODISCARD constexpr const T& get_member_value(const C& obj, T C::* member) noexcept
{
    return obj.*member;
}

    //ᅟ
    // Returns the value of a member of an object given an object reference and a member accessor.
    //
template <typename C, typename T>
    MAKESHIFT_NODISCARD constexpr const T& get_member_value(const C& obj, T (C::* member)(void) const) noexcept
{
    return (obj.*member)();
}

    //ᅟ
    // Returns the value of a member of an object given an object reference and a member accessor.
    //
template <typename C, typename F,
          typename = decltype(std::declval<F>()(std::declval<const C&>()))>
    MAKESHIFT_NODISCARD constexpr auto get_member_value(const C& obj, F&& member)
{
    return member(obj);
}


} // inline namespace metadata


namespace detail
{


template <typename ArrayFuncT, std::size_t... Is>
    constexpr auto array_to_sequence_impl(std::index_sequence<Is...>)
{
    constexpr auto array = ArrayFuncT{ }();
    using Array = decltype(array);
    using T = typename Array::value_type;
    return sequence<T, array[Is]...>{ };
}
template <typename ArrayFuncT,
          typename = std::enable_if_t<std::is_default_constructible<ArrayFuncT>::value>>
    constexpr auto array_to_sequence(ArrayFuncT = { })
{
    using Array = decltype(ArrayFuncT{ }());
    return array_to_sequence_impl<ArrayFuncT>(std::make_index_sequence<std::tuple_size<Array>::value>{ });
}

/*template <typename T, T... Vs>
    constexpr std::array<T, sizeof...(Vs)> sequence_to_array(sequence<T, Vs...>)
{
    return { Vs... };
}*/


template <typename T, typename ValuesT>
    struct value_array_func
{
    constexpr auto operator ()(void) const noexcept
    {
        return ValuesT{ }(type_v<T>);
    }
};


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // A `sequence<>` of the values of a given type, retrieved from metadata.
    // Equals `sequence<bool, false, true>` if the type argument is `bool`.
    // TODO: will this really be used? perhaps we can simply do without...
    //
template <typename T, typename ValuesT> using value_sequence = decltype(makeshift::detail::array_to_sequence<makeshift::detail::value_array_func<T, ValuesT>>());


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
