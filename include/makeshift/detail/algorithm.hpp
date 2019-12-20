
#ifndef INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_


#include <gsl/gsl-lite.hpp> // for Expects(), ssize(), void_t<>, gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# include <tuple>      // for tuple_size<>
#endif // !gsl_CPP17_OR_GREATER

#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for tuple_size<> (C++17), move(), forward<>(), swap(), integer_sequence<>
#include <type_traits> // for integral_constant<>, declval<>(), decay<>

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/range-index.hpp> // for range_index_t
#include <makeshift/detail/type_traits.hpp> // for disjunction<>


namespace makeshift
{

namespace detail
{


template <typename R, typename = void> struct has_data : std::false_type { };
template <typename R> struct has_data<R, gsl::std17::void_t<decltype(gsl::std17::data(std::declval<R>()))>> : std::true_type { };

template <typename R, typename = void> struct has_size : std::false_type { };
template <typename R> struct has_size<R, gsl::std17::void_t<decltype(gsl::std17::size(std::declval<R>()))>> : std::true_type { };

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
    using base = range_iterator_leaf_base<I, R, range_iterator_leaf_mode_<std::decay_t<R>>::value>;
    using base::base;
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
    range_iterator_base(F func, Rs&... ranges)
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
        Expects(!(isNotEnd && isEnd));

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


constexpr std::ptrdiff_t inconsistent_size = -3;
constexpr std::ptrdiff_t no_size = -2;
constexpr std::ptrdiff_t unknown_size = -1;

constexpr std::ptrdiff_t merge_sizes(std::ptrdiff_t size1, std::ptrdiff_t size2)
{
    if (size1 > size2)
    {
        std::swap(size1, size2);
    }

    switch (size1)
    {
    case inconsistent_size:
        return inconsistent_size;
    case no_size:
    case unknown_size: // `size1 <= size2`, so `size2` is either `unknown_size` or a valid size
        return size2;
    default:
        return size2 == size1 ? size1 : inconsistent_size;
    }
}
constexpr std::ptrdiff_t merge_sizes(std::ptrdiff_t size)
{
    return size;
}
template <typename... Ts>
constexpr std::ptrdiff_t merge_sizes(std::ptrdiff_t size1, std::ptrdiff_t size2, Ts... sizes)
{
    return merge_sizes(merge_sizes(size1, size2), sizes...);
}

template <typename R, typename = void> struct static_range_size : std::integral_constant<std::ptrdiff_t, unknown_size> { };
template <typename R> struct static_range_size<R, gsl::std17::void_t<decltype(std::tuple_size<R>::value)>> : std::integral_constant<std::ptrdiff_t, std::tuple_size<R>::value> { };

template <typename R, typename = void>
struct dynamic_range_size_
{
    constexpr std::ptrdiff_t operator ()(R const&) const
    {
        return unknown_size;
    }
};
template <>
struct dynamic_range_size_<range_index_t>
{
    constexpr std::ptrdiff_t operator ()(range_index_t) const
    {
        return no_size;
    }
};
template <typename R>
struct dynamic_range_size_<R, gsl::std17::void_t<decltype(gsl::std20::ssize(std::declval<R>()))>>
{
    constexpr std::ptrdiff_t operator ()(R const& range) const
    {
        return gsl::std20::ssize(range);
    }
};

template <typename... Rs>
constexpr std::ptrdiff_t static_ranges_size(void)
{
    return detail::merge_sizes(static_range_size<std::decay_t<Rs>>::value...);
}
template <typename... Rs>
constexpr std::ptrdiff_t dynamic_ranges_size(Rs&... ranges)
{
    return detail::merge_sizes(dynamic_range_size_<std::decay_t<Rs>>{ }(ranges)...);
}

constexpr std::ptrdiff_t dynamic_size = -4;

template <typename... Rs>
constexpr std::integral_constant<std::ptrdiff_t, unknown_size> ranges_size_0(std::integral_constant<std::ptrdiff_t, unknown_size>, Rs&...)
{
    return { };
}
template <std::ptrdiff_t N, typename... Rs>
constexpr std::integral_constant<std::ptrdiff_t, N> ranges_size_0(std::integral_constant<std::ptrdiff_t, N>, Rs&... ranges)
{
        // Check that different ranges don't have different sizes.
    Expects(detail::dynamic_ranges_size<Rs...>(ranges...) >= 0);

    return { };
}
template <typename... Rs>
constexpr std::ptrdiff_t ranges_size_0(std::integral_constant<std::ptrdiff_t, dynamic_size>, Rs&... ranges)
{
    std::ptrdiff_t n = detail::dynamic_ranges_size<Rs...>(ranges...);

        // Check that different ranges don't have different sizes.
    Expects(n >= 0);

    return n;
}
template <std::ptrdiff_t StaticSize, typename... Rs>
constexpr auto ranges_size(Rs&... ranges)
{
    constexpr std::ptrdiff_t staticSizeEx =
        StaticSize != unknown_size ? StaticSize
      : disjunction<detail::has_size<Rs>...>::value ? dynamic_size
      : unknown_size;
    return detail::ranges_size_0(std::integral_constant<std::ptrdiff_t, staticSizeEx>{ }, ranges...);
}


template <typename N, typename F, typename... Rs>
constexpr void
range_for(N n, F&& func, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<F>, Rs...>{ std::forward<F>(func), ranges... };
    for (std::ptrdiff_t i = 0; i != n; ++i, ++rangeIterator)
    {
            // Check that different ranges don't have different sizes.
        Expects(rangeIterator.is_consistent());

        rangeIterator.invoke(i);
    }
}

template <typename F, typename... Rs>
constexpr void
range_for(std::integral_constant<std::ptrdiff_t, unknown_size>, F&& func, Rs&... ranges)
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
        Expects(rangeIterator.is_consistent());

        result = reduce(std::move(result), rangeIterator.invoke(i));
    }
}

template <typename T, typename ReduceFuncT, typename TransformFuncT, typename... Rs>
constexpr std::decay_t<T>
range_transform_reduce(std::integral_constant<std::ptrdiff_t, unknown_size>, T&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<TransformFuncT>, Rs...>{ std::forward<TransformFuncT>(transform), ranges... };
    auto result = std::forward<T>(initialValue);
    for (std::ptrdiff_t i = 0; !rangeIterator.is_end(); ++i, ++rangeIterator)
    {
        result = reduce(std::move(result), rangeIterator.invoke(i));
    }
}


template <typename PredFuncT, typename N, typename T, typename TransformFuncT, typename... Rs>
constexpr bool
range_conjunction(N n, TransformFuncT&& transform, Rs&... ranges)
{
    auto rangeIterator = range_iterator<std::decay_t<TransformFuncT>, Rs...>{ std::forward<TransformFuncT>(transform), ranges... };
    for (std::ptrdiff_t i = 0; i != n; ++i, ++rangeIterator)
    {
            // Check that different ranges don't have different sizes.
        Expects(rangeIterator.is_consistent());

        if (!PredFuncT{ }(rangeIterator.invoke(i))) return false;
    }
    return true;
}

template <typename PredFuncT, typename T, typename TransformFuncT, typename... Rs>
constexpr bool
range_conjunction(std::integral_constant<std::ptrdiff_t, unknown_size>, TransformFuncT&& transform, Rs&... ranges)
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
