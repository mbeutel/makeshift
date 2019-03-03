
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_


#include <utility>     // for move()


namespace makeshift
{

inline namespace metadata
{


template <typename T>
    struct named2;


} // inline namespace metadata


namespace detail
{


template <typename T> struct unwrap_named_ { using type = T; };
template <typename T> struct unwrap_named_<named2<T>> { using type = T; };
template <typename T>
    constexpr named2<T> wrap_named(T value) noexcept
{
    return { std::move(value), { } };
}
template <typename T>
    constexpr named2<T> wrap_named(named2<T> value) noexcept
{
    return { std::move(value) };
}

struct value_metadata2_base { };
struct compound_metadata2_base { };

template <typename ValuesT>
    struct raw_value_metadata : private value_metadata2_base
{
    ValuesT values;

    constexpr raw_value_metadata(ValuesT _values)
        : values{ std::move(_values) }
    {
    }
};
template <>
    struct raw_value_metadata<void> : private value_metadata2_base
{
};


template <typename MembersT>
    struct raw_compound_metadata : private compound_metadata2_base
{
    MembersT members;

    constexpr raw_compound_metadata(MembersT _members)
        : members{ std::move(_members) }
    {
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
