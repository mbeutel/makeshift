
#ifndef INCLUDED_MAKESHIFT_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_ALGORITHM_HPP_


#include <cstddef>     // for ptrdiff_t
#include <utility>     // for forward<>(), swap()
#include <iterator>    // for iterator_traits<>
#include <type_traits> // for integral_constant<>, decay<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_Expects()

#include <makeshift/detail/algorithm.hpp>
#include <makeshift/detail/range-index.hpp> // for identity_transform_t, all_of_pred, none_of_pred


namespace makeshift
{


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

        // We deliberately don't use `std::distance()` in order to support iterators with proxy reference types (which cannot implement LegacyRandomAccessIterator
        // even though they may be random-access).
    Diff length = last - first;

    for (Diff i = 0; i < length; ++i)
    {
        Diff current = i;
        while (i != indices[current])
        {
            Diff next = indices[current];
            if (next < 0 || next >= length)
            {
                    // Improve post-mortem debuggability by storing the out-of-range index in the array.
                indices[i] = next;

                gsl_Expects(false); // invalid index in permutation
            }
            if (next == current)
            {
                indices[i] = next;
                gsl_Expects(false); // not a permutation
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

        // We deliberately don't use std::distance() in order to support iterators with proxy reference types (which cannot implement LegacyRandomAccessIterator
        // even though they may be random-access).
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
    // Similar to `std::shuffle()`, but support iterators with proxy reference types such as `std::vector<bool>` or `soa_span<>` (which cannot implement
    // LegacyRandomAccessIterator even though they may be random-access), and permits passing a user-defined integer distribution.
    //ᅟ
    //ᅟ    shuffle(v.begin(), v.end(), rng,
    //ᅟ        std::uniform_int_distribution<std::ptrdiff_t>{ });
    //
template <typename RandomIt, typename URBG, typename UniformIntDistributionT>
constexpr void
shuffle(RandomIt first, RandomIt last, URBG&& rng, UniformIntDistributionT dist)
{
    // Implementation taken from https://en.cppreference.com/w/cpp/algorithm/shuffle.

    using Diff = typename std::iterator_traits<RandomIt>::difference_type;
    using Param = typename UniformIntDistributionT::param_type;
 
    Diff length = last - first;
    for (Diff i = length - 1; i > 0; --i)
    {
        Diff j = dist(rng, Param(0, i));
        if (i != j)
        {
            using std::swap;
            swap(first[i], first[j]);
        }
    }
}


    //
    // Takes a scalar procedure (i.e. a function of non-range arguments which returns nothing) and calls the procedure for every set of elements in the given ranges.
    //ᅟ
    //ᅟ    range_for(
    //ᅟ        [](gsl::index i, int val) { std::cout << "array[" << i << "]: " << val << '\n'; },
    //ᅟ        range_index, std::array{ 1, 2, 3 });
    //ᅟ    // prints "array[0]: 1\narray[1]: 2\narray[2]: 3\n"
    //
template <typename F, typename... Rs>
constexpr void
range_for(F&& func, Rs&&... ranges)
{
    static_assert(!gsl::conjunction_v<std::is_same<std::decay_t<Rs>, detail::range_index_t>...>, "no range argument and no size given");

    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    detail::range_for(mergedSize, std::forward<F>(func), ranges...);
}

    //
    // Takes a scalar procedure (i.e. a function of non-range arguments which returns nothing) and calls the procedure for every set of elements in the given ranges.
    //ᅟ
    //ᅟ    range_for_n(3,
    //ᅟ        [](gsl::index i) { std::cout << i << '\n'; },
    //ᅟ        range_index);
    //ᅟ    // prints "0\n1\n2\n"
    //
template <typename SizeC, typename F, typename... Rs>
constexpr void
range_for_n(SizeC size, F&& func, Rs&&... ranges)
{
    static_assert(std::is_convertible<SizeC, std::size_t>::value, "argument is not convertible to a size");

    auto mergedSize = detail::merge_sizes(detail::to_ptrdiff_size(size), detail::range_size(ranges)...);
    detail::range_for(mergedSize, std::forward<F>(func), ranges...);
}


    //
    // Takes an initial value, a reducer, a transformer, and a list of ranges and reduces them to a scalar value.
    //ᅟ
    //ᅟ    range_transform_reduce(
    //ᅟ        gsl::index(0),
    //ᅟ        std::plus<>{ },
    //ᅟ        [](auto&& str) { return str.length(); },
    //ᅟ        std::array{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns 13
    //
template <typename T, typename ReduceFuncT, typename TransformFuncT, typename... Rs>
gsl_NODISCARD constexpr std::decay_t<T>
range_transform_reduce(T&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Rs&&... ranges)
{
    static_assert(!gsl::conjunction_v<std::is_same<std::decay_t<Rs>, detail::range_index_t>...>, "no range argument and no size given");

    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    detail::range_transform_reduce(mergedSize, std::forward<T>(initialValue), std::forward<ReduceFuncT>(reduce), std::forward<TransformFuncT>(transform), ranges...);
}


    //
    // Takes an initial value, a reducer, and a range and reduces it to a scalar value.
    //ᅟ
    //ᅟ    range_reduce(
    //ᅟ        std::string{ },
    //ᅟ        std::plus<>{ },
    //ᅟ        std::array{ "Hello, "s, "World!"s });
    //ᅟ    // returns "Hello, World!"s;
    //
template <typename T, typename ReduceFuncT, typename R>
gsl_NODISCARD constexpr auto
range_reduce(T&& initialValue, ReduceFuncT&& reduce, R&& range)
{
    static_assert(!std::is_same<std::decay_t<R>, detail::range_index_t>::value, "no range argument and no size given");

    auto mergedSize = detail::range_size(range);
    return detail::range_transform_reduce(mergedSize, std::forward<T>(initialValue), std::forward<ReduceFuncT>(reduce), gsl::identity{ }, range);
}


    //
    // Takes a predicate and a list of ranges and counts the sets of range elements for which the predicate applies.
    //ᅟ
    //ᅟ    range_count_if(
    //ᅟ        [](auto&& str) { return !str.empty(); },
    //ᅟ        std::array{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns 2
    //
template <typename PredT, typename... Rs>
gsl_NODISCARD constexpr std::ptrdiff_t
range_count_if(PredT&& pred, Rs&&... ranges)
{
    static_assert(!gsl::conjunction_v<std::is_same<std::decay_t<Rs>, detail::range_index_t>...>, "no range argument and no size given");

    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    return detail::range_transform_reduce(mergedSize, std::ptrdiff_t(0), std::plus<std::ptrdiff_t>{ }, detail::count_if_fn<PredT>{ std::forward<PredT>(pred) }, ranges...);
}


    //
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for all sets of range elements.
    //ᅟ
    //ᅟ    range_all_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::array{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns false
    //
template <typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_all_of(PredicateT&& predicate, Rs&&... ranges)
{
    static_assert(!gsl::conjunction_v<std::is_same<std::decay_t<Rs>, detail::range_index_t>...>, "no range argument and no size given");

    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    return detail::range_conjunction<gsl::identity>(mergedSize, std::forward<PredicateT>(predicate), ranges...);
}


    //
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for any set of range elements.
    //ᅟ
    //ᅟ    range_any_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::array{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns false
    //
template <typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_any_of(PredicateT&& predicate, Rs&&... ranges)
{
    static_assert(!gsl::conjunction_v<std::is_same<std::decay_t<Rs>, detail::range_index_t>...>, "no range argument and no size given");

    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    return !detail::range_conjunction<detail::negation_fn>(mergedSize, std::forward<PredicateT>(predicate), ranges...);
}


    //
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for no set of range elements.
    //ᅟ
    //ᅟ    range_none_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::tuple{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns true
    //
template <typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_none_of(PredicateT&& predicate, Rs&&... ranges)
{
    static_assert(!gsl::conjunction_v<std::is_same<std::decay_t<Rs>, detail::range_index_t>...>, "no range argument and no size given");

    auto mergedSize = detail::merge_sizes(detail::range_size(ranges)...);
    return detail::range_conjunction<detail::negation_fn>(mergedSize, std::forward<PredicateT>(predicate), ranges...);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ALGORITHM_HPP_
