
#ifndef INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_


#include <gsl-lite/gsl-lite.hpp> // for disjunction<>, gsl_Expects(), ssize(), void_t<>, gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# include <tuple>      // for tuple_size<>
#endif // !gsl_CPP17_OR_GREATER

#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for tuple_size<> (C++17), move(), forward<>(), swap(), integer_sequence<>
#include <type_traits> // for integral_constant<>, declval<>(), decay<>

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/type_traits.hpp> // for is_tuple_like<>
#include <makeshift/detail/range-index.hpp> // for range_index_t


namespace makeshift
{


namespace gsl = ::gsl_lite;


namespace detail
{


template <typename R, typename = void> struct has_data : std::false_type { };
template <typename R> struct has_data<R, gsl::void_t<decltype(gsl::data(std::declval<R>()))>> : std::true_type { };

template <typename R, typename = void> struct has_size : std::false_type { };
template <typename R> struct has_size<R, gsl::void_t<decltype(gsl::size(std::declval<R>()))>> : std::true_type { };

enum class iterator_mode
{
    iterator_pair,
    iterator,
    index,
    range_index
};

template <bool HasData, bool HasSize> struct range_iterator_leaf_mode_0_;
template <> struct range_iterator_leaf_mode_0_<false, false> : std::integral_constant<iterator_mode, iterator_mode::iterator_pair> { };
template <> struct range_iterator_leaf_mode_0_<false, true> : std::integral_constant<iterator_mode, iterator_mode::iterator> { };
template <> struct range_iterator_leaf_mode_0_<true, true> : std::integral_constant<iterator_mode, iterator_mode::index> { };
template <typename R> struct range_iterator_leaf_mode_ : range_iterator_leaf_mode_0_<has_data<R>::value, has_size<R>::value> { };
template <> struct range_iterator_leaf_mode_<range_index_t> : std::integral_constant<iterator_mode, iterator_mode::range_index> { };

template <std::size_t I, typename R, iterator_mode IteratorMode>
struct range_iterator_leaf_base;
template <std::size_t I, typename R>
struct range_iterator_leaf_base<I, R, iterator_mode::iterator_pair>
{
    using iterator = decltype(std::declval<R>().begin());

    iterator pos;
    iterator end;

    constexpr range_iterator_leaf_base(R& range)
        : pos(range.begin()), end(range.end())
    {
    }
    constexpr range_iterator_leaf_base& operator ++(void)
    {
        ++pos;
        return *this;
    }
    constexpr decltype(auto) operator [](std::ptrdiff_t /*index*/)
    {
        return *pos;
    }
    constexpr void is_end(bool& isNotEnd, bool& isEnd) const
    {
        if (pos != end) isNotEnd = true;
        else isEnd = true;
    }
};
template <std::size_t I, typename R>
struct range_iterator_leaf_base<I, R, iterator_mode::iterator>
{
    using iterator = decltype(std::declval<R>().begin());

    iterator pos;

    constexpr range_iterator_leaf_base(R& range)
        : pos(range.begin())
    {
    }
    constexpr range_iterator_leaf_base& operator ++(void)
    {
        ++pos;
        return *this;
    }
    constexpr decltype(auto) operator [](std::ptrdiff_t /*index*/)
    {
        return *pos;
    }
    constexpr void is_end(bool&, bool&) const
    {
    }
};
template <std::size_t I, typename R>
struct range_iterator_leaf_base<I, R, iterator_mode::index>
{
    using pointer = decltype(std::declval<R>().data());

    pointer data;

