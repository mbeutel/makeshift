
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <array>
#include <type_traits> // for integral_constant<>, enable_if<>, decay<>, is_same<>, is_enum<>, is_class<>, is_member_object_pointer<>
#include <tuple>
#include <utility>     // for move(), forward<>()
#include <string_view>

#include <makeshift/type_traits.hpp> // for sequence<>, tag<>, is_instantiation_of<>, can_apply<>
#include <makeshift/tuple.hpp>       // for type_tuple<>

#include <makeshift/detail/utility_flags.hpp>


namespace makeshift
{


inline namespace metadata
{


    //ᅟ
    // Determines qualities of an enumeration type.
    //
struct enum_flag : define_flags<enum_flag>
{
        //ᅟ
        // Indicates that the enumeration type is a flags type.
        //
    static constexpr flag flags_enum { 1 };
};
using enum_flags = enum_flag::flags;


    //ᅟ
    // Determines qualities of a class type.
    //
struct class_flag : define_flags<class_flag>
{
        //ᅟ
        // Indicates that the class type is a value type (either a scalar type or a user-defined wrapper).
        //
    static constexpr flag value { 1 };

        //ᅟ
        // Indicates that the class type is a compound type, i.e. it has the semantics of a named tuple with regard to identity and comparability.
        // This type flag does not necessarily require aggregate-ness as defined in the C++ standard (`std::is_aggregate<>`), which imposes
        // unnecessary limitations (e.g. it may be reasonable for a compound type to have a user-defined constructor).
        //
    static constexpr flag compound { 2 };

        //ᅟ
        // Indicates that the class type is a composite type which itself forms a value, e.g. a geometrical point defined as `struct Point { int x, y; };`.
        //
    static constexpr flag compound_value = compound | value;
};
using class_flags = class_flag::flags;


template <typename T>
    struct named
{
    T value;
    std::string_view name;
};

template <typename T>
    constexpr named<T> with_name(T value, std::string_view name) noexcept
{
    return { std::move(value), name };
}


template <typename EnumT, enum_flags Flags, typename ValuesT>
    struct enum_metadata;
template <typename EnumT, enum_flags Flags, std::size_t N>
    struct enum_metadata<EnumT, Flags, std::array<named<EnumT>, N>>
{
    using value_type = EnumT;
    static constexpr enum_flags flags = Flags;
    std::array<named<EnumT>, N> values;
};


template <typename ClassT, class_flags Flags, typename MembersT>
    struct class_metadata;
template <typename ClassT, class_flags Flags, typename... MembersT>
    struct class_metadata<ClassT, Flags, std::tuple<named<MembersT>...>>
{
    using value_type = ClassT;
    static constexpr class_flags flags = Flags;
    std::tuple<named<MembersT>...> members;
};


} // inline namespace metadata


namespace detail
{


template <typename TupleT, typename FuncT, std::size_t... Is>
    constexpr auto tuple_transform2_impl(TupleT&& tuple, FuncT&& func, std::index_sequence<Is...>)
{
    return std::make_tuple(func(std::get<Is>(std::forward<TupleT>(tuple)))...);
}
template <typename TupleT, typename FuncT,
          typename = is_tuple_like_v<std::decay_t<TupleT>>>
    constexpr auto tuple_transform2(TupleT&& tuple, FuncT&& func)
{
    return makeshift::detail::tuple_transform2_impl(std::forward<TupleT>(tuple), std::forward<FuncT>(func), std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ });
}

template <typename T, typename ArrayT, typename FuncT, std::size_t... Is>
    constexpr auto array_transform2_impl(ArrayT&& array, FuncT&& func, std::index_sequence<Is...>)
        -> std::array<decltype(func(std::declval<T>())), sizeof...(Is)>
{
    return { func(array[Is])... };
}
template <typename T, std::size_t N, typename FuncT>
    constexpr auto array_transform2(std::array<T, N>&& array, FuncT&& func)
{
    return makeshift::detail::array_transform2_impl<T>(std::move(array), std::forward<FuncT>(func), std::make_index_sequence<N>{ });
}
template <typename T, std::size_t N, typename FuncT>
    constexpr auto array_transform2(const std::array<T, N>& array, FuncT&& func)
{
    return makeshift::detail::array_transform2_impl<T>(array, std::forward<FuncT>(func), std::make_index_sequence<N>{ });
}


template <typename ArrayT> struct array_size2_;
template <typename T, std::size_t N> struct array_size2_<std::array<T, N>> : std::integral_constant<std::size_t, N> { };
template <typename T, std::size_t N> struct array_size2_<T (&)[N]> : std::integral_constant<std::size_t, N> { };
template <typename T, std::size_t N> struct array_size2_<T (&&)[N]> : std::integral_constant<std::size_t, N> { };


