
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <type_traits> // for is_class<>, is_enum<>, is_same<>,integral_constant<>, enable_if<>, is_default_constructible<>, declval<>()
#include <tuple>
#include <utility>     // for move(), integer_sequence<>

#include <makeshift/type_traits.hpp> // for sequence<>, tag<>, can_apply<>
#include <makeshift/metadata2.hpp>
#include <makeshift/tuple2.hpp>      // for tuple_transform2(), array_transform2()


namespace makeshift
{

inline namespace metadata
{


template <typename EnumT, enum_flags Flags, typename ValuesT>
    struct enum_metadata;
template <typename EnumT, enum_flags Flags, std::size_t N>
    struct enum_metadata<EnumT, Flags, std::array<named2<EnumT>, N>>
{
    using value_type = EnumT;
    static constexpr enum_flags flags = Flags;
    std::array<named2<EnumT>, N> values;
};


template <typename ClassT, class_flags Flags, typename MembersT>
    struct class_metadata;
template <typename ClassT, class_flags Flags, typename... MembersT>
    struct class_metadata<ClassT, Flags, std::tuple<named2<MembersT>...>>
{
    using value_type = ClassT;
    static constexpr class_flags flags = Flags;
    std::tuple<named2<MembersT>...> members;
};


} // inline namespace metadata


namespace detail
{


template <typename T> using raw_metadata2_of_r = decltype(reflect(tag<T>{ }));


template <typename T, enum_flags Flags, typename ValuesT>
    constexpr enum_metadata<T, Flags, ValuesT> qualify_metadata(raw_enum_metadata<Flags, ValuesT> md) noexcept
{
    return { std::move(md).values };
}
template <typename T, enum_flags Flags>
    constexpr enum_metadata<T, Flags, std::array<named2<T>, 0>> qualify_metadata(raw_enum_metadata<Flags, void> md) noexcept
{
    return { { } };
}


template <typename T, class_flags Flags, typename MembersT>
    constexpr class_metadata<T, Flags, MembersT> qualify_metadata(raw_class_metadata<Flags, MembersT> md) noexcept
{
    static_assert(std::is_class<T>::value, "cannot use compound() or compound_value() for arguments of non-class type");
    return { std::move(md).values };
}


template <typename T>
    struct qualified_reflector
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::detail::qualify_metadata<T>(reflect(tag<T>{ }));
    }
};
template <>
    struct qualified_reflector<bool>
{
    constexpr auto operator ()(void) const noexcept
    {
        return enum_metadata<bool, enum_flags::none, std::array<named2<bool>, 2>>{ {
            with_name(false, "false"),
            with_name(true, "true")
        } };
    }
};


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // Use `metadata_of<T>` to look up metadata for a type.
    //
template <typename T> static constexpr auto metadata2_of = makeshift::detail::qualified_reflector<T>{ }();

    //ᅟ
    // Use `metadata_of<T>` to look up metadata for a type.
    //
template <typename T> using metadata2_of_t = decltype(metadata2_of<T>);


    //ᅟ
    // Determines whether there is metadata for the given type.
    //
template <typename T> struct have_metadata2 : can_apply<makeshift::detail::raw_metadata2_of_r, T> { };
template <> struct have_metadata2<bool> : std::true_type { };

    //ᅟ
    // Determines whether there is metadata for the given type.
    //
template <typename T> constexpr bool have_metadata2_v = have_metadata2<T>::value;


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
    constexpr auto array_to_sequence(ArrayFuncT)
{
	using Array = decltype(ArrayFuncT{ }());
    return array_to_sequence_impl<ArrayFuncT>(std::make_index_sequence<std::tuple_size<Array>::value>{ });
}

/*template <typename T, T... Vs>
    constexpr std::array<T, sizeof...(Vs)> sequence_to_array(sequence<T, Vs...>)
{
    return { Vs... };
}*/

