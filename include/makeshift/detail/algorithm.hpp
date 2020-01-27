
#ifndef INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_


#include <gsl-lite/gsl-lite.hpp> // for disjunction<>, gsl_Expects(), ssize(), void_t<>, gsl_CPP17_OR_GREATER


#include <cstddef>     // for size_t, ptrdiff_t
#include <tuple>
#include <utility>     // for forward<>(), integer_sequence<>
#include <type_traits> // for integral_constant<>, declval<>(), decay<>

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/range-index.hpp> // for range_index_t
#include <makeshift/detail/zip.hpp>


namespace makeshift
{


namespace gsl = ::gsl_lite;


namespace detail
{


constexpr gsl::dim unknown_size = -1;

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
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this).advance(), void(), int{ })... };
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
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this).advance(), void(), int{ })... };
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


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
