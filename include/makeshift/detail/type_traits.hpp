
#ifndef INCLUDED_MAKESHIFT_DETAIL_TYPE_TRAITS_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TYPE_TRAITS_HPP_


#include <makeshift/macros.hpp> // for MAKESHIFT_CXX

#if MAKESHIFT_CXX < 17
 #include <tuple>      // tuple_size<>
#endif // MAKESHIFT_CXX < 17

#include <cstddef>     // for size_t
#include <iterator>    // for begin(), end()
#include <utility>     // for integer_sequence<>, tuple_size<> (C++17)
#include <type_traits> // for declval<>(), integral_constant<>, is_convertible<>, void_t<>


namespace makeshift
{

namespace detail
{

// TODO: remove?
namespace is_iterable_ns
{


using std::begin;
using std::end;


template <typename T> using is_iterable_r = decltype(begin(std::declval<T&>()) != end(std::declval<T&>()));


} // is_iterable_ns


template <bool V0, typename T0, typename... Ts> struct disjunction_ { using type = T0; };
template <typename T0, typename T1, typename... Ts> struct disjunction_<false, T0, T1, Ts...> : disjunction_<T1::value, T1, Ts...> { };
template <bool V0, typename T0, typename... Ts> struct conjunction_ { using type = T0; };
template <typename T0, typename T1, typename... Ts> struct conjunction_<true, T0, T1, Ts...> : conjunction_<T1::value, T1, Ts...> { };
template <typename... Ts> struct disjunction;
template <> struct disjunction<> : std::false_type { };
template <typename T0, typename... Ts> struct disjunction<T0, Ts...> : makeshift::detail::disjunction_<T0::value, T0, Ts...>::type { };
template <typename... Ts> struct conjunction;
template <> struct conjunction<> : std::true_type { };
template <typename T0, typename... Ts> struct conjunction<T0, Ts...> : makeshift::detail::conjunction_<T0::value, T0, Ts...>::type { };


struct type_enum_base { };

struct unwrap_enum_tag { };


template <template <typename...> class, typename, typename...> struct can_instantiate_ : std::false_type { };
#if MAKESHIFT_CXX < 17
    // Cf. https://en.cppreference.com/w/cpp/types/void_t
template<typename... Ts> struct make_void { using type = void; };
template<typename... Ts> using void_t = typename make_void<Ts...>::type;
template <template <typename...> class Z, typename... Ts> struct can_instantiate_<Z, void_t<Z<Ts...>>, Ts...> : std::true_type { };
#else // MAKESHIFT_CXX < 17
template<typename... Ts> using void_t = std::void_t<Ts...>;
template <template <typename...> class Z, typename... Ts> struct can_instantiate_<Z, std::void_t<Z<Ts...>>, Ts...> : std::true_type { };
#endif // MAKESHIFT_CXX < 17


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


#ifdef __clang__
 #if __has_builtin(__type_pack_element)
  #define MAKESHIFT_BUILTIN_TYPE_PACK_ELEMENT_
 #endif // __has_builtin(__type_pack_element)
#endif // __clang__

#ifdef MAKESHIFT_BUILTIN_TYPE_PACK_ELEMENT_
template <std::size_t I, typename... Ts> struct nth_type_ { using type = __type_pack_element<I, Ts...>; };
#else // MAKESHIFT_BUILTIN_TYPE_PACK_ELEMENT_
    // work around a VC++ bug with decltype() and dependent types
template <std::size_t I, typename... Ts> using nth_type_ = type_pack_element_<I, Ts...>;
#endif // MAKESHIFT_BUILTIN_TYPE_PACK_ELEMENT_

#undef MAKESHIFT_BUILTIN_TYPE_PACK_ELEMENT_

template <typename T, typename... Ts> using try_index_of_type = std::integral_constant<std::size_t, makeshift::detail::try_type_pack_index_<T, Ts...>::value>;

template <template <typename...> class Z, typename SeqT> struct instantiate_;
template <template <typename...> class Z, template <typename...> class SeqT, typename... Ts> struct instantiate_<Z, SeqT<Ts...>> { using type = Z<Ts...>; };


template <typename T> struct type_t;

template <typename T> struct type_set_leaf_ { };
template <typename T, std::size_t I> struct type_set_indexed_leaf_ : type_set_leaf_<T> { };
template <typename Is, typename... Ts> struct type_set_0_;
template <std::size_t... Is, typename... Ts> struct type_set_0_<std::index_sequence<Is...>, Ts...> : type_set_indexed_leaf_<Ts, Is>... { };
template <typename... Ts> struct type_set_ : type_set_0_<std::index_sequence_for<Ts...>, Ts...> { };
template <typename T, typename... Ts> struct is_in_ : std::is_base_of<type_set_leaf_<T>, type_set_<Ts...>> { };

template <typename Rs, typename Ts> struct unique_sequence_0_;
template <typename Rs> struct unique_sequence_0_<Rs, type_set_<>> { using type = Rs; };
template <typename Rs, bool IsIn, typename T0, typename Ts> struct unique_sequence_1_;
template <typename Rs, typename T0, typename Ts> struct unique_sequence_1_<Rs, true, T0, Ts> : unique_sequence_0_<Rs, Ts> { };
template <template <typename...> class TypeSeqT, typename... Rs, typename T0, typename Ts> struct unique_sequence_1_<TypeSeqT<Rs...>, false, T0, Ts> : unique_sequence_0_<TypeSeqT<Rs..., T0>, Ts> { };
template <typename Rs, typename T0, typename... Ts> struct unique_sequence_0_<Rs, type_set_<T0, Ts...>> : unique_sequence_1_<Rs, is_in_<T0, Ts...>::value, T0, type_set_<Ts...>> { };

template <typename Ts> struct unique_sequence_;
template <template <typename...> class TypeSeqT, typename... Ts> struct unique_sequence_<TypeSeqT<Ts...>> : unique_sequence_0_<TypeSeqT<>, type_set_<Ts...>> { };


