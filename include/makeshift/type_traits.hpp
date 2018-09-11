
#ifndef INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
#define INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_


#include <type_traits> // for integral_constant<>, declval<>()
#include <utility>     // for integer_sequence<>


namespace makeshift
{

namespace detail
{


struct serializer_base { };

struct flags_base { };
struct flags_tag { };

struct constrained_integer_base { };

struct any_tag_tag { };
template <typename... Ts> any_tag_tag any_tag_func(Ts...) { return { }; }

struct all_tags_tag { };
template <typename... Ts> all_tags_tag all_tags_func(Ts...) { return { }; }


template <typename...> using void_t = void; // ICC doesn't have std::void_t<> yet
template <template <typename...> class, typename, typename...> struct can_apply_1_ : std::false_type { };
template <template <typename...> class Z, typename... Ts> struct can_apply_1_<Z, void_t<Z<Ts...>>, Ts...> : std::true_type { };


template <typename RSeqT, typename... Ts> struct type_sequence_cat_;
template <typename RSeqT> struct type_sequence_cat_<RSeqT> { using type = RSeqT; };
template <template <typename...> class TypeSeqT, typename... RSeqT, typename... NSeqT, typename... Ts>
    struct type_sequence_cat_<TypeSeqT<RSeqT...>, TypeSeqT<NSeqT...>, Ts...>
        : type_sequence_cat_<TypeSeqT<RSeqT..., NSeqT...>, Ts...> 
{
};


    // taken from http://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
    // (cf. the same URL for a discussion of the benefits and drawbacks of the MI approach vs. a recursive one)
template <std::size_t I, typename T>
    struct type_pack_index_base
{
    static constexpr std::size_t index = I;
    using type = T;
};
struct type_pack_no_match
{
    static constexpr std::size_t index = std::size_t(-1);
};
template <typename IsT, typename... Ts> struct type_pack_indexer;
template <std::size_t... Is, typename... Ts> struct type_pack_indexer<std::index_sequence<Is...>, Ts...> : type_pack_index_base<Is, Ts>... { };
template <std::size_t I, typename T>
    static type_pack_index_base<I, T> select_type_seq_entry_by_idx(type_pack_index_base<I, T>);
template <typename T, std::size_t I>
    static type_pack_index_base<I, T> select_type_seq_entry_by_type(type_pack_index_base<I, T>);
template <typename T>
    static type_pack_no_match select_type_seq_entry_by_type(...);

template <std::size_t I, typename... Ts>
    struct type_pack_element_
{
    using index_base = decltype(makeshift::detail::select_type_seq_entry_by_idx<I>(type_pack_indexer<std::make_index_sequence<sizeof...(Ts)>, Ts...>{ }));
    using type = typename index_base::type;
};

template <typename T, typename... Ts>
    struct try_type_pack_index_
{
    using index_base = decltype(makeshift::detail::select_type_seq_entry_by_type<T>(type_pack_indexer<std::make_index_sequence<sizeof...(Ts)>, Ts...>{ }));
    static constexpr std::size_t value = index_base::index;
};

template <typename T, typename... Ts>
    struct type_pack_index_
{
    static constexpr std::size_t value = type_pack_index_<T, Ts...>::value;
    static_assert(value != ~std::size_t(0), "type T does not appear in type sequence");
};