    //ᅟ
    // Determines the size of the given array.
    //
template <typename ArrayT> struct array_size2 : makeshift::detail::array_size2_<std::decay_t<ArrayT>> { };

    //ᅟ
    // Determines the size of the given array.
    //
template <typename ArrayT> static constexpr std::size_t array_size2_v = array_size2<ArrayT>::value;


template <typename T> using raw_metadata2_of_r = decltype(reflect(tag<T>{ }));


template <typename T>
    struct as_named_value
{
    using value_type = T;

    static constexpr named<T> invoke(T value) noexcept
    {
        return { std::move(value), { } };
    }
};
template <typename T>
    struct as_named_value<named<T>>
{
    using value_type = T;

    static constexpr named<T> invoke(named<T> value) noexcept
    {
        return std::move(value);
    }
};


template <enum_flags Flags, typename ValuesT>
    struct raw_enum_metadata
{
    static constexpr enum_flags flags = Flags;
    ValuesT values;
};
template <enum_flags Flags>
    struct raw_enum_metadata<Flags, void>
{
    static constexpr enum_flags flags = Flags;
};


template <class_flags Flags, typename MembersT>
    struct raw_class_metadata
{
    static constexpr class_flags flags = Flags;
    MembersT members;
};


template <typename T, enum_flags Flags, typename ValuesT>
    constexpr enum_metadata<T, Flags, ValuesT> qualify_metadata(raw_enum_metadata<Flags, ValuesT> md) noexcept
{
    return { std::move(md).values };
}
template <typename T, enum_flags Flags>
    constexpr enum_metadata<T, Flags, std::array<named<T>, 0>> qualify_metadata(raw_enum_metadata<Flags, void> md) noexcept
{
    return { { } };
}


template <typename T, enum_flags Flags, typename MembersT>
    constexpr enum_metadata<T, Flags, MembersT> qualify_metadata(raw_class_metadata<Flags, MembersT> md) noexcept
{
    return { std::move(md).values };
}


} // namespace detail


inline namespace metadata
{


template <typename... Ts>
    constexpr auto flags_enumeration(Ts... args) noexcept
{
    static_assert(std::is_enum<T>::value, "cannot use flags_enumeration() for arguments of non-enumeration type");
    using T = std::common_type_t<typename makeshift::detail::as_named_value<Ts>::value_type...>; // TODO: is this clean?
    using Values = std::array<named<T>, sizeof...(Ts)>;
    return makeshift::detail::raw_enum_metadata<enum_flag::flags_enum, Values>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}
constexpr inline auto flags_enumeration(void) noexcept
{
    return makeshift::detail::raw_enum_metadata<enum_flag::flags_enum, void>{ };
}

template <typename... Ts>
    constexpr auto enumeration(Ts... args) noexcept
{
    static_assert(std::is_enum<T>::value, "cannot use enumeration() for arguments of non-enumeration type");
    using T = std::common_type_t<typename makeshift::detail::as_named_value<Ts>::value_type...>; // TODO: is this clean?
    using Values = std::array<named<T>, sizeof...(Ts)>;
    return makeshift::detail::raw_enum_metadata<enum_flags::none, Values>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}
constexpr inline auto enumeration(void) noexcept
{
    return makeshift::detail::raw_enum_metadata<enum_flags::none, void>{ };
}


template <typename... Ts>
    constexpr auto compound(Ts... args) noexcept
{
    static_assert(std::is_class<T>::value, "cannot use compound() for arguments of non-class type");
    using Members = std::tuple<typename makeshift::detail::as_named_value<Ts>::value_type...>;
    return makeshift::detail::raw_class_metadata<class_flag::compound, Members>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}

template <typename... Ts>
    constexpr auto compound_value(Ts... args) noexcept
{
    static_assert(std::is_class<T>::value, "cannot use compound_value() for arguments of non-class type");
    using Members = std::tuple<typename makeshift::detail::as_named_value<Ts>::value_type...>;
    return makeshift::detail::raw_class_metadata<class_flag::compound_value, Members>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}


    //ᅟ
    // Use `metadata_of<T>` to look up metadata for a type.
    //
template <typename T> static constexpr auto metadata2_of = makeshift::detail::qualify_metadata<T>(reflect(tag<T>{ }));

    //ᅟ
    // Use `metadata_of<T>` to look up metadata for a type.
    //
template <typename T> using metadata2_of_t = decltype(makeshift::detail::qualify_metadata<T>(reflect(tag<T>{ })));


    //ᅟ
    // Determines whether there is metadata for the given type.
    //
template <typename T> struct have_metadata2 : can_apply<makeshift::detail::raw_metadata2_of_r, T> { };

    //ᅟ
    // Determines whether there is metadata for the given type.
    //
template <typename T> constexpr bool have_metadata2_v = have_metadata2<T>::value;


} // inline namespace metadata


