
#ifndef INCLUDED_MAKESHIFT_ITERATOR_HPP_
#define INCLUDED_MAKESHIFT_ITERATOR_HPP_


#include <iterator>    // for input_iterator_tag, random_access_iterator_tag

#include <gsl-lite/gsl-lite.hpp> // for index, diff, gsl_NODISCARD

#include <makeshift/detail/macros.hpp> // for MAKESHIFT_DETAIL_FORCEINLINE


namespace makeshift {

namespace gsl = ::gsl_lite;


class index_iterator
{
private:
    gsl::index i_;

public:
    explicit constexpr index_iterator(gsl::index _i) noexcept
        : i_(_i)
    {
    }

    MAKESHIFT_DETAIL_FORCEINLINE constexpr gsl::index
    operator *(void) const
    {
        return i_;
    }

    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_iterator&
    operator ++(void)
    {
        ++i_;
        return *this;
    }

    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE constexpr bool
    friend operator ==(index_iterator lhs, index_iterator rhs) noexcept
    {
        return lhs.i_ == rhs.i_;
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE constexpr bool
    friend operator !=(index_iterator lhs, index_iterator rhs) noexcept
    {
        return !(lhs == rhs);
    }


    // Additional *LegacyIterator* requirements:
    // https://en.cppreference.com/w/cpp/named_req/Iterator

    using difference_type = gsl::diff;
    using value_type = gsl::index;
    using pointer = void;
    using reference = gsl::index;

        // We only satisfy the *LegacyInputIterator* requirement (see below).
    using iterator_category = std::input_iterator_tag;

        // However, we do satisfy the `std::random_access_iterator<>` concept.
    using iterator_concept = std::random_access_iterator_tag;

    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_iterator
    operator ++(int)
    {
        auto result = *this;
        ++*this;
        return result;
    }


    // Additional *LegacyInputIterator* requirements:
    // https://en.cppreference.com/w/cpp/named_req/InputIterator

private:
    struct ArrowProxy  // cf. https://quuxplusone.github.io/blog/2019/02/06/arrow-proxy/.
    {
        gsl::index i;

        MAKESHIFT_DETAIL_FORCEINLINE constexpr gsl::index*
        operator ->() && noexcept
        {
            return &i;
        }
    };

public:
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE constexpr ArrowProxy
    operator ->(void)
    {
        return { i_ };
    }


    // We cannot satisfy the *LegacyForwardIterator* requirements:
    // https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    // because our `reference` is neither `value_type&` nor `value_type const&`. However, we do already satisfy the
    // `std::forward_iterator<>` concept:
    // https://en.cppreference.com/w/cpp/iterator/forward_iterator


    // Additional `std::bidirectional_iterator<>` concept requirements:
    // https://en.cppreference.com/w/cpp/iterator/bidirectional_iterator

    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_iterator&
    operator --()
    {
        --i_;
        return *this;
    }

    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_iterator
    operator --(int)
    {
        auto result = *this;
        --*this;
        return result;
    }
    

    // Additional `std::random_access_iterator<>` concept requirements:
    // https://en.cppreference.com/w/cpp/iterator/random_access_iterator

    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_iterator&
    operator +=(difference_type offset)
    {
        i_ += offset;
        return *this;
    }
    MAKESHIFT_DETAIL_FORCEINLINE constexpr index_iterator&
    operator -=(difference_type offset)
    {
        i_ -= offset;
        return *this;
    }

    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr index_iterator
    operator +(index_iterator it, difference_type offset)
    {
        it += offset;
        return it;
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr index_iterator
    operator +(difference_type offset, index_iterator it)
    {
        it += offset;
        return it;
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr index_iterator
    operator -(index_iterator it, difference_type offset)
    {
        it -= offset;
        return it;
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr difference_type
    operator -(index_iterator lhs, index_iterator rhs) noexcept
    {
        return lhs.i_ - rhs.i_;
    }

    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool
    operator <(index_iterator lhs, index_iterator rhs) noexcept
    {
        return lhs.i_ < rhs.i_;
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool
    operator <=(index_iterator lhs, index_iterator rhs) noexcept
    {
        return !(rhs < lhs);
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool
    operator >(index_iterator lhs, index_iterator rhs) noexcept
    {
        return rhs < lhs;
    }
    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE friend constexpr bool
    operator >=(index_iterator lhs, index_iterator rhs) noexcept
    {
        return !(lhs < rhs);
    }

    gsl_NODISCARD MAKESHIFT_DETAIL_FORCEINLINE constexpr gsl::index
    operator [](difference_type offset)
    {
        return *(*this + offset);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ITERATOR_HPP_
