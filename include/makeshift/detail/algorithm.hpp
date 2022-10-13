
#ifndef INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_


#include <cstddef>      // for size_t, ptrdiff_t
#include <tuple>
#include <utility>      // for forward<>(), integer_sequence<>
#include <type_traits>  // for integral_constant<>, declval<>(), decay<>

#include <gsl-lite/gsl-lite.hpp>  // for dim, index

#include <makeshift/detail/macros.hpp>  // for MAKESHIFT_DETAIL_EMPTY_BASES, MAKESHIFT_DETAIL_FORCEINLINE
#include <makeshift/detail/ranges.hpp>  // for range_index_t
#include <makeshift/detail/zip.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;

namespace detail {


template <typename T>
struct zip_iterator_sentinel;
template <>
struct zip_iterator_sentinel<gsl::dim>
{
private:
    gsl::dim n_;
    
public:
    explicit constexpr zip_iterator_sentinel(gsl::dim _n) noexcept
        : n_(_n)
    {
    }
    template <typename It>
    MAKESHIFT_DETAIL_FORCEINLINE constexpr bool _is_end(It const& it) const
    {
        bool isEnd = it.i_ == n_;
        it._check_end(isEnd);
        return isEnd;
    }
};
template <gsl::dim N>
struct zip_iterator_sentinel<dim_constant<N>>
{
    explicit constexpr zip_iterator_sentinel(dim_constant<N>)
    {
    }
    template <typename It>
    MAKESHIFT_DETAIL_FORCEINLINE constexpr bool _is_end(It const& it) const
    {
        bool isEnd = it.i_ == N;
        it._check_end(isEnd);
        return isEnd;
    }
};
template <>
struct zip_iterator_sentinel<dim_constant<unknown_size>>
{
    explicit constexpr zip_iterator_sentinel(dim_constant<unknown_size>)
    {
    }
    template <typename It>
    MAKESHIFT_DETAIL_FORCEINLINE constexpr bool _is_end(It const& it) const
    {
        return it._is_end();
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
MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator_leaf<I, R>& get_leaf(zip_iterator_leaf<I, R>& self) noexcept
{
    return self;
}
template <std::size_t I, typename R>
MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator_leaf<I, R> const& get_leaf(zip_iterator_leaf<I, R> const& self) noexcept
{
    return self;
}

template <typename DerivedT, typename N, typename Is, typename... Rs>
class zip_iterator_base;
template <typename DerivedT, typename N, std::size_t... Is, typename... Rs>
class MAKESHIFT_DETAIL_EMPTY_BASES zip_iterator_base<DerivedT, N, std::index_sequence<Is...>, Rs...>
    : private zip_iterator_leaf<Is, Rs>...
{
    friend struct zip_iterator_sentinel<N>;

private:
    gsl::index i_;

        // for zip_iterator_sentinel<>
    auto _is_end(void) const
    {
        return detail::is_end(detail::get_leaf<Is>(*this)._is_end()...);
    }

public:
    using difference_type = std::ptrdiff_t; // we just assume that here
    using value_type = std::tuple<typename zip_iterator_leaf<Is, Rs>::value_type...>;
    using pointer = void;
    using reference = std::tuple<typename zip_iterator_leaf<Is, Rs>::reference...>;
    using iterator_category = common_iterator_tag<range_iterator_category_t<std::decay_t<Rs>>...>;
#ifdef __cpp_concepts
        // this deliberately cannot include ContiguousIterator (zipping contiguous iterators does not yield a contiguous iterator)
    using iterator_concept = common_iterator_tag<range_iterator_concept_t<std::decay_t<Rs>>...>;
#endif // __cpp_concepts

    explicit constexpr zip_iterator_base(Rs&... ranges)
        : zip_iterator_leaf<Is, Rs>(ranges)..., i_(0)
    {
    }
    explicit constexpr zip_iterator_base(end_tag, N _size, Rs&... ranges)
        : zip_iterator_leaf<Is, Rs>(ranges, end_tag{ })..., i_(_size)
    {
    }

        // sanity check
    constexpr void _check_end(bool isEnd) const
    {
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this)._check_end(isEnd), 0)... };
    }