namespace detail
{


template <typename ArrayRetrieverT, std::size_t... Is>
    constexpr auto array_to_sequence_impl(std::index_sequence<Is...>)
{
    constexpr auto array = ArrayRetrieverT::invoke();
    using Array = decltype(array);
    using T = typename Array::value_type;
    return sequence<T, array[Is]...>{ };
}
template <typename ArrayRetrieverT>
    constexpr auto array_to_sequence(tag<ArrayRetrieverT> = { })
{
    using Array = decltype(ArrayRetrieverT::invoke());
    return array_to_sequence_impl<ArrayRetrieverT>(std::make_index_sequence<array_size2_v<Array>>{ });
}

template <typename T>
    struct values_of_impl
{
    static_assert(std::is_enum<T>::value, "cannot enumerate values of types other than bool and enumerations");
    static_assert(have_metadata2_v<T>, "cannot enumerate values of enumerations without metadata")

    struct array_retriever
    {
        static constexpr auto invoke(void) noexcept
        {
            auto values = metadata2_of<T>.values; // array of named<T>
            return array_transform2(values, [](auto namedValue) { return namedValue.value; });
        }
    };

    static constexpr auto to_array(void) noexcept
    {
        return array_retriever::invoke();
    }
    static constexpr auto to_sequence(void) noexcept
    {
        return array_to_sequence<array_retriever>();
    }
};
template <>
    struct values_of_impl<bool>
{
    static constexpr std::array<bool, 2> to_array(void) noexcept
    {
        return { false, true };
    }
    static constexpr sequence<bool, false, true> to_squence(void) noexcept
    {
        return { };
    }
};

/*template <typename T, T... Vs>
    constexpr std::array<T, sizeof...(Vs)> sequence_to_array(sequence<T, Vs...>)
{
    return { Vs... };
}*/


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // For bool and for enum types with metadata, returns a `sequence<>` of possible values.
    //ᅟ
template <typename T>
    constexpr auto value_sequence(tag<T> = { })
{
    static_assert(have_metadata2_v<T>, "no metadata was defined for the given type");
    
    return makeshift::detail::values_of_impl<T>::to_sequence();
}


    //ᅟ
    // For bool and for enum types with metadata, returns an array of possible values.
    //ᅟ
template <typename T>
    constexpr auto value_range(tag<T> = { })
{
    static_assert(have_metadata2_v<T>, "no metadata was defined for the given type");
    
    return makeshift::detail::sequence_to_array(makeshift::detail::values_of_impl<T>::to_array());
}


    //ᅟ
    // Returns a tuple of member metadata objects describing a compound class type.
    //
template <typename T>
    constexpr auto members_of(tag<T> = { })
{
    static_assert(have_metadata2_v<T>, "no metadata was defined for the given type");

    metadata_
    return metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, member_metadata>);
}


    //ᅟ
    // Returns an accessor for the given member metadata object.
    //
template <typename MemberMetadataT>
    constexpr auto member_accessor(const MemberMetadataT&)
{
    using AccessorsTuple = apply_t<type_tuple, typename MemberMetadataT::accessors>;
    auto memberObjectPointer = AccessorsTuple{ }
        | tuple_filter(trait_v<std::is_member_object_pointer>(value_type_transform_v))
        | single_or_none();
    if constexpr (!is_none_v<decltype(memberObjectPointer)>)
        return makeshift::detail::member_object_pointer_accessor{ memberObjectPointer };
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
