
#ifndef MAKESHIFT_TYPE_TRAITS_HPP_
#define MAKESHIFT_TYPE_TRAITS_HPP_


#include <type_traits> // for integral_constant<>, declval<>()
#include <utility>     // for integer_sequence<>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Represents an index value.
    //
using index_t = std::ptrdiff_t;


    //ᅟ
    // Represents an array stride.
    //
using stride_t = std::ptrdiff_t;


    //ᅟ
    // Represents a dimension value.
    //
using dim_t = std::ptrdiff_t;


    //ᅟ
    // Encodes a sequence of index values in a type.
    //
template <index_t... Is> struct index_constant : std::integer_sequence<index_t, Is...> { };


    //ᅟ
    // Encodes a sequence of index values in the type of the expression.
    //
template <index_t... Is> constexpr index_constant<Is...> index{ };


    //ᅟ
    // Encodes a sequence of dimension values in a type.
    //
template <dim_t... Ds> struct shape_constant : std::integer_sequence<dim_t, Ds...> { };


    //ᅟ
    // Encodes a sequence of dimension values in the type of the expression.
    //
template <dim_t... Ds> constexpr shape_constant<Ds...> shape{ };


    //ᅟ
    // Encodes a dimension value in a type.
    //
template <dim_t D> using dim_constant = shape_constant<D>;


    //ᅟ
    // Encodes a dimension value in the type of the expression.
    //
template <dim_t D> constexpr dim_constant<D> dim{ };





} // inline namespace types


namespace detail
{


struct flags_base { };
struct flags_tag { };

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
    using type = T;
};
template <typename IsT, typename... Ts> struct type_pack_indexer;
template <std::size_t... Is, typename... Ts> struct type_pack_indexer<std::index_sequence<Is...>, Ts...> : type_pack_index_base<Is, Ts>... { };
template <std::size_t I, typename T>
    static type_pack_index_base<I, T> select_type_seq_entry(type_pack_index_base<I, T>);

template <std::size_t I, typename... Ts>
    struct type_pack_element_
{
    using index_base = decltype(makeshift::detail::select_type_seq_entry<I>(type_pack_indexer<std::make_index_sequence<sizeof...(Ts)>, Ts...>{ }));
    using type = typename index_base::type;
};


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


constexpr inline std::ptrdiff_t cpow10(int I, int N) noexcept
{
    std::ptrdiff_t result = 1;
    for (int n = 0; n != N - 1 - I; ++n)
        result *= 10;
    return result;
}
template <typename... Ts>
    constexpr inline std::ptrdiff_t csum(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    auto term = std::ptrdiff_t(0);
    return (vs + ... + term);
}
template <typename... Ts>
    constexpr inline bool cand(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    return (vs && ...);
}
template <typename Is, char... Cs>
    struct make_index_constant_;
template <std::size_t... Is, char... Cs>
    struct make_index_constant_<std::index_sequence<Is...>, Cs...>
{
    static_assert(cand(Cs >= '0' && Cs <= '9'...), "invalid character: index must be an integral value");
    static constexpr std::ptrdiff_t value = csum((Cs - '0') * cpow10(Is, sizeof...(Cs))...);
};
template <char... Cs> struct make_index_constant : make_index_constant_<std::make_index_sequence<sizeof...(Cs)>, Cs...> { };
template <char... Cs> constexpr std::ptrdiff_t make_index_constant_v = make_index_constant<Cs...>::value;


template <auto I, auto V> using substitute = std::integral_constant<decltype(V), V>;
template <typename Is> struct zero_index_0_;
template <std::size_t... Is> struct zero_index_0_<std::index_sequence<Is...>> { using type = index_constant<substitute<Is, std::ptrdiff_t(0)>::value...>; };
template <dim_t Dim> struct zero_index_ : zero_index_0_<std::make_index_sequence<Dim>> { };


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
    // Helper for type dispatching.
    //
template <typename T = void> struct tag_t { using type = T; };

    //ᅟ
    // Helper for type dispatching.
    //
template <typename T = void> constexpr tag_t<T> tag { };


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
template <typename T> struct flag_type_of : decltype(flag_type_of_(std::declval<T>(), std::declval<makeshift::detail::flags_tag>())) { };

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


    //ᅟ
    // A multi-index of dimension `Dim` with zero-valued entries.
    //ᅟ
    //ᅟ    using I0 = zero_index_t<3>; // I0 is index_constant<0, 0, 0>
    //
template <dim_t Dim> using zero_index_t = typename makeshift::detail::zero_index_<Dim>::type;

    //ᅟ
    // Constructs a multi-index of dimension `Dim` with zero-valued entries.
    //ᅟ
    //ᅟ    auto i0 = zero_index<3>; // decltype(i0) is index_constant<0, 0, 0>
    //
template <dim_t Dim> constexpr zero_index_t<Dim> zero_index{ };


} // inline namespace types


inline namespace literals
{


    //ᅟ
    // Encodes an index value given as numeric literal in the type of the expression using `index_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_idx; // decltype(i) is index_constant<42>
    //
template <char... Cs>
    constexpr inline index_constant<makeshift::detail::make_index_constant<Cs...>::value>
    operator "" _idx(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes a dimension value given as numeric literal in the type of the expression using `dim_constant<>`.
    //ᅟ
    //ᅟ    auto d = 3_dim; // decltype(i) is dim_constant<3>
    //
template <char... Cs>
    constexpr inline dim_constant<makeshift::detail::make_index_constant<Cs...>::value>
    operator "" _dim(void) noexcept
{
    return { };
}


} // inline namespace literals

} // namespace makeshift


#endif // MAKESHIFT_TYPE_TRAITS_HPP_