    //ᅟ
    // Tag argument type compatible with arguments that inherit from at least one of the given tag types.
    //
template <typename... Ts>
    struct any_tag_of
{
    template <typename T,
              std::enable_if_t<disjunction<std::is_convertible<T, Ts>...>::value, int> = 0>
        constexpr any_tag_of(const T&) noexcept
    {
    }
};


template <typename... Ts> struct equal_types_;
template <> struct equal_types_<> : std::false_type { }; // we opt for false because then we don't have to name the common type
template <typename T> struct equal_types_<T> : std::true_type { using common_type = T; };
template <typename T0, typename T1, typename... Ts> struct equal_types_<T0, T1, Ts...> : std::false_type { };
template <typename T01, typename... Ts> struct equal_types_<T01, T01, Ts...> : equal_types_<T01, Ts...> { };


struct constval_tag { };

template <typename C> struct is_integral_constant_ : std::false_type { };
template <typename T, T V> struct is_integral_constant_<std::integral_constant<T, V>> : std::true_type { };

template <typename T> struct is_constval_ : disjunction<std::is_base_of<constval_tag, T>, is_integral_constant_<T>> { };

template <typename C, typename R> struct is_nullary_functor_of_type_ : std::is_same<R, decltype(std::declval<C>()())> { };
template <typename T, typename R> struct is_constval_of_type_ : conjunction<is_constval_<T>, is_nullary_functor_of_type_<T, R>> { };


template <typename T> struct as_dependent_type_ { using type = T; };


template <typename T> using is_tuple_like_r = decltype(std::tuple_size<T>::value);
template <typename T> struct is_tuple_like : can_instantiate_<is_tuple_like_r, void, T> { };


template <typename T, typename V1, typename V2, typename V3, typename V4, typename R1, typename R2, typename R3> struct is_bitmask_type_result : std::false_type { };
template <typename T> struct is_bitmask_type_result<T, T, T, T, T, T&, T&, T&> : std::true_type { };

template <typename T> using is_bitmask_type_r = is_bitmask_type_result<T,
    decltype(~std::declval<T const&>()),
    decltype(std::declval<T const&>() | std::declval<T const&>()),
    decltype(std::declval<T const&>() & std::declval<T const&>()),
    decltype(std::declval<T const&>() ^ std::declval<T const&>()),
    decltype(std::declval<T&>() |= std::declval<T const&>()),
    decltype(std::declval<T&>() &= std::declval<T const&>()),
    decltype(std::declval<T&>() ^= std::declval<T const&>())>;

template <typename T> struct is_bitmask_type_0_ : is_bitmask_type_r<T> { };
template <typename T> using is_bitmask_type = conjunction<can_instantiate_<is_bitmask_type_r, void, T>, is_bitmask_type_0_<T>>;

} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TYPE_TRAITS_HPP_
