
#ifndef INCLUDED_MAKESHIFT_ALGORITHM_HPP_
#define INCLUDED_MAKESHIFT_ALGORITHM_HPP_


#include <iterator> // for begin(), end()


namespace makeshift
{


    //ᅟ
    // Computes the conjunction of the given range of booleans.
    //
struct all_of_fn
{
    template <typename It, typename EndIt>
    constexpr bool operator ()(It first, EndIt last) const
    {
        for (; first != last; ++first)
        {
            if (!*first) return false;
        }
        return true;
    }

    template <typename R>
    constexpr bool operator ()(R&& range) const
    {
        using std::begin;
        using std::end;
        return (*this)(begin(range), end(range));
    }
};
 
constexpr inline all_of_fn all_of;


    //ᅟ
    // Computes the disjunction of the given range of booleans.
    //
struct any_of_fn
{
    template <typename It, typename EndIt>
    constexpr bool operator ()(It first, EndIt last) const
    {
        for (; first != last; ++first)
        {
            if (*first) return true;
        }
        return false;
    }

    template <typename R>
    constexpr bool operator ()(R&& range) const
    {
        using std::begin;
        using std::end;
        return (*this)(begin(range), end(range));
    }
};
 
constexpr inline any_of_fn any_of;


    //ᅟ
    // Computes the negated disjunction of the given range of booleans.
    //
struct none_of_fn
{
    template <typename It, typename EndIt>
    constexpr bool operator ()(It first, EndIt last) const
    {
        return !any_of(first, last);
    }

    template <typename R>
    constexpr bool operator ()(R&& range) const
    {
        using std::begin;
        using std::end;
        return (*this)(begin(range), end(range));
    }
};
 
constexpr inline none_of_fn none_of;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ALGORITHM_HPP_
