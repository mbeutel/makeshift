
#ifndef INCLUDED_MAKESHIFT_DETAIL_ZIP_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ZIP_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <tuple>
#include <utility>     // for tuple_size<> (C++17), forward<>()
#include <type_traits> // for integral_constant<>, declval<>()

#include <gsl-lite/gsl-lite.hpp> // for disjunction<>, gsl_Expects(), ssize(), void_t<>, gsl_CPP17_OR_GREATER

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES, MAKESHIFT_DETAIL_FORCEINLINE
#include <makeshift/detail/type_traits.hpp> // for is_tuple_like<>
#include <makeshift/detail/utility.hpp>     // for any_sink
#include <makeshift/detail/range-index.hpp> // for range_index_t


namespace makeshift {

namespace gsl = ::gsl_lite;

namespace detail {


struct negation_fn
{
    MAKESHIFT_DETAIL_FORCEINLINE constexpr bool operator ()(bool cond) const noexcept
    {
        return !cond;
    }
};

template <typename T>
struct fill_fn
{
    T const& value;

    MAKESHIFT_DETAIL_FORCEINLINE constexpr T const& operator ()(void) const noexcept
    {
        return value;
    }
};


template <gsl::dim N>
using index_constant = std::integral_constant<gsl::index, N>;

template <gsl::dim N>
using dim_constant = std::integral_constant<gsl::dim, N>;


    //
    // Pass `range_index` to `range_*()`, `array_*()`, `template_*()`, or `tuple_*()` algorithms to have the array element index
    // passed as a functor argument. The argument is of type `gsl::index`.
    //ᅟ
    //ᅟ    auto indices = array_transform_n<3>(
    //ᅟ        [](gsl::index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array<gsl::index, 3>{ 0, 1, 2 }
    //
struct range_index_t { };

template <std::size_t I>
MAKESHIFT_DETAIL_FORCEINLINE constexpr std::ptrdiff_t
get(range_index_t) noexcept
{
    return I;
}


    //
    // Pass `tuple_index` to `array_transform()`, `template_for()`, or `tuple_transform()` to have the tuple element index passed
    // as a functor argument. The argument is of type `integral_constant<index, I>`.
    //ᅟ
    //ᅟ        // print all alternatives of a variant
    //ᅟ    constexpr auto numAlternatives = std::variant_size_v<MyVariant>;
    //ᅟ    template_for<numAlternatives>(
    //ᅟ        [](auto idxC)
    //ᅟ        {
    //ᅟ            using T = std::variant_alternative_t<idxC(), MyVariant>;
    //ᅟ            printTypename<T>();
    //ᅟ        });
    //
struct tuple_index_t { };

template <std::size_t I>
MAKESHIFT_DETAIL_FORCEINLINE constexpr index_constant<I>
get(tuple_index_t) noexcept
{
    return { };
}


template <typename R, typename = void> struct has_data : std::false_type { };
template <typename R> struct has_data<R, gsl::void_t<decltype(gsl::data(std::declval<R>()))>> : std::true_type { };

template <typename R, typename = void> struct has_size : std::false_type { };
template <typename R> struct has_size<R, gsl::void_t<decltype(gsl::size(std::declval<R>()))>> : std::true_type { };

enum class iterator_mode
{
    iterator_pair,
    iterator,
    index,
    range_index,
    tuple_element,
    tuple_index
};

/*

range_for                           template_for
                                    template_for_n

                                array_transform     tuple_transform
                                array_transform_n   tuple_transform_n
                                array_fill
                                array_fill_n
                                array_copy
                                array_copy_n

range_transform_reduce              template_transform_reduce
range_reduce                        template_reduce
range_count_if

range_all_of                    template_all_of
range_any_of                    template_any_of
range_none_of                   template_none_of


                                array_cat           [tuple_cat]

range_zip

*/