template <typename T>
    struct values_impl
{
    static_assert(std::is_enum<T>::value, "cannot enumerate values of types other than bool and enumerations");
    static_assert(have_metadata2_v<T>, "cannot enumerate values of enumerations without metadata");

    struct value_array_func
    {
        constexpr auto operator ()(void) const noexcept
        {
            return array_transform2(
                [](auto namedValue) { return namedValue.value; },
                metadata2_of<T>.values);
        }
    };
    static constexpr auto value_sequence(void) noexcept
    {
        return array_to_sequence(value_array_func{ });
    }
};
template <>
    struct values_impl<bool>
{
    static constexpr sequence<bool, false, true> value_sequence(void) noexcept
    {
        return { };
    }
};


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // A `sequence<>` of the enumerators of a given enumeration type, retrieved from metadata.
    // Equals `sequence<bool, false, true>` if the type argument is `bool`.
    //ᅟ
template <typename T> using enum_value_sequence = decltype(makeshift::detail::values_impl<T>::value_sequence());


    //ᅟ
    // An array of the enumerators of a given enumeration type, retrieved from metadata.
    // Returns `std::array{ false, true }` if the type argument is `bool`.
    //ᅟ
template <typename T>
    constexpr auto enum_values(tag<T> = { })
{
    static_assert(std::is_enum<T>::value || std::is_same<T, bool>::value, "cannot enumerate values of types other than bool and enumerations");
    static_assert(have_metadata2_v<T>, "cannot enumerate values of enumerations without metadata");
    
    return array_transform2(
        [](auto namedValue) { return namedValue.value; },
        metadata2_of<T>.values);
}


    //ᅟ
    // An array of the names and values of the enumerators of a given enumeration type, retrieved from metadata.
    // Returns `std::array{ with_name(false, "false"), with_name(true, "true") }` if the type argument is `bool`.
    //ᅟ
template <typename T>
    constexpr auto named_enum_values(tag<T> = { })
{
    static_assert(std::is_enum<T>::value || std::is_same<T, bool>::value, "cannot enumerate values of types other than bool and enumerations");
    static_assert(have_metadata2_v<T>, "cannot enumerate values of enumerations without metadata");
    
    return metadata2_of<T>.values;
}


    //ᅟ
    // A tuple of the accessors of the members in a given compound type, retrieved from metadata.
    //ᅟ
template <typename T>
    constexpr auto compound_members(tag<T> = { })
{
    static_assert(std::is_class<T>::value, "cannot enumerate members of non-class types");
    static_assert(have_metadata2_v<T>, "cannot enumerate members of classes without metadata");
    
    return tuple_transform2(
        [](auto namedMember) { return namedMember.value; },
        metadata2_of<T>.members);
}


    //ᅟ
    // A tuple of the names and accessors of the members in a given compound type, retrieved from metadata.
    //ᅟ
template <typename T>
    constexpr auto named_compound_members(tag<T> = { })
{
    static_assert(std::is_class<T>::value, "cannot enumerate members of non-class types");
    static_assert(have_metadata2_v<T>, "cannot enumerate members of classes without metadata");
    
    return metadata2_of<T>.members;
}


    //ᅟ
    // Returns the value of a member of an object given an object reference and a member accessor.
    //ᅟ
template <typename C, typename T>
    constexpr const T& get_member_value(const C& obj, T C::* member) noexcept
{
    return obj->*member;
}

    //ᅟ
    // Returns the value of a member of an object given an object reference and a member accessor.
    //ᅟ
template <typename C, typename T>
    constexpr const T& get_member_value(const C& obj, T (C::* member)(void) const) noexcept
{
    return (obj->*member)();
}

    //ᅟ
    // Returns the value of a member of an object given an object reference and a member accessor.
    //ᅟ
template <typename C, typename F,
          typename = decltype(std::declval<F>()(std::declval<const C&>()))>
    constexpr auto get_member_value(const C& obj, F&& member)
{
    return member(obj);
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
