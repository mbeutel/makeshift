
#ifndef INCLUDED_MAKESHIFT_DETAIL_SPAN_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SPAN_HPP_


#include <tuple>
#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for swap()
#include <iterator>    // for input_iterator_tag, random_access_iterator_tag
#include <type_traits> // for integral_constant<>, enable_if<>, remove_cv<>, is_const<>, is_same<>

#include <gsl-lite/gsl-lite.hpp> // for conjunction<>, negation<>, span<>, gsl_Expects(), gsl_NODISCARD

#include <makeshift/tuple.hpp>       // for template_for(), tuple_transform()
#include <makeshift/type_traits.hpp> // for nth_type<>


namespace makeshift {


namespace gsl = ::gsl_lite;


template <typename... Ts>
class soa_span;


namespace detail {


template <typename... Ts>
class soa_span_iterator;

template <typename... Ts>
class soa_reference;

template <typename... Ts>
class soa_reference
{
    template <typename... RTs> friend class makeshift::soa_span;
    template <typename... RTs> friend class detail::soa_span_iterator;

private:
    std::tuple<std::remove_cv_t<Ts>*...> const& data_;
    std::ptrdiff_t index_;

    constexpr soa_reference(std::tuple<std::remove_cv_t<Ts>*...> const& _data, std::ptrdiff_t _index)
        : data_(_data), index_(_index)
    {
    }

public:
    soa_reference(soa_reference const&) = default;
    constexpr soa_reference& operator =(soa_reference const& rhs) noexcept
    {
        makeshift::template_for(
            [dstIndex = index_, srcIndex = rhs.index_]
            (auto* dst, auto* src)
            {
                dst[dstIndex] = src[srcIndex];
            },
            data_, rhs.data_);
        return *this;
    }
    constexpr soa_reference& operator =(std::tuple<std::remove_cv_t<Ts>...> const& rhs) noexcept
    {
        makeshift::template_for(
            [index = index_]
            (auto* dst, auto const& val)
            {
                dst[index] = val;
            },
            data_, rhs);
        return *this;
    }
    gsl_NODISCARD constexpr operator std::tuple<std::remove_cv_t<Ts>...>(void) const noexcept
    {
        return makeshift::tuple_transform(
            [index = index_]
            (auto* p)
            {
                return p[index];
            },
            data_);
    }

        // Implement tuple-like interface for `soa_reference<>`.
    template <std::size_t I>
    gsl_NODISCARD friend constexpr nth_type_t<I, Ts...>& get(soa_reference const& self) noexcept
    {
        return std::get<I>(self.data_)[self.index_];
    }
};

template <typename... Ts>
constexpr std::enable_if_t<gsl::conjunction_v<gsl::negation<std::is_const<Ts>>...>>
swap(soa_reference<Ts...> const& lhs, soa_reference<Ts...> const& rhs) noexcept
{
    makeshift::template_for<sizeof...(Ts)>(
        [&lhs, &rhs]
        (auto iC)
        {
            using std::swap;
            using std::get;
            swap(get<iC>(lhs), get<iC>(rhs));
        },
        tuple_index);
}


template <typename... Ts>
class soa_span_iterator
{
    template <typename... RTs> friend class makeshift::soa_span;
    template <typename... RTs> friend class detail::soa_span_iterator;

private:
    std::tuple<std::remove_cv_t<Ts>*...> const* data_;
    std::ptrdiff_t index_;

