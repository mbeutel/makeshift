
#ifndef INCLUDED_MAKESHIFT_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_ALGORITHM_HPP_


#include <cstddef>     // for ptrdiff_t
#include <utility>     // for forward<>(), swap()
#include <iterator>    // for iterator_traits<>
#include <type_traits> // for integral_constant<>, decay<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/detail/algorithm.hpp>
#include <makeshift/detail/range-index.hpp> // for identity_transform_t, all_of_pred, none_of_pred
#include <makeshift/detail/type_traits.hpp> // for disjunction<>


namespace makeshift
{


    //ᅟ
    // Permutes the range of elements [first, last) using the permutation given by the index range.
    //
template <typename RandomIt, typename IndexRandomIt>
constexpr void
apply_permutation(RandomIt first, RandomIt last, IndexRandomIt indices)
{
    // Implementation based on Raymond Chen's article at https://devblogs.microsoft.com/oldnewthing/20170110-00/?p=95155.

    using Diff = typename std::iterator_traits<IndexRandomIt>::value_type;

    using std::swap;

        // We deliberately don't use std::distance() in order to support iterators with proxy reference types (which cannot implement LegacyRandomAccessIterator
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
                indices[i] = next; // Improve post-mortem debuggability by storing the out-of-range index in the array.
                Expects(false); // invalid index in permutation
            }
            if (next == current)
            {
                indices[i] = next;
                Expects(false); // not a permutation
            }
            swap(first[current], first[next]);
            indices[current] = current;
            current = next;
        }
        indices[current] = current;
    }
}

    //ᅟ
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
            Expects(next >= 0 && next < length); // make sure index is valid
            Expects(next != indices[next]); // make sure this is actually a permutation, which isn't the case if an index occurs more than once
            swap(first[i], first[next]);
            swap(indices[i], indices[next]);
        }
    }
}


    //ᅟ
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



    //ᅟ
    // Takes a scalar procedure (i.e. a function of non-range arguments which returns nothing) and calls the procedure for every set of elements in the given ranges.
    //ᅟ
    //ᅟ    range_for(
    //ᅟ        [](index i, int val) { std::cout << "array[" << i << "]: " << val << '\n'; },
    //ᅟ        range_index, std::array{ 1, 2, 3 });
    //ᅟ    // prints "array[0]: 1\narray[1]: 2\narray[2]: 3\n"
    //
template <typename F, typename... Rs>
constexpr void
range_for(F&& func, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::static_ranges_size<Rs...>();
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    detail::range_for(detail::ranges_size<staticSize>(ranges...), std::forward<F>(func), ranges...);
}

    //ᅟ
    // Takes a scalar procedure (i.e. a function of non-range arguments which returns nothing) and calls the procedure for every set of elements in the given ranges.
    //ᅟ
    //ᅟ    range_for<3>(
    //ᅟ        [](index i) { std::cout << i << '\n'; },
    //ᅟ        range_index);
    //ᅟ    // prints "0\n1\n2\n"
    //
template <std::size_t N, typename F, typename... Rs>
constexpr void
range_for(F&& func, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::merge_sizes(detail::static_ranges_size<Rs...>(), N);
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    detail::range_for(detail::ranges_size<staticSize>(ranges...), std::forward<F>(func), ranges...);
}


    //ᅟ
    // Takes an initial value, a reducer, a transformer, and a list of ranges and reduces them to a scalar value.
    //ᅟ
    //ᅟ    range_transform_reduce(
    //ᅟ        std::size_t(0),
    //ᅟ        std::plus<>{ },
    //ᅟ        [](auto&& str) { return str.length(); },
    //ᅟ        std::array{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns 13
    //
template <typename T, typename ReduceFuncT, typename TransformFuncT, typename... Rs>
gsl_NODISCARD constexpr std::decay_t<T>
range_transform_reduce(T&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::static_ranges_size<Rs...>();
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_transform_reduce(detail::ranges_size<staticSize>(ranges...),
        std::forward<T>(initialValue), std::forward<ReduceFuncT>(reduce), std::forward<TransformFuncT>(transform), ranges...);
}


    //ᅟ
    // Takes an initial value, a reducer, a transformer, and a list of ranges and reduces them to a scalar value.
    //ᅟ
    //ᅟ    range_transform_reduce<3>(
    //ᅟ        std::size_t(0),
    //ᅟ        std::plus<std::size_t>{ },
    //ᅟ        [](std::size_t i) { return i*i; },
    //ᅟ        range_index);
    //ᅟ    // returns 5
    //
