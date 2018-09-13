
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_HASH_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_HASH_HPP_


#include <functional> // for hash<>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Like `std::hash<>` but permits conditional specialization with `enable_if<>`.
    // This is required to support specializations for e.g. constrained integers.
    //
template <typename KeyT, typename = void> struct hash : std::hash<KeyT> { };


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_HASH_HPP_