        // LegacyIterator: dereference, increment
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr reference operator *(void)
    {
        return reference{ detail::get_leaf<Is>(*this)._deref(i_)... };
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr DerivedT& operator ++(void)
    {
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this)._inc(), void(), int{ })... };
        ++i_;
        return static_cast<DerivedT&>(*this);
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr DerivedT operator ++(int)
    {
        DerivedT result = static_cast<DerivedT&>(*this);
        ++*this;
        return result;
    }

        // LegacyInputIterator: equality compare
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator ==(DerivedT const& lhs, DerivedT const& rhs) noexcept
    {
        return lhs.i_ == rhs.i_;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator !=(DerivedT const& lhs, DerivedT const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator ==(DerivedT const& it, zip_iterator_sentinel<N> sentinel)
    {
        return sentinel._is_end(it);
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator ==(zip_iterator_sentinel<N> sentinel, DerivedT const& it)
    {
        return it == sentinel;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator !=(DerivedT const& it, zip_iterator_sentinel<N> sentinel)
    {
        return !(it == sentinel);
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator !=(zip_iterator_sentinel<N> sentinel, DerivedT const& it)
    {
        return !(it == sentinel);
    }

        // BidirectionalIterator: decrement
    MAKESHIFT_DETAIL_FORCEINLINE constexpr DerivedT& operator --(void)
    {
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this)._dec(), void(), int{ })... };
        --i_;
        return static_cast<DerivedT&>(*this);
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr DerivedT operator --(int)
    {
        DerivedT result = static_cast<DerivedT&>(*this);
        --*this;
        return result;
    }

        // RandomAccessIterator: subscript, arithmetic, ordering compare
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr reference operator [](std::ptrdiff_t d)
    {
        return reference{ detail::get_leaf<Is>(*this)._deref(i_, d)... };
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr DerivedT& operator +=(std::ptrdiff_t d)
    {
        using Swallow = int[];
        (void) Swallow{ 1, (detail::get_leaf<Is>(*this)._advance(d), void(), int{ })... };
        i_ += d;
        return static_cast<DerivedT&>(*this);
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr DerivedT& operator -=(std::ptrdiff_t d)
    {
        return *this += -d;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr DerivedT operator +(DerivedT it, std::ptrdiff_t d)
    {
        it += d;
        return it;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr DerivedT operator +(std::ptrdiff_t d, DerivedT it)
    {
        it += d;
        return it;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr DerivedT operator -(DerivedT it, std::ptrdiff_t d)
    {
        it -= d;
        return it;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr std::ptrdiff_t operator -(DerivedT const& lhs, DerivedT const& rhs)
    {
        return lhs.i_ - rhs.i_;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator <(DerivedT const& lhs, DerivedT const& rhs) noexcept
    {
        return lhs.i_ < rhs.i_;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator <=(DerivedT const& lhs, DerivedT const& rhs) noexcept
    {
        return !(rhs < lhs);
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator >(DerivedT const& lhs, DerivedT const& rhs) noexcept
    {
        return rhs < lhs;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool operator >=(DerivedT const& lhs, DerivedT const& rhs) noexcept
    {
        return !(lhs < rhs);
    }

    template <typename F>
    MAKESHIFT_DETAIL_FORCEINLINE constexpr decltype(auto) apply(F&& func)
    {
        return func(detail::get_leaf<Is>(*this)._deref(i_)...);
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
MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator<N, Rs&...> make_zip_begin_iterator(N, Rs&... ranges)
{
    return zip_iterator<N, Rs&...>(ranges...);
}
template <typename N, typename... Rs>
MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator<N, Rs&...> make_zip_end_iterator(N size, Rs&... ranges)
{
    return zip_iterator<N, Rs&...>(end_tag{ }, size, ranges...);
}
template <typename N>
MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator_sentinel<N> make_zip_iterator_sentinel(N n) noexcept
{
    return zip_iterator_sentinel<N>(n);
}

template <typename... Rs> struct ranges_are_random_access_ : std::is_base_of<std::random_access_iterator_tag, common_iterator_tag<range_iterator_category_t<std::decay_t<Rs>>...>> { };

template <typename N>
class zip_range_size_base;
template <>
class zip_range_size_base<gsl::dim>
{
private:
    std::size_t n_;

protected:
    explicit constexpr zip_range_size_base(gsl::dim _n)
        : n_(std::size_t(_n))
    {
    }
    constexpr gsl::dim _size(void) const noexcept
    {
        return gsl::dim(n_);
    }

public:
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr std::size_t
    size(void) const noexcept
    {
        return n_;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr bool
    empty(void) const noexcept
    {
        return n_ == 0;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator_sentinel<gsl::dim>
    end(void) const noexcept
    {
        return zip_iterator_sentinel<gsl::dim>(n_);
    }
};
template <gsl::dim N>
class zip_range_size_base<dim_constant<N>>
{
protected:
    explicit constexpr zip_range_size_base(dim_constant<N>)
    {
    }
    static constexpr dim_constant<N> _size(void) noexcept
    {
        return { };
    }

public:
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE static constexpr std::size_t
    size(void) noexcept
    {
        return N;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE static constexpr bool
    empty(void) noexcept
    {
        return N == 0;
    }
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator_sentinel<dim_constant<N>>
    end(void) const noexcept
    {
        return zip_iterator_sentinel<dim_constant<N>>({ });
    }
};
template <>
class zip_range_size_base<dim_constant<unknown_size>>
{
protected:
    explicit constexpr zip_range_size_base(dim_constant<unknown_size>)
    {
    }
    static constexpr dim_constant<unknown_size> _size(void) noexcept
    {
        return { };
    }

public:
    [[nodiscard]] MAKESHIFT_DETAIL_FORCEINLINE constexpr zip_iterator_sentinel<dim_constant<unknown_size>>
    end(void) const noexcept
    {
        return zip_iterator_sentinel<dim_constant<unknown_size>>({ });
    }
};
template <typename N, typename Is, typename... Rs>
class zip_range_base;
template <typename N, std::size_t... Is, typename... Rs>
class zip_range_base<N, std::index_sequence<Is...>, Rs...>
    : public zip_range_size_base<N>
{
private:
    using iterator = zip_iterator<N, Rs&...>;

    std::tuple<Rs...> ranges_;

public:
    explicit constexpr zip_range_base(N _size, Rs... _ranges)
        : zip_range_size_base<N>(_size),
          ranges_(std::forward<Rs>(_ranges)...)
    {
    }

    [[nodiscard]] constexpr iterator
    begin(void) const
    {
        return iterator(std::get<Is>(ranges_)...);
    }
    [[nodiscard]] constexpr iterator
    common_end(void) const
    {
        return iterator(end_tag{ }, this->_size(), std::get<Is>(ranges_)...);
    }
};
template <bool IsRandomIt, typename N, typename... Rs>
class zip_range_subscript_base;
template <typename N, typename... Rs>
class zip_range_subscript_base<false, N, Rs...>
    : public zip_range_base<N, std::make_index_sequence<sizeof...(Rs)>, Rs...>
{
    using base = zip_range_base<N, std::make_index_sequence<sizeof...(Rs)>, Rs...>;
    using base::base;
};
template <typename N, typename... Rs>
class zip_range_subscript_base<true, N, Rs...>
    : public zip_range_base<N, std::make_index_sequence<sizeof...(Rs)>, Rs...>
{
    using base = zip_range_base<N, std::make_index_sequence<sizeof...(Rs)>, Rs...>;
    using base::base;

private:
    using iterator = zip_iterator<N, Rs&...>;

public:
    [[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE typename iterator::reference
    operator [](std::size_t i) const
    {
        return this->begin()[i];
    }
};

template <typename N, typename... Rs>
class zip_range
    : public zip_range_subscript_base<ranges_are_random_access_<Rs...>::value, N, Rs...>
{
    using base = zip_range_subscript_base<ranges_are_random_access_<Rs...>::value, N, Rs...>;
    using base::base;
};
template <typename N, typename... Rs>
constexpr zip_range<N, Rs...> make_zip_range(N n, Rs&&... ranges)
{
    return zip_range<N, Rs...>(n, std::forward<Rs>(ranges)...);
}

template <typename N, typename... Rs>
class zip_common_range
    : public zip_range_subscript_base<ranges_are_random_access_<Rs...>::value, N, Rs...>
{
private:
    using iterator = zip_iterator<N, Rs&...>;

    using base = zip_range_subscript_base<ranges_are_random_access_<Rs...>::value, N, Rs...>;
    using base::base;

public:
    [[nodiscard]] constexpr iterator
    end(void) const // overwriting `zip_range_size_base<>::end()`
    {
        return this->common_end();
    }
};
template <typename N, typename... Rs>
constexpr zip_common_range<N, Rs...> make_zip_common_range(N n, Rs&&... ranges)
{
    return zip_common_range<N, Rs...>(n, std::forward<Rs>(ranges)...);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ALGORITHM_HPP_