    // borrowed from the VC++ STL's variant implementation
template <typename T, std::size_t I>
    struct type_with_index
{
    static constexpr std::size_t index = I;
    using type = T;
};
template <std::size_t I, typename T>
    struct value_overload_
{
    using type = type_with_index<T, I> (*)(T);
    operator type(void) const;
};
template <typename Is, typename... Ts> struct value_overload_set_;
template <std::size_t... Is, typename... Ts> struct value_overload_set_<std::index_sequence<Is...>, Ts...> : value_overload_<Is, Ts>... { };
template <typename... Ts> using value_overload_set = value_overload_set_<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

template <typename EnableT, typename T, typename... Ts> struct value_overload_init_ { };
template <typename T, typename... Ts>
    struct value_overload_init_<void_t<decltype(value_overload_set<Ts...>{ }(std::declval<T>()))>, T, Ts...>
{
    using type = decltype(value_overload_set<Ts...>{ }(std::declval<T>()));
};

template <typename T, typename... Ts> struct value_overload_type { using type = typename value_overload_init_<void, T, Ts...>::type::type; };
template <typename T, typename... Ts> using value_overload_type_t = typename value_overload_init_<void, T, Ts...>::type::type::type;

template <typename T, typename... Ts> struct value_overload_index : std::integral_constant<std::size_t, value_overload_init_<void, T, Ts...>::type::index> { };
template <typename T, typename... Ts> static constexpr std::size_t value_overload_index_v = value_overload_init_<void, T, Ts...>::type::index;

#ifdef __clang__
 #if __has_builtin(__type_pack_element)
    template <std::size_t I, typename... Ts> struct nth_type_ { using type = __type_pack_element<I, Ts...>; };
 #else // __has_builtin(__type_pack_element)
template <std::size_t I, typename... Ts> using nth_type_ = type_pack_element_<I, Ts...>;
 #endif // __has_builtin(__type_pack_element)
#else // __clang__
    // work around a VC++ bug with decltype() and dependent types
template <std::size_t I, typename... Ts> using nth_type_ = type_pack_element_<I, Ts...>;
#endif


} // namespace detail


inline namespace types
{


    //ᅟ
    // Use as an argument or return type if a type mismatch error is not desired (e.g. because it would be misleading and another error follows anyway),
    // or to enforce the lowest rank in overload resolution (e.g. to define the default behavior for a function in case no matching function is found
    // via argument-dependent lookup).
    //
struct universally_convertible
{
    template <typename T> constexpr universally_convertible(const T&) noexcept { }
    template <typename T> constexpr operator T(void) const;
};

    //ᅟ
    // Use as an argument type to enforce lowest rank in overload resolution (e.g. to define the default behavior for a function in case no matching
    // function is found via argument-dependent lookup).
    //
template <typename T>
    struct convertible_from
{
    T value;
    constexpr convertible_from(const T& _value) : value(_value) { }
};


    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
    //
template <template <typename...> class Z, typename... Ts> struct can_apply : makeshift::detail::can_apply_1_<Z, void, Ts...> { };

    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
    //
template <template <typename...> class Z, typename... Ts> constexpr bool can_apply_v = can_apply<Z, Ts...>::value;


    //ᅟ
    // Applies the type arguments to the given template template, i.e. instantiates `Z<Ts...>` for `SeqT = type_sequence<Ts...>`.
    //
template <template <typename...> class Z, typename SeqT> struct apply;
template <template <typename...> class Z, template <typename...> class SeqT, typename... Ts> struct apply<Z, SeqT<Ts...>> { using type = Z<Ts...>; };

    //ᅟ
    // Applies the type arguments to the given template template, i.e. instantiates `Z<Ts...>` for `SeqT = type_sequence<Ts...>`.
    //
template <template <typename...> class Z, typename SeqT> using apply_t = typename apply<Z, SeqT>::type;


    //ᅟ
    // Type sequence (strictly for compile-time purposes).
    //
template <typename... Ts> struct type_sequence { };


    //ᅟ
    // Return a type sequence that represents the types of the given values.
    //
template <typename... Ts>
    constexpr type_sequence<std::decay_t<Ts>...> make_type_sequence(Ts&&...) noexcept
{
    return { };
}


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
    // Determines the `N`-th value in the variadic sequence.
    //
template <std::size_t N, auto... Vs> struct nth_value : nth_type_t<N, std::integral_constant<decltype(Vs), Vs>...> { };


    //ᅟ
    // Determines the `N`-th value in the variadic sequence.
    //
template <std::size_t N, auto... Vs> constexpr typename nth_value<N, Vs...>::value_type nth_value_v = nth_value<N, Vs...>::value;


    //ᅟ
    // Determines the `N`-th type in the given type sequence.
    //
template <std::size_t N, typename TypeSeqT> struct nth_type_in;
template <std::size_t N, template <typename...> class TypeSeqT, typename... Ts> struct nth_type_in<N, TypeSeqT<Ts...>> : makeshift::detail::nth_type_<N, Ts...> { };

    //ᅟ
    // Determines the `N`-th type in the given type sequence.
    //
template <std::size_t N, typename TypeSeqT> using nth_type_in_t = typename nth_type_in<N, TypeSeqT>::type;


    //ᅟ
    // Determines the index of the type `T` in the given type sequence. If `T` does not appear in the type sequence, the index is `size_t(-1)`.
    //
template <typename T, typename TypeSeqT> struct try_index_of_type_in;
template <typename T, template <typename...> class TypeSeqT, typename... Ts> struct try_index_of_type_in<T, TypeSeqT<Ts...>> : std::integral_constant<std::size_t, makeshift::detail::try_type_pack_index_<T, Ts...>::value> { };

    //ᅟ
    // Determines the index of the type `T` in the given type sequence. If `T` does not appear in the type sequence, the index is `size_t(-1)`.
    //
template <typename T, typename TypeSeqT> constexpr std::size_t try_index_of_type_in_v = try_index_of_type_in<T, TypeSeqT>::value;


    //ᅟ
    // Determines the index of the type `T` in the given type sequence.
    //
template <typename T, typename TypeSeqT> struct index_of_type_in;
template <typename T, template <typename...> class TypeSeqT, typename... Ts> struct index_of_type_in<T, TypeSeqT<Ts...>> : std::integral_constant<std::size_t, makeshift::detail::type_pack_index_<T, Ts...>::value> { };

    //ᅟ
    // Determines the index of the type `T` in the given type sequence.
    //
template <typename T, typename TypeSeqT> constexpr std::size_t index_of_type_in_v = index_of_type_in<T, TypeSeqT>::value;


    //ᅟ
    // Determines the `N`-th value in the given sequence.
    //
template <std::size_t N, typename SeqT> struct nth_value_in;
template <std::size_t N, typename T, template <typename, T> class SeqT, T... Vs> struct nth_value_in<N, SeqT<T, Vs...>> : nth_type_t<N, std::integral_constant<T, Vs>...> { };


    //ᅟ
    // Determines the `N`-th value in the given sequence.
    //
template <std::size_t N, typename SeqT> constexpr typename SeqT::value_type nth_value_in_v = nth_value_in<N, SeqT>::value;


    //ᅟ
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> struct type_sequence_cat : makeshift::detail::type_sequence_cat_<type_sequence<>, Ts...> { };

    //ᅟ
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> using type_sequence_cat_t = typename type_sequence_cat<Ts...>::type;



    //ᅟ
    // Tag argument type compatible with arguments that inherit from at least one of the given tag types, retaining ADL.
    // Requires that the argument type be defined as a tag with `define_tag<>`, or combined from existing tag types with `combine_tags<>`.
    //
template <typename... Ts> using any_tag_of = makeshift::detail::any_tag_tag (&)(Ts...);

    //ᅟ
    // Tag argument type compatible with arguments that inherit from all of the given tag types, retaining ADL.
    // Requires that the argument type be defined as a tag with `define_tag<>`, or combined from existing tag types with `combine_tags<>`.
    //
template <typename... Ts> using all_tags_of = makeshift::detail::all_tags_tag (&)(Ts...);

    //ᅟ
    // Tag argument type compatible with any tag type.
    //
struct any_tag { };


    //ᅟ
    // Use `define_tag<>` to define a tag type. Tag types defined this way can be combined algebraically with `any_tag_of<>` or `all_tags_of<>`.
    //ᅟ
    //ᅟ    struct MyTag : define_tag<MyTag> { };
    //
template <typename TagT, typename... BaseTagsT>
    struct define_tag : BaseTagsT...
{
    template <typename... Ts,
              typename = std::enable_if_t<std::disjunction<std::is_base_of<Ts, TagT>...>::value>>
        constexpr operator any_tag_of<Ts...>(/*void*/) const noexcept
    {
        return makeshift::detail::any_tag_func<Ts...>;
    }
    template <typename... Ts,
              typename = std::enable_if_t<std::conjunction<std::is_base_of<Ts, TagT>...>::value>>
        constexpr operator all_tags_of<Ts...>(/*void*/) const noexcept
    {
        return makeshift::detail::all_tags_func<Ts...>;
    }
    constexpr operator any_tag(/*void*/) const noexcept { return { }; }
};


    //ᅟ
    // Refers to an ad-hoc tag type that inherits from the given list of tag types.
    //
template <typename... TagsT> struct combine_tags : define_tag<combine_tags<TagsT...>, TagsT...> { };

    //ᅟ
    // Refers to an instance of an ad-hoc tag type that inherits from the given list of tag types.
    //
template <typename... TagsT> constexpr combine_tags<TagsT...> combine_tags_v{ };


    //ᅟ
    // Helper for type dispatching.
    //
template <typename T = void> struct tag : define_tag<tag<T>> { using type = T; };

    //ᅟ
    // Helper for type dispatching.
    //
template <typename T = void> constexpr tag<T> tag_v { };


    //ᅟ
    // Helper for type dispatching.
    //
template <template <typename...> class T> struct template_tag : define_tag<template_tag<T>> { };

    //ᅟ
    // Helper for type dispatching.
    //
template <template <typename...> class T> constexpr template_tag<T> template_tag_v{ };


    //ᅟ
    // Encodes a value in a type.
    //
template <auto V, typename = decltype(V)> using constant = std::integral_constant<decltype(V), V>;

    //ᅟ
    // Encodes a value in the type of the expression.
    //
template <auto V, typename = decltype(V)> constexpr constant<V> c{ };


    //ᅟ
    // Encodes a sequence of constants in a type.
    //
template <typename T, T... Vs>
    struct sequence
{
    using value_type = T;

    static constexpr std::size_t size(void) noexcept { return sizeof...(Vs); }

    constexpr sequence(/*void*/) noexcept { }
    template <typename U = T,
              typename = std::enable_if_t<std::is_same<T, U>::value>>
        constexpr sequence(std::integer_sequence<U, Vs...>) noexcept
    {
    }
    constexpr sequence(std::integral_constant<T, Vs>...) noexcept { }
};
template <typename T>
    struct sequence<T>
{
    using value_type = T;

    static constexpr std::size_t size(void) noexcept { return 0; }

    constexpr sequence(/*void*/) noexcept { }
    template <typename U = T,
              typename = std::enable_if_t<std::is_same<T, U>::value>>
        constexpr sequence(std::integer_sequence<U>) noexcept
    {
    }
};
template <typename T, T... Vs>
    sequence(std::integer_sequence<T, Vs...>) -> sequence<T, Vs...>;
template <typename T, T... Vs>
    sequence(std::integral_constant<T, Vs>...) -> sequence<T, Vs...>;


    //ᅟ
    // Encodes the integer range [First, Last) in a type.
    //
template <typename T, T First, T Last>
    struct integer_range
{
    static_assert(Last >= First, "invalid range");

    using value_type = T;

    static constexpr std::size_t size(void) noexcept { return std::size_t(Last - First); }
    static constexpr T first(void) noexcept { return First; }
    static constexpr T last(void) noexcept { return Last; }

    constexpr integer_range(/*void*/) noexcept { }

    // TODO: there is more to do here! (e.g. iterators)
};


    //ᅟ
    // Encodes the integer range [First, Last] in a type.
    //
template <typename T, T First, T Last>
    struct integer_inclusive_range
{
    static_assert(Last >= First, "invalid range");

    using value_type = T;

    static constexpr std::size_t size(void) noexcept { return std::size_t(Last - First + 1); }
    static constexpr T first(void) noexcept { return First; }
    static constexpr T last(void) noexcept { return Last; }

    constexpr integer_inclusive_range(/*void*/) noexcept { }

    // TODO: there is more to do here! (e.g. iterators, sequence conversions)
};


    //ᅟ
    // Encodes the upper-half integer range [First, ...) in a type.
    //
template <typename T, T First>
    struct integer_upper_half_range
{
    using value_type = T;

    static constexpr T first(void) noexcept { return First; }

    constexpr integer_upper_half_range(/*void*/) noexcept { }
};


    //ᅟ
    // Encodes the lower-half integer range (, Last) in a type.
    //
template <typename T, T Last>
    struct integer_lower_half_range
{
    using value_type = T;

    static constexpr T last(void) noexcept { return Last; }

    constexpr integer_lower_half_range(/*void*/) noexcept { }
};


    //ᅟ
    // Encodes the lower-half integer range (, Last] in a type.
    //
template <typename T, T Last>
    struct integer_lower_half_inclusive_range
{
    using value_type = T;

    static constexpr T last(void) noexcept { return Last; }

    constexpr integer_lower_half_inclusive_range(/*void*/) noexcept { }
};


    //ᅟ
    // Null type for tuple functions, flag enums and other purposes.
    //
struct none_t { };

    //ᅟ
    // Null element for tuple functions, flag enums and other purposes.
    //
constexpr none_t none { };


    //ᅟ
    // Determines whether the given type is `none_t`.
    //
template <typename T> struct is_none : std::false_type { };
template <> struct is_none<none_t> : std::true_type { };

    //ᅟ
    // Determines whether the given type is `none_t`.
    //
template <typename T> constexpr bool is_none_v = is_none<T>::value;


    //ᅟ
    // Retrieves the flag type (i.e. the `struct` which inherits from `define_flags<>` and defines flag values) of a flags enum.
    //
template <typename T> struct flag_type_of : decltype(flag_type_of_(std::declval<T>(), makeshift::detail::flags_tag{ })) { };

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
    // Determines whether the given type is a constrained integer.
    //
template <typename T> struct is_constrained_integer : std::is_base_of<makeshift::detail::constrained_integer_base, T> { };

    //ᅟ
    // Determines whether the given type is a constrained integer.
    //
template <typename T> constexpr bool is_constrained_integer_v = is_constrained_integer<T>::value;


    //ᅟ
    // Determines whether a type is a serializer.
    //
template <typename T> struct is_serializer : std::is_base_of<makeshift::detail::serializer_base, T> { };

    //ᅟ
    // Determines whether a type is a serializer.
    //
template <typename T> constexpr bool is_serializer_v = is_serializer<T>::value;


    //ᅟ
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U> struct is_same_template : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_same_template<U<Ts...>, U> : std::true_type { };

    //ᅟ
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U> constexpr bool is_same_template_v = is_same_template<T, U>::value;


    //ᅟ
    // Removes an rvalue reference from a type.
    //
template <typename T> struct remove_rvalue_reference { using type = T; };
template <typename T> struct remove_rvalue_reference<T&&> { using type = T; };

    //ᅟ
    // Removes an rvalue reference from a type.
    //
template <typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class tuple_size<makeshift::type_sequence<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_sequence<Ts...>> : public makeshift::detail::nth_type_<I, Ts...> { };


} // namespace std


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
