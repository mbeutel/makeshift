
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_ALGORITHM_HPP_


#include <utility>     // for swap()
#include <iterator>    // for iterator_traits<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_Expects()

#include <makeshift/detail/algorithm.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Permutes the range of elements [first, last) using the permutation given by the index range.
    //
template <typename RandomIt, typename IndexRandomIt>
constexpr void
apply_permutation(RandomIt first, RandomIt last, IndexRandomIt indices)
{
    // Implementation based on Raymond Chen's article at https://devblogs.microsoft.com/oldnewthing/20170110-00/?p=95155.

    using Diff = typename std::iterator_traits<IndexRandomIt>::value_type;

    using std::swap;

        // We deliberately don't use `std::distance()` in order to support iterators with proxy reference types (which cannot
        // implement LegacyRandomAccessIterator even though they may be random-access).
    Diff length = last - first;

    for (Diff i = 0; i < length; ++i)
    {
        Diff current = i;
        while (i != indices[current])
        {
            Diff next = indices[current];
            if (next < 0 || next >= length || next == current)
            {
                    // Improve post-mortem debuggability by storing the out-of-range index in the array.
                indices[i] = next;
                gsl_Expects(false); // invalid index or duplicate index
            }
            swap(first[current], first[next]);
            indices[current] = current;
            current = next;
        }
        indices[current] = current;
    }
}

    //
    // Permutes the range of elements [first, last) using the inverse of the permutation given by the index range.
    //
template <typename RandomIt, typename IndexRandomIt>
constexpr void
apply_reverse_permutation(RandomIt first, RandomIt last, IndexRandomIt indices)
{
    // Implementation based on Raymond Chen's article at https://devblogs.microsoft.com/oldnewthing/20170111-00/?p=95165.

    using Diff = typename std::iterator_traits<IndexRandomIt>::value_type;

    using std::swap;

        // We deliberately don't use `std::distance()` in order to support iterators with proxy reference types (which cannot
        // implement LegacyRandomAccessIterator even though they may be random-access).
    Diff length = last - first;

    for (Diff i = 0; i < length; ++i)
    {
        while (i != indices[i])
        {
            Diff next = indices[i];
            gsl_Expects(next >= 0 && next < length); // make sure index is valid
            gsl_Expects(next != indices[next]); // make sure this is actually a permutation, which isn't the case if an index occurs more than once
            swap(first[i], first[next]);
            swap(indices[i], indices[next]);
        }
    }
}


    //
    // Given a list of ranges, returns a range of tuples. The range returns a sentinel as end iterator.
    //ᅟ
    //ᅟ    for (auto&& [i, val] : range_zip(range_index, std::array{ 1, 2, 3 })) {
    //ᅟ        std::cout << "array[" << i << "]: " << val << '\n';
    //ᅟ    }
    //ᅟ    // prints "array[0]: 1\narray[1]: 2\narray[2]: 3\n"
    //
template <typename... Rs>
constexpr auto
range_zip_sentinel(Rs&&... ranges)
{
    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    static_assert(!std::is_same<decltype(mergedSize), detail::dim_constant<detail::unknown_size>>::value, "no range argument and no size given");

    return detail::make_zip_range(mergedSize, std::forward<Rs>(ranges)...);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_ALGORITHM_HPP_
