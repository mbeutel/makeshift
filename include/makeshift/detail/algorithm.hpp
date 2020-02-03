
#ifndef INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_


#include <gsl-lite/gsl-lite.hpp> // for disjunction<>, gsl_Expects(), ssize(), void_t<>, gsl_CPP17_OR_GREATER


#include <cstddef>     // for size_t, ptrdiff_t
#include <tuple>
#include <utility>     // for tuple_size<> (C++17), move(), forward<>(), swap(), integer_sequence<>
#include <type_traits> // for integral_constant<>, declval<>(), decay<>

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/type_traits.hpp> // for is_tuple_like<>
#include <makeshift/detail/range-index.hpp> // for range_index_t


namespace makeshift {


namespace gsl = ::gsl_lite;


namespace detail {


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

template <std::size_t I, typename R, iterator_mode IteratorMode>
struct zip_iterator_leaf_base;
template <std::size_t I, typename R>
struct zip_iterator_leaf_base<I, R, iterator_mode::iterator_pair>
{
    using iterator = decltype(std::declval<R>().begin());

    iterator pos;
    iterator end;

    constexpr zip_iterator_leaf_base(R& range)
        : pos(range.begin()), end(range.end())
    {
    }
    constexpr zip_iterator_leaf_base& operator ++(void)
    {
        ++pos;
        return *this;
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
struct zip_iterator_leaf_base<I, R, iterator_mode::iterator>
{
    using iterator = decltype(std::declval<R>().begin());

    iterator pos;

    constexpr zip_iterator_leaf_base(R& range)
        : pos(range.begin())
    {
    }
    constexpr zip_iterator_leaf_base& operator ++(void)
    {
        ++pos;
        return *this;
    }
    constexpr decltype(auto) operator [](gsl::index)
    {
        return *pos;
    }
    constexpr void check_end(bool /*isEnd*/) const noexcept
    {
    }
    constexpr nullopt_bool is_end(void) const noexcept
    {
        return { };
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::index>
{
    using pointer = decltype(std::declval<R>().data());

    pointer data;

    constexpr zip_iterator_leaf_base(R& range)
        : data(range.data())
    {
    }
    constexpr zip_iterator_leaf_base& operator ++(void) noexcept
    {
        return *this;
    }
    constexpr decltype(auto) operator [](gsl::index index) noexcept
    {
        return data[index];
    }
    constexpr void check_end(bool /*isEnd*/) const noexcept
    {
    }
    constexpr nullopt_bool is_end(void) const noexcept
    {
        return { };
    }
};
template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf_base<I, R, iterator_mode::range_index>
{
    constexpr zip_iterator_leaf_base(range_index_t)
    {
    }
    constexpr zip_iterator_leaf_base& operator ++(void) noexcept
    {
        return *this;
    }
    constexpr gsl::index operator [](gsl::index index) noexcept
    {
        return index;
    }
    constexpr void check_end(bool /*isEnd*/) const noexcept
    {
    }
    constexpr nullopt_bool is_end(void) const noexcept
    {
        return { };
    }
};

template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_leaf
    : zip_iterator_leaf_base<I, R, range_iterator_leaf_mode_<std::decay_t<R>>::value>
{
    using leaf_base = zip_iterator_leaf_base<I, R, range_iterator_leaf_mode_<std::decay_t<R>>::value>;
    using leaf_base::leaf_base;
};
template <std::size_t I, typename R>
constexpr zip_iterator_leaf<I, R>& get_leaf(zip_iterator_leaf<I, R>& self)
{
    return self;
}

constexpr gsl::dim unknown_size = -1;

template <gsl::dim N> using dim_constant = std::integral_constant<gsl::dim, N>;

template <typename T>
struct zip_iterator_sentinel;
template <>
struct zip_iterator_sentinel<gsl::dim>
{
private:
    gsl::dim n_;
    
public:
    constexpr zip_iterator_sentinel(gsl::dim _n) noexcept
        : n_(_n)
    {
    }
    constexpr operator gsl::dim(void) const noexcept
    {
        return n_;
    }
};
template <gsl::dim N>
struct zip_iterator_sentinel<dim_constant<N>> : dim_constant<N>
{
    constexpr zip_iterator_sentinel(dim_constant<N>)
    {
    }
};
template <>
struct zip_iterator_sentinel<dim_constant<unknown_size>>
{
    constexpr zip_iterator_sentinel(dim_constant<unknown_size>)
    {
    }
};

template <typename... Ts>
constexpr std::tuple<Ts...> make_forwarding_tuple(Ts&&... args)
{
    return std::tuple<Ts...>{ std::forward<Ts>(args)... };
}

template <typename DerivedT, typename N, typename Is, typename... Rs>
class zip_iterator_base;
template <typename DerivedT, typename N, std::size_t... Is, typename... Rs>
class MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_base<DerivedT, N, std::index_sequence<Is...>, Rs...>
    : private zip_iterator_leaf<Is, Rs>...
{
private:
    gsl::index i_;

public:
    explicit constexpr zip_iterator_base(Rs&... ranges)
        : zip_iterator_leaf<Is, Rs>(ranges)..., i_(0)
    {
    }
    constexpr DerivedT& operator ++(void)
    {
        using Swallow = int[];
        (void) Swallow{ 1, (++detail::get_leaf<Is>(*this), void(), int{ })... };
        ++i_;
        return static_cast<DerivedT&>(*this);
    }
    constexpr decltype(auto) operator *(void)
    {
        return detail::make_forwarding_tuple(detail::get_leaf<Is>(*this)[i_]...);
    }
    template <typename F>
    constexpr decltype(auto) invoke(F&& func)
    {
        return func(detail::get_leaf<Is>(*this)[i_]...);
    }
    constexpr bool operator !=(zip_iterator_sentinel<N> sentinel)
    {
        bool isEnd = i_ == sentinel;
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this).check_end(isEnd), 0)... };
        return !isEnd;
    }
};
template <typename DerivedT, std::size_t... Is, typename... Rs>
class MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_base<DerivedT, dim_constant<unknown_size>, std::index_sequence<Is...>, Rs...>
    : private zip_iterator_leaf<Is, Rs>...
{
public:
    explicit constexpr zip_iterator_base(Rs&... ranges)
        : zip_iterator_leaf<Is, Rs>(ranges)...
    {
    }
    constexpr DerivedT& operator ++(void)
    {
        using Swallow = int[];
        (void) Swallow{ 1, (++detail::get_leaf<Is>(*this), void(), int{ })... };
        return static_cast<DerivedT>(*this);
    }
    constexpr decltype(auto) operator *(void)
    {
        return detail::make_forwarding_tuple(detail::get_leaf<Is>(*this)[{ }]...);
    }
    template <typename F>
    constexpr decltype(auto) invoke(F&& func)
    {
        return func(detail::get_leaf<Is>(*this)[{ }]...);
    }
    constexpr bool operator !=(zip_iterator_sentinel<dim_constant<unknown_size>>)
    {
        return detail::is_end(detail::get_leaf<Is>(*this).is_end()...);
    }
};
template <typename N, typename... Rs>
struct zip_iterator
    : zip_iterator_base<zip_iterator<N, Rs...>, N, std::make_index_sequence<sizeof...(Rs)>, Rs...>
{
    using base = zip_iterator_base<zip_iterator<N, Rs...>, N, std::make_index_sequence<sizeof...(Rs)>, Rs...>;
    using base::base;
};

template <typename N, typename... Rs>
constexpr zip_iterator<N, Rs&...> make_zip_iterator(N, Rs&... ranges)
{
    return zip_iterator<N, Rs&...>(ranges...);
}
template <typename N>
constexpr zip_iterator_sentinel<N> make_zip_iterator_sentinel(N n) noexcept
{
    return { n };
}

template <typename N, typename Is, typename... Rs>
class zip_range_base;
template <typename N, std::size_t... Is, typename... Rs>
class zip_range_base<N, std::index_sequence<Is...>, Rs...>
{
private:
    using iterator = zip_iterator<N, Rs&...>;
    using sentinel = zip_iterator_sentinel<N>;

    std::tuple<Rs...> ranges_;
    N size_;

public:
    explicit constexpr zip_range_base(N _size, Rs... _ranges)
        : ranges_(std::forward<Rs>(_ranges)...), size_(_size)
    {
    }

    constexpr iterator begin(void) { return iterator(std::get<Is>(ranges_)...); }
    constexpr sentinel end(void) { return sentinel(size_); }
};
template <typename N, typename... Rs>
class zip_range
    : zip_range_base<N, std::make_index_sequence<sizeof...(Rs)>, Rs...>
{
    using base = zip_range_base<N, std::make_index_sequence<sizeof...(Rs)>, Rs...>;
    using base::base;
};

template <typename N, typename... Rs>
constexpr zip_range<N, Rs...> make_zip_range(N n, Rs&&... ranges)
{
    return zip_range<N, Rs...>(n, std::forward<Rs>(ranges)...);
}


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

template <typename T, std::size_t Size>
constexpr dim_constant<Size> to_range_size(std::integral_constant<T, Size>) noexcept
{
    return { };
}
template <typename T>
constexpr gsl::dim to_range_size(T size) noexcept
{
    return gsl::dim(size);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
