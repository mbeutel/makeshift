
#ifndef INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_
#define INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_


#include <cstddef>     // for size_t
#include <utility>     // for tuple_size<>, tuple_element<>
#include <type_traits> // for integral_constant<>, declval<>(), conjunction<>, is_enum<>, is_same<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/type_traits2.hpp>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // `basic_type<>` is a generic type tag. Use `basic_type<>` instead of `type<>` if argument deduction is desired:
    //ᅟ
    //     template <typename T>
    //         std::is_integral<T> is_integral_f(basic_type<T>);
    //
template <typename T>
    struct basic_type : makeshift::detail::type_tag
{
    using type = T;
};


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
template <template <typename...> class Z, typename... Ts> struct can_apply : makeshift::detail::can_apply_1_<Z, void, Ts...> { };

    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
    //
template <template <typename...> class Z, typename... Ts> constexpr bool can_apply_v = can_apply<Z, Ts...>::value;


    //ᅟ
    // Type sequence, i.e. tuple without runtime value representation.
    //
template <typename... Ts>
    struct type_sequence2
{
    constexpr type_sequence2(void) noexcept = default;
    constexpr type_sequence2(basic_type<Ts>...) noexcept { }
};
template <>
    struct type_sequence2<>
{
};


    //ᅟ
    // Type sequence, i.e. tuple without runtime value representation.
    //
template <typename... Ts> constexpr inline type_sequence2<Ts...> type_sequence2_v { };


} // inline namespace types


namespace detail
{


template <typename T>
    struct type_
{
        //ᅟ
        // `type<>` is a generic type tag.
        //ᅟ
        // `type<>` is defined as a dependent type, hence the type argument cannot be deduced, and no argument-dependent lookups relating to the type argument take place.
        // `type<T>` inherits from `basic_type<T>`. Declare function arguments with type `basic_type<T>` to permit deduction of `T`.
        // `type<T>` also has the subclass `adl_type<T>`. Pass arguments of type `adl_type<T>` to enable argument-dependent lookup related to `T`.
        //
    struct _ : basic_type<T>
    {
         // We hide the actual definition here to suppress ADL (cf. https://quuxplusone.github.io/blog/2019/04/09/adl-insanity-round-2/).

            // This conversion exists so expressions of type `type<>` can be used as case labels of switch statements over type enums.
        template <typename EnumT,
                  typename TypeEnumTypeT = decltype(type_enum_type_of_(EnumT{ }, makeshift::detail::unwrap_enum_tag{ })),
                  typename = std::enable_if_t<try_index_of_type_in<T, typename TypeEnumTypeT::type::types>::value != -1>>
            constexpr operator EnumT(void) const noexcept
        {
            return EnumT(int(try_index_of_type_in<T, typename TypeEnumTypeT::types>::value));
        }

        template <typename Type2T,
                  typename = std::enable_if_t<std::is_base_of<type_tag, Type2T>::value>>
            MAKESHIFT_NODISCARD constexpr std::is_same<T, typename Type2T::type>
            operator ==(Type2T) const noexcept
        {
            return { };
        }
        template <typename Type2T,
                  typename = std::enable_if_t<std::is_base_of<type_tag, Type2T>::value>>
            MAKESHIFT_NODISCARD constexpr std::negation<std::is_same<T, typename Type2T::type>>
            operator !=(Type2T) const noexcept
        {
            return { };
        }
    };
};


} // namespace detail


inline namespace types
{


    //ᅟ
    // `type<>` is a generic type tag.
    //ᅟ
    // `type<>` is defined as a dependent type, hence the type argument cannot be deduced, and no argument-dependent lookups relating to the type argument take place.
    // `type<T>` inherits from `basic_type<T>`. Declare function arguments with type `basic_type<T>` to permit deduction of `T`.
    // `type<T>` also has the subclass `adl_type<T>`. Pass arguments of type `adl_type<T>` to enable argument-dependent lookup related to `T`.
    //
template <typename T> using type = typename makeshift::detail::type_<T>::_;

    //ᅟ
    // `type<>` is a generic type tag.
    //ᅟ
    // `type<>` is defined as a dependent type, hence no argument-dependent lookups relating to the type argument take place.
    // `type<T>` also has the subclass `adl_type<T>`. Pass arguments of type `adl_type<T>` to enable argument-dependent lookup related to `T`.
    //
template <typename T> constexpr inline type<T> type_v { };


    //ᅟ
    // `adl_type<>` is a generic type tag. Use `adl_type<>` instead of `type<>` if argument-dependent lookup is desired:
    //ᅟ
    //     auto rawMetadata = reflect(adl_type_v<MyType>); // search for `reflect()` in associated namespaces and classes of `MyType`
    //
template <typename T> struct adl_type : type<T> { };

    //ᅟ
    // `adl_type<>` is a generic type tag. Use `adl_type<>` instead of `type<>` if argument-dependent lookup is desired:
    //ᅟ
    //     auto rawMetadata = reflect(adl_type_v<MyType>); // search for `reflect()` in associated namespaces and classes of `MyType`
    //
template <typename T> constexpr inline adl_type<T> adl_type_v { };


    //ᅟ
    // Return a type sequence that represents the types of the given values.
    //
template <typename... Ts>
    constexpr type_sequence2<Ts...> make_type_sequence2(basic_type<Ts>...) noexcept
{
    return { };
}


    //ᅟ
    // Returns the `I`-th element in the type sequence.
    //
template <std::size_t I, typename... Ts>
    constexpr type<nth_type_t<I, Ts...>> get(const type_sequence2<Ts...>&) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return { };
}

    //ᅟ
    // Returns the type sequence element of type `T`.
    //
template <typename T, typename... Ts>
    constexpr type<T> get(const type_sequence2<Ts...>&) noexcept
{
	constexpr std::size_t index = try_index_of_type_v<T, Ts...>;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


    //ᅟ
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> struct type_sequence2_cat : makeshift::detail::type_sequence_cat_<type_sequence2<>, Ts...> { };

    //ᅟ
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> using type_sequence2_cat_t = typename type_sequence2_cat<Ts...>::type;


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
template <typename T> struct is_iterable : can_apply<makeshift::detail::is_iterable_ns::is_iterable_r, T> { };

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
template <typename T> struct is_flags_enum : std::conjunction<std::is_enum<T>, can_apply<flag_type_of, T>> { };

    //ᅟ
    // Determines whether the given type is a flags enum.
    //
template <typename T> constexpr bool is_flags_enum_v = is_flags_enum<T>::value;


    //ᅟ
    // Tag class to identify user-defined constexpr values. Inherit from `constval_tag` to declare your function object a constexpr value.
    //
struct constval_tag { };


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class tuple_size<makeshift::type_sequence2<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_sequence2<Ts...>> { using type = makeshift::type<makeshift::nth_type_t<I, Ts...>>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_
