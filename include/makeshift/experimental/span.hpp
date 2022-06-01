
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_SPAN_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_SPAN_HPP_


#include <tuple>
#include <cstddef>      // for size_t, ptrdiff_t
#include <type_traits>  // for remove_cv<>

#include <gsl-lite/gsl-lite.hpp>  // for span<>, gsl_Expects(), gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/type_traits.hpp>  // for nth_type<>

#include <makeshift/experimental/detail/span.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Tuple of spans with span-of-tuples interface.
    //ᅟ
    //ᅟ    std::vector<int> is, js;
    //ᅟ    std::vector<double> vs;
    //ᅟ    auto A = soa_span(gsl::span(is), gsl::span(js), gsl::span(vs));
    //ᅟ    auto [i, j, v] = A[k];
    //
template <typename... Ts>
class soa_span
{
private:
    std::tuple<std::remove_cv_t<Ts>*...> data_;
    std::size_t size_;

    constexpr soa_span(std::tuple<std::remove_cv_t<Ts>*...> const& _data, std::size_t _size)
        : data_(_data), size_(_size)
    {
    }

public:
    using element_type = std::tuple<Ts...>;
    using value_type = std::tuple<std::remove_cv_t<Ts>...>;
    using index_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = detail::soa_reference<Ts...>;
    using const_reference = detail::soa_reference<Ts const...>;
    using iterator = detail::soa_span_iterator<Ts...>;
    using const_iterator = detail::soa_span_iterator<Ts const...>;

    explicit constexpr soa_span(gsl::span<Ts>... spans)
        : soa_span(
            std::make_tuple(const_cast<std::remove_cv_t<Ts>*>(spans.data())...),
            detail::check_all_equal<std::size_t>(spans.size()...))
    {
    }

    [[nodiscard]] constexpr std::size_t
    size(void) const noexcept
    {
        return std::size_t(size_);
    }
    [[nodiscard]] constexpr bool
    empty(void) const noexcept
    {
        return size_ != 0;
    }

    [[nodiscard]] constexpr iterator
    begin(void) const noexcept
    {
        return { &data_, 0 };
    }
    [[nodiscard]] constexpr iterator
    end(void) const noexcept
    {
        return { &data_, difference_type(size_) };
    }
    [[nodiscard]] constexpr const_iterator
    cbegin(void) const noexcept
    {
        return { &data_, 0 };
    }
    [[nodiscard]] constexpr const_iterator
    cend(void) const noexcept
    {
        return { &data_, difference_type(size_) };
    }

    [[nodiscard]] constexpr reference
    operator [](std::size_t i) const
    {
        gsl_Expects(i < size_);

        return { data_, std::ptrdiff_t(i) };
    }
    [[nodiscard]] constexpr reference
    front(void) const
    {
        gsl_Expects(!empty());
        
        return { data_, 0 };
    }
    [[nodiscard]] constexpr reference
    back(void) const
    {
        gsl_Expects(!empty());
        
        return { data_, difference_type(size_ - 1) };
    }

    [[nodiscard]] constexpr soa_span<Ts...>
    subspan(std::size_t offset, std::size_t count = std::size_t(-1)) const
    {
        gsl_Expects(offset <= size() && (count == std::size_t(-1) || count <= size() - offset));

        std::size_t newSize = (count != std::size_t(-1))
            ? count
            : size_ - offset;
        return {
            makeshift::tuple_transform(
                [offset = std::ptrdiff_t(offset)]
                (auto* p)
                {
                    return p + offset;
                },
                data_),
            newSize
        };
    }
    [[nodiscard]] constexpr soa_span<Ts...>
    first(std::size_t count) const
    {
        return subspan(0, count);
    }
    [[nodiscard]] constexpr soa_span<Ts...>
    last(std::size_t count) const
    {
        gsl_Expects(count <= size());

        return subspan(size() - count, count);
    }

        // Implement tuple-like interface for `soa_span<>`.
    template <std::size_t I>
    [[nodiscard]] friend constexpr gsl::span<nth_type_t<I, Ts...>>
    get(soa_span const& self) noexcept
    {
        return { std::get<I>(self.data_), self.size_ };
    }
};
#if gsl_CPP17_OR_GREATER
template <typename... Ts>
soa_span(std::tuple<Ts*...> const&, std::size_t) -> soa_span<Ts...>;
template <typename... Ts>
soa_span(gsl::span<Ts>...) -> soa_span<Ts...>;
#endif // gsl_CPP17_OR_GREATER

    //
    // Tuple of spans with span-of-tuples interface.
    //ᅟ
    //ᅟ    std::vector<int> is, js;
    //ᅟ    std::vector<double> vs;
    //ᅟ    auto A = make_soa_span(gsl::span(is), gsl::span(js), gsl::span(vs));
    //ᅟ    auto [i, j, v] = A[k];
    //
template <typename... Ts>
[[nodiscard]] constexpr soa_span<Ts...>
make_soa_span(gsl::span<Ts>... spans)
{
    return soa_span<Ts...>(spans...);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_SPAN_HPP_