    // 1-based bit indices
constexpr unsigned input_iterator_bit         = 1;
constexpr unsigned forward_iterator_bit       = 2;
constexpr unsigned bidirectional_iterator_bit = 3;
constexpr unsigned random_access_iterator_bit = 4;
constexpr unsigned input_iterator_tag_bit(any_sink) { return 0; }
constexpr unsigned input_iterator_tag_bit(std::input_iterator_tag) { return input_iterator_bit; }
constexpr unsigned input_iterator_tag_bit(std::forward_iterator_tag) { return forward_iterator_bit; }
constexpr unsigned input_iterator_tag_bit(std::bidirectional_iterator_tag) { return bidirectional_iterator_bit; }
constexpr unsigned input_iterator_tag_bit(std::random_access_iterator_tag) { return random_access_iterator_bit; }

constexpr unsigned output_iterator_bit = 1;
constexpr unsigned output_iterator_tag_bit(any_sink) { return 0; }
constexpr unsigned output_iterator_tag_bit(std::output_iterator_tag) { return output_iterator_bit; }

constexpr unsigned fill_mask(unsigned bit)
{
    return (1 << bit) - 1;
}

constexpr unsigned cbitand(std::initializer_list<unsigned> values)
{
    unsigned result = ~0u;
    for (auto value : values)
    {
        result &= value;
    }
    return result;
}

constexpr unsigned bit_width(unsigned mask)
{
    unsigned bit = 0;
    while (mask != 0)
    {
        ++bit;
        mask >>= 1;
    }
    return bit;
}

template <typename... Ts>
constexpr unsigned common_input_iterator_bit(void)
{
    return detail::bit_width(detail::cbitand({ detail::fill_mask(detail::input_iterator_tag_bit(Ts{ }))... }));
}
template <typename... Ts>
constexpr unsigned common_output_iterator_bit(void)
{
    return detail::bit_width(detail::cbitand({ detail::fill_mask(detail::output_iterator_tag_bit(Ts{ }))... }));
}

template <unsigned MSB> struct input_iterator_tag_base;
template <> struct input_iterator_tag_base<0> { };
template <> struct input_iterator_tag_base<input_iterator_bit> : std::input_iterator_tag { };
template <> struct input_iterator_tag_base<forward_iterator_bit> : std::forward_iterator_tag { };
template <> struct input_iterator_tag_base<bidirectional_iterator_bit> : std::bidirectional_iterator_tag { };
template <> struct input_iterator_tag_base<random_access_iterator_bit> : std::random_access_iterator_tag { };
template <unsigned MSB> struct output_iterator_tag_base;
template <> struct output_iterator_tag_base<0> { };
template <> struct output_iterator_tag_base<output_iterator_bit> : std::output_iterator_tag { };

template <typename... TagsT>
struct common_iterator_tag
    : input_iterator_tag_base<detail::common_input_iterator_bit<TagsT...>()>,
      output_iterator_tag_base<detail::common_output_iterator_bit<TagsT...>()>
{
};

template <typename R>
constexpr decltype(auto) range_begin(R&& range)
{
    using std::begin;
    return begin(range);
}
template <typename R>
constexpr decltype(auto) range_end(R&& range)
{
    using std::end;
    return end(range);
}

template <typename R> struct range_iterator_category_{ using type = typename std::iterator_traits<decltype(detail::range_begin(std::declval<R>()))>::iterator_category; };
template <> struct range_iterator_category_<range_index_t> { using type = std::random_access_iterator_tag; };
template <> struct range_iterator_category_<tuple_index_t> { using type = std::input_iterator_tag; }; // TODO: ?
template <typename R> using range_iterator_category_t = typename range_iterator_category_<R>::type;

#ifdef __cpp_concepts
template <typename R> struct range_iterator_concept_{ using type = typename std::iterator_traits<decltype(detail::range_begin(std::declval<R>()))>::iterator_concept; };
template <> struct range_iterator_concept_<range_index_t> : range_iterator_category_<range_index_t> { };
template <> struct range_iterator_concept_<tuple_index_t> : range_iterator_category_<tuple_index_t> { };
template <typename R> using range_iterator_concept_t = typename range_iterator_concept_<R>::type;
#endif // __cpp_concepts

template <bool HasData, bool HasSize> struct range_iterator_leaf_mode_0_;
template <> struct range_iterator_leaf_mode_0_<false, false> : std::integral_constant<iterator_mode, iterator_mode::iterator_pair> { };
template <> struct range_iterator_leaf_mode_0_<false, true> : std::integral_constant<iterator_mode, iterator_mode::iterator> { };
template <> struct range_iterator_leaf_mode_0_<true, true> : std::integral_constant<iterator_mode, iterator_mode::index> { };
template <typename R> struct range_iterator_leaf_mode_ : range_iterator_leaf_mode_0_<has_data<R>::value, has_size<R>::value> { };
template <> struct range_iterator_leaf_mode_<range_index_t> : std::integral_constant<iterator_mode, iterator_mode::range_index> { };
template <> struct range_iterator_leaf_mode_<tuple_index_t> : std::integral_constant<iterator_mode, iterator_mode::tuple_index> { };

template <typename R, typename = void> struct tuple_iterator_leaf_mode_ : range_iterator_leaf_mode_<R> { };
template <typename R> struct tuple_iterator_leaf_mode_<R, gsl::void_t<decltype(std::tuple_size<R>::value)>> : std::integral_constant<iterator_mode, iterator_mode::tuple_element> { };

struct nullopt_bool { };

MAKESHIFT_DETAIL_FORCEINLINE constexpr nullopt_bool is_end(void) noexcept
{
    return { };
}
MAKESHIFT_DETAIL_FORCEINLINE constexpr nullopt_bool is_end(nullopt_bool) noexcept
{
    return { };
}
MAKESHIFT_DETAIL_FORCEINLINE constexpr bool is_end(bool isEnd) noexcept
{
    return isEnd;
}
MAKESHIFT_DETAIL_FORCEINLINE constexpr bool is_end(bool isEnd1, bool isEnd2)
{
    gsl_Expects(isEnd1 == isEnd2);
    return isEnd1;
}
MAKESHIFT_DETAIL_FORCEINLINE constexpr bool is_end(bool isEnd, nullopt_bool) noexcept
{
    return isEnd;
}
MAKESHIFT_DETAIL_FORCEINLINE constexpr bool is_end(nullopt_bool, bool isEnd) noexcept
{
    return isEnd;
}
MAKESHIFT_DETAIL_FORCEINLINE constexpr nullopt_bool is_end(nullopt_bool, nullopt_bool) noexcept
{
    return { };
}
template <typename E1, typename E2, typename... Es>
constexpr bool is_end(E1 e1, E2 e2, Es... es)
{
    return detail::is_end(detail::is_end(e1, e2), detail::is_end(es...));
}

struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_defaults
{
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _inc(void) noexcept
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _dec(void) noexcept
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _advance(std::ptrdiff_t) noexcept
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _check_end(bool /*isEnd*/) const noexcept
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr nullopt_bool _is_end(void) const noexcept
    {
        return { };
    }
};
template <std::size_t I, typename R, iterator_mode IteratorMode>
struct zip_iterator_leaf_base;
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::iterator_pair>
{
    using iterator = decltype(detail::range_begin(std::declval<R>()));
    using sentinel = decltype(detail::range_end(std::declval<R>()));
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using reference = typename std::iterator_traits<iterator>::reference;

    iterator pos;
    sentinel end;

    constexpr zip_iterator_leaf_base(R& range)
        : pos(detail::range_begin(range)), end(detail::range_end(range))
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _inc(void)
    {
        ++pos;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _dec(void)
    {
        --pos;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _advance(std::ptrdiff_t d)
    {
        pos += d;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index) const
    {
        return *pos;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index, std::ptrdiff_t d) const
    {
        return pos[d];
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _check_end(bool isEnd) const
    {
        gsl_Expects((pos == end) == isEnd);
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr bool _is_end(void) const
    {
        return pos == end;
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::iterator> : zip_iterator_defaults
{
    using iterator = decltype(detail::range_begin(std::declval<R>()));
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using reference = typename std::iterator_traits<iterator>::reference;

    iterator pos;

    constexpr zip_iterator_leaf_base(R& range)
        : pos(detail::range_begin(range))
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _inc(void)
    {
        ++pos;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _dec(void)
    {
        --pos;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr void _advance(std::ptrdiff_t d)
    {
        pos += d;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index) const
    {
        return *pos;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index, std::ptrdiff_t d) const
    {
        return pos[d];
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::index> : zip_iterator_defaults
{
    using pointer = decltype(std::declval<R>().data());
    using value_type = std::remove_pointer_t<pointer>;
    using reference = value_type&;

    pointer data;

    constexpr zip_iterator_leaf_base(R& range)
        : data(range.data())
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index i) const
    {
        return data[i];
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index i, std::ptrdiff_t d) const
    {
        return data[i + d];
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::range_index> : zip_iterator_defaults
{
    using value_type = gsl::index;
    using reference = gsl::index;

    constexpr zip_iterator_leaf_base(range_index_t)
    {
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index i) const noexcept
    {
        return i;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr reference _deref(gsl::index i, std::ptrdiff_t d) const
    {
        return i + d;
    }
};
template <std::size_t I, typename T>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, T, iterator_mode::tuple_element> : zip_iterator_defaults
{
    T&& tuple;

    constexpr zip_iterator_leaf_base(T&& _tuple)
        : tuple(_tuple)
    {
    }
    template <gsl::index Index>
    MAKESHIFT_DETAIL_FORCEINLINE constexpr decltype(auto) _deref(index_constant<Index>) const
    {
        using std::get;
        return get<Index>(std::forward<T>(tuple));
    }
};
template <std::size_t I, typename T>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, T, iterator_mode::tuple_index> : zip_iterator_defaults
{
    constexpr zip_iterator_leaf_base(tuple_index_t)
    {
    }
    template <gsl::index Index>
    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_constant<Index> _deref(index_constant<Index>) const noexcept
    {
        return { };
    }
};


constexpr gsl::dim unknown_size = -1;

template <typename T> struct static_range_size_type_ : std::false_type { };
template <> struct static_range_size_type_<dim_constant<unknown_size>> : std::false_type { };
template <gsl::dim N> struct static_range_size_type_<dim_constant<N>> : std::true_type { using range_size_type = dim_constant<N>; };
template <typename T> struct dynamic_range_size_type_ : std::false_type { };
template <> struct dynamic_range_size_type_<gsl::dim> : std::true_type { using range_size_type = gsl::dim; };
struct default_range_size_type_ : std::true_type { using range_size_type = dim_constant<unknown_size>; };
template <typename... Ts> using range_size_type = typename gsl::disjunction<static_range_size_type_<Ts>..., dynamic_range_size_type_<Ts>..., default_range_size_type_>::range_size_type;

constexpr gsl::dim merge_sizes_0(dim_constant<unknown_size>, gsl::dim n)
{
    return n;
}
template <gsl::dim N>
constexpr dim_constant<N> merge_sizes_0(dim_constant<N>, gsl::dim n)
{
    gsl_Expects(n == N || n == unknown_size);
    return { };
}
template <gsl::dim N, gsl::dim M>
constexpr dim_constant<(N == unknown_size ? M : N)> merge_sizes_0(dim_constant<N>, dim_constant<M>)
{
    static_assert(N == unknown_size || M == unknown_size || N == M, "inconsistent sizes");
    return { };
}
template <gsl::dim N>
constexpr auto merge_sizes_0(gsl::dim n, dim_constant<N>)
{
    return detail::merge_sizes_0(dim_constant<N>{ }, n);
}
constexpr auto merge_sizes_0(gsl::dim n, gsl::dim m)
{
    if (n == unknown_size || m == unknown_size) return unknown_size;
    gsl_Expects(n == m);
    return n;
}

constexpr dim_constant<unknown_size>
merge_sizes(void)
{
    return { };
}
template <typename T>
constexpr T
merge_sizes(T n)
{
    return n;
}
template <gsl::dim Size1, typename... Ts>
constexpr range_size_type<dim_constant<Size1>, gsl::dim, Ts...>
merge_sizes(dim_constant<Size1> size1, gsl::dim size2, Ts... sizes);
template <gsl::dim Size2, typename... Ts>
constexpr range_size_type<gsl::dim, dim_constant<Size2>, Ts...>
merge_sizes(gsl::dim size1, dim_constant<Size2> size2, Ts... sizes);
template <typename... Ts>
constexpr range_size_type<gsl::dim, Ts...>
merge_sizes(gsl::dim size1, gsl::dim size2, Ts... sizes);
template <gsl::dim Size1, gsl::dim Size2, typename... Ts>
constexpr range_size_type<dim_constant<Size1>, dim_constant<Size2>, Ts...>
merge_sizes(dim_constant<Size1> size1, dim_constant<Size2> size2, Ts... sizes)
{
    return detail::merge_sizes(detail::merge_sizes_0(size1, size2), sizes...);
}
template <gsl::dim Size1, typename... Ts>
constexpr range_size_type<dim_constant<Size1>, gsl::dim, Ts...>
merge_sizes(dim_constant<Size1> size1, gsl::dim size2, Ts... sizes)
{
    return detail::merge_sizes_0(size1, detail::merge_sizes(sizes..., size2));
}
template <gsl::dim Size2, typename... Ts>
constexpr range_size_type<gsl::dim, dim_constant<Size2>, Ts...>
merge_sizes(gsl::dim size1, dim_constant<Size2> size2, Ts... sizes)
{
    return detail::merge_sizes_0(size2, detail::merge_sizes(sizes..., size1));
}
template <typename... Ts>
constexpr range_size_type<gsl::dim, Ts...>
merge_sizes(gsl::dim size1, gsl::dim size2, Ts... sizes)
{
    return detail::merge_sizes_0(detail::merge_sizes(sizes..., size1), size2);
}

template <typename R>
constexpr gsl::dim range_size_1(std::true_type /*hasSize*/, R const& range) noexcept
{
    return gsl::ssize(range);
}
template <typename R>
constexpr dim_constant<unknown_size> range_size_1(std::false_type /*hasSize*/, R const&) noexcept
{
    return { };
}
template <typename R>
constexpr dim_constant<std::tuple_size<R>::value> range_size_0(std::true_type /*isTupleLike*/, R const&) noexcept
{
    return { };
}
template <typename R>
constexpr auto range_size_0(std::false_type /*isTupleLike*/, R const& range) noexcept
{
    return detail::range_size_1(has_size<R>{ }, range);
}
template <typename R>
constexpr auto range_size(R const& range) noexcept
{
    return detail::range_size_0(is_tuple_like<R>{ }, range);
}
constexpr dim_constant<unknown_size> range_size(range_index_t) noexcept
{
    return { };
}


} // namespace detail


    //
    // Pass `range_index` to `range_*()`, `array_*()`, `template_*()`, or `tuple_*()` algorithms to have the array element index
    // passed as a functor argument. The argument is of type `gsl::index`.
    //ᅟ
    //ᅟ    auto indices = array_transform_n<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array<gsl::index, 3>{ 0, 1, 2 }
    //
static constexpr detail::range_index_t const&
range_index = static_const<detail::range_index_t>;


    // Pass `tuple_index` to `array_transform()`, `template_for()`, or `tuple_transform()` to have the tuple element index passed
    // as a functor argument. The argument is of type `integral_constant<index, I>`.
    //ᅟ
    //ᅟ        // print all alternatives of a variant
    //ᅟ    constexpr auto numAlternatives = std::variant_size_v<MyVariant>;
    //ᅟ    template_for<numAlternatives>(
    //ᅟ        [](auto idxC)
    //ᅟ        {
    //ᅟ            using T = std::variant_alternative_t<idxC(), MyVariant>;
    //ᅟ            printTypename<T>();
    //ᅟ        });
    //
static constexpr detail::tuple_index_t const&
tuple_index = static_const<detail::tuple_index_t>;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ZIP_HPP_