    constexpr range_iterator_leaf_base(R& range)
        : data(range.data())
    {
    }
    constexpr range_iterator_leaf_base& operator ++(void)
    {
        return *this;
    }
    constexpr decltype(auto) operator [](std::ptrdiff_t index)
    {
        return data[index];
    }
    constexpr void is_end(bool&, bool&) const
    {
    }
};
template <std::size_t I, typename R>
struct range_iterator_leaf_base<I, R, iterator_mode::range_index>
{
    constexpr range_iterator_leaf_base(R&)
    {
    }
    constexpr range_iterator_leaf_base& operator ++(void)
    {
        return *this;
    }
    constexpr std::ptrdiff_t operator [](std::ptrdiff_t index)
    {
        return index;
    }
    constexpr void is_end(bool&, bool&) const
    {
    }
};

template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES range_iterator_leaf
    : range_iterator_leaf_base<I, R, range_iterator_leaf_mode_<std::decay_t<R>>::value>
{
    using leaf_base = range_iterator_leaf_base<I, R, range_iterator_leaf_mode_<std::decay_t<R>>::value>;
    using leaf_base::leaf_base;
};
template <std::size_t I, typename R>
constexpr range_iterator_leaf<I, R>& get_leaf(range_iterator_leaf<I, R>& self)
{
    return self;
}

template <typename F, typename Is, typename... Rs> struct range_iterator_base;
template <typename F, std::size_t... Is, typename... Rs>
struct MAKESHIFT_DETAIL_EMPTY_BASES range_iterator_base<F, std::index_sequence<Is...>, Rs...>
    : F, range_iterator_leaf<Is, Rs>...
{
    constexpr range_iterator_base(F func, Rs&... ranges)
        : F(std::move(func)), range_iterator_leaf<Is, Rs>(ranges)...
    {
    }

    constexpr range_iterator_base& operator ++(void)
    {
        using Swallow = int[];
        (void) Swallow{ 1, (++detail::get_leaf<Is>(*this), void(), int{ })... };
        return *this;
    }
    constexpr bool is_consistent(void)
    {
        bool isNotEnd = false;
        bool isEnd = false;
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this).is_end(isNotEnd, isEnd), void(), int{ })... };
        return !(isNotEnd && isEnd);
    }
    constexpr bool is_end(void)
    {
        bool isNotEnd = false;
        bool isEnd = false;
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this).is_end(isNotEnd, isEnd), void(), int{ })... };

            // Check that different ranges don't have different sizes.
        gsl_Expects(!(isNotEnd && isEnd));

        return isEnd;
    }
    constexpr decltype(auto) invoke(std::ptrdiff_t index)
    {
        (void) index;
        return static_cast<F&>(*this)(detail::get_leaf<Is>(*this)[index]...);
    }
};
template <typename F, typename... Rs>
struct range_iterator
    : range_iterator_base<F, std::make_index_sequence<sizeof...(Rs)>, Rs...>
{
    using base = range_iterator_base<F, std::make_index_sequence<sizeof...(Rs)>, Rs...>;
    using base::base;
};


template <typename F>
struct count_if_fn
{
    F func;

    template <typename... ArgsT>
    constexpr std::ptrdiff_t operator ()(ArgsT&&... args) const
    {
        return func(std::forward<ArgsT>(args)...) ? std::ptrdiff_t(1) : std::ptrdiff_t(0);
    }
};


constexpr std::ptrdiff_t unknown_size = -1;

template <std::ptrdiff_t N> using ptrdiff_constant = std::integral_constant<std::ptrdiff_t, N>;

template <typename T> struct static_range_size_type_ : std::false_type { };
template <> struct static_range_size_type_<ptrdiff_constant<unknown_size>> : std::false_type { };
template <std::ptrdiff_t N> struct static_range_size_type_<ptrdiff_constant<N>> : std::true_type { using range_size_type = ptrdiff_constant<N>; };
template <typename T> struct dynamic_range_size_type_ : std::false_type { };
template <> struct dynamic_range_size_type_<std::ptrdiff_t> : std::true_type { using range_size_type = std::ptrdiff_t; };
struct default_range_size_type_ : std::true_type { using range_size_type = ptrdiff_constant<unknown_size>; };
template <typename... Ts> using range_size_type = typename gsl::disjunction<static_range_size_type_<Ts>..., dynamic_range_size_type_<Ts>..., default_range_size_type_>::range_size_type;

