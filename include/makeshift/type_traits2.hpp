
#ifndef INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_
#define INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_


#include <cstddef>     // for size_t
#include <utility>     // for tuple_size<>, tuple_element<>
#include <type_traits> // for integral_constant<>, declval<>(), conjunction<>, is_enum<>, is_same<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD, MAKESHIFT_CXX17

#include <makeshift/detail/type_traits2.hpp>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // `type<>` is a generic type tag.
    //ᅟ
    // Use `type_v<T>` as a value representation of `T`.
    //
template <typename T> using type = makeshift::detail::type_t<T>;


    //ᅟ
    // Use `type_v<T>` as a value representation of `T` for tag dispatching.
    //
template <typename T> constexpr inline makeshift::detail::type_t<T> type_v { };


    //ᅟ
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts> struct nth_type : makeshift::detail::nth_type_<N, Ts...> { };

    //ᅟ
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts> using nth_type_t = typename nth_type<N, Ts...>::type;


    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence. If `T` does not appear in the type sequence, the index is `size_t(-1)`.
    //
template <typename T, typename... Ts> struct try_index_of_type : std::integral_constant<std::size_t, makeshift::detail::try_type_pack_index_<T, Ts...>::value> { };

    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence. If `T` does not appear in the type sequence, the index is `size_t(-1)`.
    //
template <typename T, typename... Ts> constexpr std::size_t try_index_of_type_v = try_index_of_type<T, Ts...>::value;


    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts> struct index_of_type : std::integral_constant<std::size_t, makeshift::detail::type_pack_index_<T, Ts...>::value> { };

    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts> constexpr std::size_t index_of_type_v = index_of_type<T, Ts...>::value;


    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
    //
template <template <typename...> class Z, typename... Ts> struct can_instantiate : makeshift::detail::can_instantiate_<Z, void, Ts...> { };

    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
    //
template <template <typename...> class Z, typename... Ts> constexpr bool can_instantiate_v = can_instantiate<Z, Ts...>::value;


    //ᅟ
    // Type sequence, i.e. tuple without runtime value representation.
    //
template <typename... Ts>
    struct type_sequence
{
    constexpr type_sequence(void) noexcept = default;
    constexpr type_sequence(makeshift::detail::type_t<Ts>...) noexcept { }
};
template <>
    struct type_sequence<>
{
};


    //ᅟ
    // Type sequence, i.e. tuple without runtime value representation.
    //
template <typename... Ts> constexpr inline type_sequence<Ts...> type_sequence_v { };


    //ᅟ
    // Return a type sequence that represents the types of the given values.
    //
template <typename... Ts>
    constexpr type_sequence<Ts...> make_type_sequence(type<Ts>...) noexcept
{
    return { };
}


#ifdef MAKESHIFT_CXX17
    //ᅟ
    // Encodes a value in a type.
    //
template <auto V> using constant = std::integral_constant<decltype(V), V>;

    //ᅟ
    // Encodes a value in a type.
    //
template <auto V> constexpr constant<V> c{ };
#endif // MAKESHIFT_CXX17


} // inline namespace types


namespace detail
{


template <typename T, typename TypeSeqT> struct try_index_of_type_in;
template <typename T, template <typename...> class TypeSeqT, typename... Ts> struct try_index_of_type_in<T, TypeSeqT<Ts...>> : try_index_of_type<T, Ts...> { };


    //ᅟ
    // `type<>` is a generic type tag.
    //ᅟ
    // Use `type_v<T>` as a value representation of `T`.
    //
template <typename T>
    struct type_t
{
    using type = T;

        // This conversion exists so expressions of type `type<>` can be used as case labels of switch statements over type enums.
    template <typename EnumT,
              typename TypeEnumTypeT = decltype(type_enum_type_of_(EnumT{ }, makeshift::detail::unwrap_enum_tag{ })),
              typename = std::enable_if_t<try_index_of_type_in<T, typename TypeEnumTypeT::type::types>::value != -1>>
        constexpr operator EnumT(void) const noexcept
    {
        return EnumT(int(try_index_of_type_in<T, typename TypeEnumTypeT::types>::value));
    }
};

template <typename T1, typename T2>
    MAKESHIFT_NODISCARD constexpr std::is_same<T1, T2>
    operator ==(type_t<T1>, type_t<T2>) noexcept
{
    return { };
}
template <typename T1, typename T2>
    MAKESHIFT_NODISCARD constexpr std::negation<std::is_same<T1, T2>>
    operator !=(type_t<T1>, type_t<T2>) noexcept
{
    return { };
}


} // namespace detail


inline namespace types
{


    //ᅟ
    // Returns the `I`-th element in the type sequence.
    //
template <std::size_t I, typename... Ts>
    constexpr type<nth_type_t<I, Ts...>> get(const type_sequence<Ts...>&) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return { };
}

    //ᅟ
    // Returns the type sequence element of type `T`.
    //
template <typename T, typename... Ts>
    constexpr type<T> get(const type_sequence<Ts...>&) noexcept
{
	constexpr std::size_t index = try_index_of_type_v<T, Ts...>;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


    //ᅟ
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> struct type_sequence_cat : makeshift::detail::type_sequence_cat_<type_sequence<>, Ts...> { };

    //ᅟ
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> using type_sequence_cat_t = typename type_sequence_cat<Ts...>::type;


    //ᅟ
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U> struct is_instantiation_of : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_instantiation_of<U<Ts...>, U> : std::true_type { };

    //ᅟ
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U> constexpr bool is_instantiation_of_v = is_instantiation_of<T, U>::value;


    //ᅟ
    // Determines whether the given type is iterable, i.e. functions `begin()` and `end()` are well-defined for arguments of type `T`.
    //
template <typename T> struct is_iterable : can_instantiate<makeshift::detail::is_iterable_ns::is_iterable_r, T> { };

    //ᅟ
    // Determines whether the given type is iterable, i.e. functions `begin()` and `end()` are well-defined for arguments of type `T`.
    //
template <typename T> constexpr bool is_iterable_v = is_iterable<T>::value;


    //ᅟ
    // Retrieves the flag type (i.e. the `struct` which inherits from `define_flags<>` and defines flag values) of a flags enum.
    //
template <typename T> using flag_type_of = decltype(flag_type_of_(std::declval<T>(), makeshift::detail::unwrap_enum_tag{ }));

    //ᅟ
    // Retrieves the flag type (i.e. the `struct` which inherits from `define_flags<>` and defines flag values) of a flags enum.
    //
template <typename T> using flag_type_of_t = typename flag_type_of<T>::type;


    //ᅟ
    // Determines whether the given type is a flags enum.
    //
template <typename T> struct is_flags_enum : std::conjunction<std::is_enum<T>, can_instantiate<flag_type_of, T>> { };

    //ᅟ
    // Determines whether the given type is a flags enum.
    //
template <typename T> constexpr bool is_flags_enum_v = is_flags_enum<T>::value;


    //ᅟ
    // Retrieves the given type as a dependent type. This can be useful to suppress type inference.
    //
template <typename T> using as_dependent_type = typename makeshift::detail::as_dependent_type_<T>::type;


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class tuple_size<makeshift::type_sequence<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_sequence<Ts...>> { using type = makeshift::type<makeshift::nth_type_t<I, Ts...>>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_