template <std::size_t N, typename T, typename ReduceFuncT, typename TransformFuncT, typename... Rs>
gsl_NODISCARD constexpr std::decay_t<T>
range_transform_reduce(T&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::merge_sizes(detail::static_ranges_size<Rs...>(), N);
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_transform_reduce(detail::ranges_size<staticSize>(ranges...),
        std::forward<T>(initialValue), std::forward<ReduceFuncT>(reduce), std::forward<TransformFuncT>(transform), ranges...);
}


    //ᅟ
    // Takes an initial value, a reducer, and a tuple and reduces them to a scalar value.
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
    constexpr std::ptrdiff_t staticSize = detail::static_ranges_size<R>();
    return detail::range_transform_reduce(detail::ranges_size<staticSize>(range),
        std::forward<T>(initialValue), std::forward<ReduceFuncT>(reduce), detail::identity_transform_t{ }, range);
}


    //ᅟ
    // Takes an initial value, a reducer, and a tuple and reduces them to a scalar value.
    //ᅟ
    //ᅟ    range_reduce<4>(
    //ᅟ        std::size_t(0),
    //ᅟ        std::plus<std::size_t>{ },
    //ᅟ        range_index);
    //ᅟ    // returns 6
    //
template <std::size_t N, typename T, typename ReduceFuncT, typename R>
gsl_NODISCARD constexpr auto
range_reduce(T&& initialValue, ReduceFuncT&& reduce, R&& range)
{
    constexpr std::ptrdiff_t staticSize = detail::merge_sizes(detail::static_ranges_size<R>(), N);
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_transform_reduce(detail::ranges_size<staticSize>(range),
        std::forward<T>(initialValue), std::forward<ReduceFuncT>(reduce), detail::identity_transform_t{ }, range);
}


    //ᅟ
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for all sets of tuple elements.
    //ᅟ
    //ᅟ    range_all_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::tuple{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns false
    //
template <typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_all_of(PredicateT&& predicate, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::static_ranges_size<Rs...>();
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_conjunction<detail::all_of_pred>(detail::ranges_size<staticSize>(ranges...),
        std::forward<PredicateT>(predicate), ranges...);
}


    //ᅟ
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for all sets of tuple elements.
    //ᅟ
    //ᅟ    range_all_of<3>(
    //ᅟ        [](std::size_t i) { return isPrime(i + 1); },
    //ᅟ        range_index);
    //
template <std::size_t N, typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_all_of(PredicateT&& predicate, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::merge_sizes(detail::static_ranges_size<Rs...>(), N);
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_conjunction<detail::all_of_pred>(detail::ranges_size<staticSize>(ranges...),
        std::forward<PredicateT>(predicate), ranges...);
}


    //ᅟ
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for any set of tuple elements.
    //ᅟ
    //ᅟ    range_any_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::tuple{ "Hello, "sv, "World!"sv });
    //ᅟ    // returns false
    //
template <typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_any_of(PredicateT&& predicate, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::static_ranges_size<Rs...>();
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return !detail::range_conjunction<detail::none_of_pred>(detail::ranges_size<staticSize>(ranges...),
        std::forward<PredicateT>(predicate), ranges...);
}


    //ᅟ
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for all sets of tuple elements.
    //ᅟ
    //ᅟ    range_any_of<3>(
    //ᅟ        [](std::size_t i) { return isPrime(i + 1); },
    //ᅟ        range_index);
    //ᅟ    // returns true
    //
template <std::size_t N, typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_any_of(PredicateT&& predicate, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::merge_sizes(detail::static_ranges_size<Rs...>(), N);
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return !detail::range_conjunction<detail::none_of_pred>(detail::ranges_size<staticSize>(ranges...),
        std::forward<PredicateT>(predicate), ranges...);
}


    //ᅟ
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for no set of tuple elements.
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
    constexpr std::ptrdiff_t staticSize = detail::static_ranges_size<Rs...>();
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_conjunction<detail::none_of_pred>(detail::ranges_size<staticSize>(ranges...),
        std::forward<PredicateT>(predicate), ranges...);
}


    //ᅟ
    // Takes a predicate and a list of ranges and returns whether the predicate is satisfied for no set of tuple elements.
    //ᅟ
    //ᅟ    range_none_of<3>(
    //ᅟ        [](std::size_t i) { return isPrime(i); },
    //ᅟ        range_index);
    //ᅟ    // returns false
    //
template <std::size_t N, typename PredicateT, typename... Rs>
gsl_NODISCARD constexpr bool
range_none_of(PredicateT&& predicate, Rs&&... ranges)
{
    constexpr std::ptrdiff_t staticSize = detail::merge_sizes(detail::static_ranges_size<Rs...>(), N);
    static_assert(staticSize != detail::inconsistent_size, "ranges have inconsistent sizes");
    return detail::range_conjunction<detail::none_of_pred>(detail::ranges_size<staticSize>(ranges...),
        std::forward<PredicateT>(predicate), ranges...);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ALGORITHM_HPP_