constexpr std::ptrdiff_t merge_sizes_0(ptrdiff_constant<unknown_size>, std::ptrdiff_t n)
{
    return n;
}
template <std::ptrdiff_t N>
constexpr ptrdiff_constant<N> merge_sizes_0(ptrdiff_constant<N>, std::ptrdiff_t n)
{
    gsl_Expects(n == N || n == unknown_size);
    return { };
}
template <std::ptrdiff_t N, std::ptrdiff_t M>
constexpr ptrdiff_constant<N == unknown_size ? M : N> merge_sizes_0(ptrdiff_constant<N>, ptrdiff_constant<M>)
{
    static_assert(N == unknown_size || M == unknown_size || N == M, "inconsistent sizes");
    return { };
}
template <std::ptrdiff_t N>
constexpr auto merge_sizes_0(std::ptrdiff_t n, ptrdiff_constant<N>)
{
    return detail::merge_sizes_0(ptrdiff_constant<N>{ }, n);
}
constexpr auto merge_sizes_0(std::ptrdiff_t n, std::ptrdiff_t m)
{
    if (n == unknown_size || m == unknown_size) return unknown_size;
    gsl_Expects(n == m);
    return n;
}

constexpr ptrdiff_constant<unknown_size> merge_sizes(void)
{
    return { };
}
template <typename T>
constexpr T merge_sizes(T n)
{
    return n;
}
template <std::ptrdiff_t Size1, typename... Ts>
constexpr range_size_type<ptrdiff_constant<Size1>, std::ptrdiff_t, Ts...>
merge_sizes(ptrdiff_constant<Size1> size1, std::ptrdiff_t size2, Ts... sizes);
template <std::ptrdiff_t Size2, typename... Ts>
constexpr range_size_type<std::ptrdiff_t, ptrdiff_constant<Size2>, Ts...>
merge_sizes(std::ptrdiff_t size1, ptrdiff_constant<Size2> size2, Ts... sizes);
template <typename... Ts>
constexpr range_size_type<std::ptrdiff_t, Ts...>
merge_sizes(std::ptrdiff_t size1, std::ptrdiff_t size2, Ts... sizes);
template <std::ptrdiff_t Size1, std::ptrdiff_t Size2, typename... Ts>
constexpr range_size_type<ptrdiff_constant<Size1>, ptrdiff_constant<Size2>, Ts...>
merge_sizes(ptrdiff_constant<Size1> size1, ptrdiff_constant<Size2> size2, Ts... sizes)
{
    return detail::merge_sizes(detail::merge_sizes_0(size1, size2), sizes...);
}
template <std::ptrdiff_t Size1, typename... Ts>
constexpr range_size_type<ptrdiff_constant<Size1>, std::ptrdiff_t, Ts...>
merge_sizes(ptrdiff_constant<Size1> size1, std::ptrdiff_t size2, Ts... sizes)
{
    return detail::merge_sizes_0(size1, detail::merge_sizes(sizes..., size2));
}
template <std::ptrdiff_t Size2, typename... Ts>
constexpr range_size_type<std::ptrdiff_t, ptrdiff_constant<Size2>, Ts...>
merge_sizes(std::ptrdiff_t size1, ptrdiff_constant<Size2> size2, Ts... sizes)
{
    return detail::merge_sizes_0(size2, detail::merge_sizes(sizes..., size1));
}
template <typename... Ts>
constexpr range_size_type<std::ptrdiff_t, Ts...>
merge_sizes(std::ptrdiff_t size1, std::ptrdiff_t size2, Ts... sizes)
{
    return detail::merge_sizes_0(detail::merge_sizes(sizes..., size1), size2);
}