    constexpr soa_span_iterator(std::tuple<std::remove_cv_t<Ts>*...> const* _data, std::ptrdiff_t _index)
        : data_(_data), index_(_index)
    {
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<Ts...>;
    using pointer = void;
    using reference = soa_reference<Ts...>;
    using iterator_category = std::input_iterator_tag; // Unfortunately we cannot be a legacy random-access iterator because of the proxy class.
    using iterator_concept = std::random_access_iterator_tag; // We can be a non-legacy random access iterator though in C++20.

    template <typename... RTs,
              std::enable_if_t<gsl::conjunction_v<std::is_const<Ts>..., gsl::negation<std::is_const<RTs>>..., std::is_same<Ts, const RTs>...>, int> = 0>
        constexpr soa_span_iterator(soa_span_iterator<RTs...> const& rhs) noexcept
            : data_(rhs.data_), index_(rhs.index_)
    {
    }
    soa_span_iterator(soa_span_iterator const&) = default;
    soa_span_iterator& operator =(soa_span_iterator const&) = default;

    gsl_NODISCARD constexpr reference operator *(void) const
    {
        return { *data_, index_ };
    }
    gsl_NODISCARD reference operator [](difference_type n) const
    {
        return { *data_, index_ + n };
    }
    constexpr soa_span_iterator& operator ++(void)
    {
        ++index_;
        return *this;
    }
    constexpr soa_span_iterator operator ++(int)
    {
        auto result = soa_span_iterator<Ts...>{ *this };
        ++index_;
        return result;
    }
    constexpr soa_span_iterator& operator +=(difference_type n)
    {
        index_ += n;
        return *this;
    }
    constexpr soa_span_iterator& operator --(void)
    {
        --index_;
        return *this;
    }
    constexpr soa_span_iterator operator --(int)
    {
        auto result = soa_span_iterator<Ts...>{ *this };
        --index_;
        return result;
    }
    gsl_NODISCARD friend constexpr soa_span_iterator operator +(soa_span_iterator it, difference_type n)
    {
        it.index_ += n;
        return it;
    }
    gsl_NODISCARD friend constexpr soa_span_iterator operator +(difference_type n, soa_span_iterator it)
    {
        it.index_ += n;
        return it;
    }
    gsl_NODISCARD friend constexpr soa_span_iterator operator -(soa_span_iterator it, difference_type n)
    {
        it.index_ -= n;
        return it;
    }
    gsl_NODISCARD friend constexpr difference_type operator -(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return lhs.index_ - rhs.index_;
    }

    friend constexpr void swap(soa_span_iterator& lhs, soa_span_iterator& rhs) noexcept
    {
        std::swap(lhs.data_, rhs.data_);
        std::swap(lhs.index_, rhs.index_);
    }

    gsl_NODISCARD friend constexpr bool operator ==(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return lhs.index_ == rhs.index_;
    }
    gsl_NODISCARD friend constexpr bool operator !=(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return !(lhs == rhs);
    }
    gsl_NODISCARD friend constexpr bool operator <(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return lhs.index_ < rhs.index_;
    }
    gsl_NODISCARD friend constexpr bool operator >(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return rhs < lhs;
    }
    gsl_NODISCARD friend constexpr bool operator <=(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return !(rhs < lhs);
    }
    gsl_NODISCARD friend constexpr bool operator >=(soa_span_iterator const& lhs, soa_span_iterator const& rhs)
    {
        return !(lhs < rhs);
    }
};


template <typename R>
constexpr R check_all_equal(void)
{
    return { };
}
template <typename R, typename T0>
constexpr R check_all_equal(T0 v0)
{
    return v0;
}
template <typename R, typename T0, typename... Ts>
constexpr R check_all_equal(T0 v0, Ts... vs)
{
    auto result = check_all_equal<R>(vs...);
    gsl_Expects(v0 == result);
    return result;
}


} // namespace detail

} // namespace makeshift


namespace std {


    // Implement tuple-like interface for `soa_reference<>`.
template <typename... Ts> struct tuple_size<makeshift::detail::soa_reference<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> struct tuple_element<I, makeshift::detail::soa_reference<Ts...>> { using type = makeshift::nth_type_t<I, Ts...>; };


    // Implement tuple-like interface for `soa_span<>`.
template <typename... Ts> struct tuple_size<makeshift::soa_span<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> struct tuple_element<I, makeshift::soa_span<Ts...>> { using type = gsl_lite::span<makeshift::nth_type_t<I, Ts...>>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_DETAIL_SPAN_HPP_
