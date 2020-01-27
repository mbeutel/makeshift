
#ifndef INCLUDED_MAKESHIFT_DETAIL_ZIP_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ZIP_HPP_


#include <gsl-lite/gsl-lite.hpp> // for disjunction<>, gsl_Expects(), ssize(), void_t<>, gsl_CPP17_OR_GREATER


#include <cstddef>     // for size_t, ptrdiff_t
#include <tuple>
#include <utility>     // for tuple_size<> (C++17), forward<>()
#include <type_traits> // for integral_constant<>, declval<>()

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/type_traits.hpp> // for is_tuple_like<>
#include <makeshift/detail/range-index.hpp> // for range_index_t


namespace makeshift
{


namespace gsl = ::gsl_lite;


namespace detail
{


struct negation_fn
{
    constexpr bool operator ()(bool cond) const
    {
        return !cond;
    }
};

template <typename T>
struct fill_fn
{
    T const& value;

    constexpr T const& operator ()(void) const noexcept
    {
        return value;
    }
};


template <gsl::dim N> using index_constant = std::integral_constant<gsl::index, N>;
template <gsl::dim N> using dim_constant = std::integral_constant<gsl::dim, N>;


    //
    // Pass `range_index` to `range_*()`, `array_*()`, `template_*()`, or `tuple_*()` algorithms to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform_n<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
    //
struct range_index_t { };

template <std::size_t I>
constexpr std::ptrdiff_t get(range_index_t) noexcept
{
    return I;
}


    //
    // Pass `tuple_index` to `array_transform()`, `template_for()`, or `tuple_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<index, I>`.
    //ᅟ
    //ᅟ        // print all alternatives of a variant
    //ᅟ    constexpr auto numAlternatives = std::variant_size_v<MyVariant>;
    //ᅟ    template_for_n<numAlternatives>(
    //ᅟ        [](auto idxC)
    //ᅟ        {
    //ᅟ            using T = std::variant_alternative_t<idxC(), MyVariant>;
    //ᅟ            printTypename<T>();
    //ᅟ        });
    //
struct tuple_index_t { };

template <std::size_t I>
constexpr index_constant<I> get(tuple_index_t) noexcept
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
    tuple_element,
    range_index,
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

constexpr bool is_end(bool isEnd) noexcept
{
    return isEnd;
}
constexpr bool is_end(bool isEnd1, bool isEnd2)
{
    gsl_Expects(isEnd1 == isEnd2);
    return isEnd1;
}
constexpr bool is_end(bool isEnd, nullopt_bool) noexcept
{
    return isEnd;
}
constexpr bool is_end(nullopt_bool, bool isEnd) noexcept
{
    return isEnd;
}
constexpr nullopt_bool is_end(nullopt_bool, nullopt_bool) noexcept
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
    constexpr void advance(void)
    {
    }
    constexpr void check_end(bool /*isEnd*/) const noexcept
    {
    }
    constexpr nullopt_bool is_end(void) const noexcept
    {
        return { };
    }
};
template <std::size_t I, typename R, iterator_mode IteratorMode>
struct zip_iterator_leaf_base;
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::iterator_pair>
{
    using iterator = decltype(std::declval<R>().begin());

    iterator pos;
    iterator end;

    constexpr zip_iterator_leaf_base(R& range)
        : pos(range.begin()), end(range.end())
    {
    }
    constexpr void advance(void)
    {
        ++pos;
    }
    constexpr decltype(auto) operator [](gsl::index)
    {
        return *pos;
    }
    constexpr void check_end(bool isEnd) const
    {
        gsl_Expects((pos == end) == isEnd);
    }
    constexpr bool is_end(void) const
    {
        return pos == end;
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::iterator> : zip_iterator_defaults
{
    using iterator = decltype(std::declval<R>().begin());

    iterator pos;

    constexpr zip_iterator_leaf_base(R& range)
        : pos(range.begin())
    {
    }
    constexpr void advance(void)
    {
        ++pos;
    }
    constexpr decltype(auto) operator [](gsl::index)
    {
        return *pos;
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::index> : zip_iterator_defaults
{
    using pointer = decltype(std::declval<R>().data());

    pointer data;

    constexpr zip_iterator_leaf_base(R& range)
        : data(range.data())
    {
    }
    constexpr decltype(auto) operator [](gsl::index index) noexcept
    {
        return data[index];
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
    constexpr decltype(auto) operator [](index_constant<Index>) noexcept
    {
        using std::get;
        return get<Index>(std::forward<T>(tuple));
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::range_index> : zip_iterator_defaults
{
    constexpr zip_iterator_leaf_base(range_index_t)
    {
    }
    constexpr gsl::index operator [](gsl::index index) noexcept
    {
        return index;
    }
};
template <std::size_t I, typename T>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, T, iterator_mode::tuple_index> : zip_iterator_defaults
{
    constexpr zip_iterator_leaf_base(tuple_index_t)
    {
    }
    template <gsl::index Index>
    constexpr index_constant<Index> operator [](index_constant<Index>) noexcept
    {
        return { };
    }
};


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

constexpr dim_constant<unknown_size> merge_sizes(void)
{
    return { };
}
template <typename T>
constexpr T merge_sizes(T n)
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
    // Pass `range_index` to `range_*()`, `array_*()`, `template_*()`, or `tuple_*()` algorithms to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform_n<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
    //
static constexpr detail::range_index_t const& range_index = static_const<detail::range_index_t>;


    //
    // Pass `tuple_index` to `array_transform()`, `template_for()`, or `tuple_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<index, I>`.
    //ᅟ
    //ᅟ        // print all alternatives of a variant
    //ᅟ    constexpr auto numAlternatives = std::variant_size_v<MyVariant>;
    //ᅟ    template_for_n<numAlternatives>(
    //ᅟ        [](auto idxC)
    //ᅟ        {
    //ᅟ            using T = std::variant_alternative_t<idxC(), MyVariant>;
    //ᅟ            printTypename<T>();
    //ᅟ        });
    //
static constexpr detail::tuple_index_t const& tuple_index = static_const<detail::tuple_index_t>;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ZIP_HPP_