template <typename R>
constexpr std::ptrdiff_t range_size_1(std::true_type /*hasSize*/, R const& range) noexcept
{
    return gsl::ssize(range);
}
template <typename R>
constexpr ptrdiff_constant<unknown_size> range_size_1(std::false_type /*hasSize*/, R const&) noexcept
{
    return { };
}
template <typename R>
constexpr ptrdiff_constant<std::tuple_size<R>::value> range_size_0(std::true_type /*isTupleLike*/, R const&) noexcept
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
constexpr ptrdiff_constant<unknown_size> range_size(range_index_t) noexcept
{
    return { };
}

template <typename T, std::size_t Size>
constexpr ptrdiff_constant<Size> to_ptrdiff_size(std::integral_constant<T, Size>) noexcept
{
    return { };
}
template <typename T>
constexpr std::ptrdiff_t to_ptrdiff_size(T size) noexcept
{
    return std::ptrdiff_t(size);
}


template <typename N, typename F, typename... Rs>
constexpr void
range_for(N n, F&& func, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<F>, Rs...>{ std::forward<F>(func), ranges... };
    for (std::ptrdiff_t i = 0; i != n; ++i, ++rangeIterator)
    {
            // Check that different ranges don't have different sizes.
        gsl_Expects(rangeIterator.is_consistent());

        rangeIterator.invoke(i);
    }
}

template <typename F, typename... Rs>
constexpr void
range_for(ptrdiff_constant<unknown_size>, F&& func, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<F>, Rs...>{ std::forward<F>(func), ranges... };
    for (std::ptrdiff_t i = 0; !rangeIterator.is_end(); ++i, ++rangeIterator)
    {
        rangeIterator.invoke(i);
    }
}


template <typename N, typename T, typename ReduceFuncT, typename TransformFuncT, typename... Rs>
constexpr std::decay_t<T>
range_transform_reduce(N n, T&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<TransformFuncT>, Rs...>{ std::forward<TransformFuncT>(transform), ranges... };
    auto result = std::forward<T>(initialValue);
    for (std::ptrdiff_t i = 0; i != n; ++i, ++rangeIterator)
    {
            // Check that different ranges don't have different sizes.
        gsl_Expects(rangeIterator.is_consistent());

        result = reduce(std::move(result), rangeIterator.invoke(i));
    }
    return result;
}

template <typename T, typename ReduceFuncT, typename TransformFuncT, typename... Rs>
constexpr std::decay_t<T>
range_transform_reduce(ptrdiff_constant<unknown_size>, T&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<TransformFuncT>, Rs...>{ std::forward<TransformFuncT>(transform), ranges... };
    auto result = std::forward<T>(initialValue);
    for (std::ptrdiff_t i = 0; !rangeIterator.is_end(); ++i, ++rangeIterator)
    {
        result = reduce(std::move(result), rangeIterator.invoke(i));
    }
    return result;
}


template <typename PredFuncT, typename N, typename TransformFuncT, typename... Rs>
constexpr bool
range_conjunction(N n, TransformFuncT&& transform, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<TransformFuncT>, Rs...>{ std::forward<TransformFuncT>(transform), ranges... };
    for (std::ptrdiff_t i = 0; i != n; ++i, ++rangeIterator)
    {
            // Check that different ranges don't have different sizes.
        gsl_Expects(rangeIterator.is_consistent());

        if (!PredFuncT{ }(rangeIterator.invoke(i))) return false;
    }
    return true;
}

template <typename PredFuncT, typename TransformFuncT, typename... Rs>
constexpr bool
range_conjunction(ptrdiff_constant<unknown_size>, TransformFuncT&& transform, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<TransformFuncT>, Rs...>{ std::forward<TransformFuncT>(transform), ranges... };
    for (std::ptrdiff_t i = 0; !rangeIterator.is_end(); ++i, ++rangeIterator)
    {
        if (!PredFuncT{ }(rangeIterator.invoke(i))) return false;
    }
    return true;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
