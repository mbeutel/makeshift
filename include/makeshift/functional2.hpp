
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_


#include <functional> // for hash<>

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/functional2.hpp> // for hashable_base


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Similar to `std::hash<>` but permits omitting the type argument and conditional specialization with `enable_if<>`.
    //
template <typename KeyT = void, typename = void>
    struct hash2 : std::hash<KeyT>
{
    using std::hash<KeyT>::hash;
};
template <>
    struct hash2<void>
{
        //ᅟ
        // Computes a hash value of the given argument.
        //
    template <typename T,
              typename = decltype(hash2<T>{ }(std::declval<const T&>()))>
        MAKESHIFT_NODISCARD constexpr std::size_t operator ()(const T& arg) const noexcept
    {
        return hash2<T>{ }(arg);
    }
};


    //ᅟ
    // Represents the ability of a type to be compared for equality with operators `==` and `!=`.
    //
struct equatable
{
    using default_operation = std::equal_to<>;

    template <typename EqualToT = default_operation>
        struct interface
    {
        template <typename T,
                  typename = decltype(EqualToT{ }(std::declval<const T&>(), std::declval<const T&>()))>
            MAKESHIFT_NODISCARD friend constexpr bool operator ==(const T& lhs, const T& rhs) noexcept
        {
            return EqualToT{ }(lhs, rhs);
        }
        template <typename T,
                  typename = decltype(!(std::declval<const T&>() == std::declval<const T&>()))>
            MAKESHIFT_NODISCARD friend constexpr bool operator !=(const T& lhs, const T& rhs) noexcept
        {
            return !(rhs == lhs);
        }
    };
};


    //ᅟ
    // Represents the ability of a type to be hashed using `hash<>`.
    //
struct hashable
{
    using default_operation = hash2<>;

    template <typename HashT = default_operation>
        struct interface : private makeshift::detail::hashable_base
    {
        using hashable_hash = HashT;
    };
};

template <typename T>
    struct hash2<T, std::enable_if_t<std::is_base_of<makeshift::detail::hashable_base, T>::value>>
        : T::hashable_hash
{
    using T::hashable_hash::hashable_hash;
};


    //ᅟ
    // Represents the ability of a type to be compared for order with operators `<`, `<=`, `>`, and `>=`.
    //
struct comparable
{
    using default_operation = std::less<>;

    template <typename LessT = default_operation>
        struct interface
    {
        template <typename T,
                  typename = decltype(LessT{ }(std::declval<const T&>(), std::declval<const T&>()))>
            MAKESHIFT_NODISCARD friend constexpr bool operator <(const T& lhs, const T& rhs) noexcept
        {
            return LessT{ }(lhs, rhs);
        }
        template <typename T,
                  typename = decltype(!(std::declval<const T&>() < std::declval<const T&>()))>
            MAKESHIFT_NODISCARD friend constexpr bool operator <=(const T& lhs, const T& rhs) noexcept
        {
            return !(rhs < lhs); // define in terms of LessT
        }
        template <typename T,
                  typename = decltype(std::declval<const T&>() < std::declval<const T&>())>
            MAKESHIFT_NODISCARD friend constexpr bool operator >(const T& lhs, const T& rhs) noexcept
        {
            return (rhs < lhs);
        }
        template <typename T,
                  typename = decltype(!(std::declval<const T&>() < std::declval<const T&>()))>
            MAKESHIFT_NODISCARD friend constexpr bool operator >=(const T& lhs, const T& rhs) noexcept
        {
            return !(lhs < rhs);
        }
    };
};


    //ᅟ
    // Base class which implements the interface specified abilities based on the primary interface of the ability.
    //
    // For example, to define a type with equality and ordering, inherit from `with_operations<equatable, comparable>` and implement operators `==` and `<`.
    //
template <typename... Ts>
    struct with_operations
        : Ts::template interface<>...
{
};


/*
TODO:
Req' for op_tag:

Interface:
- { op_tag = op... } -> op_stack
- op_stack | ... -> op_stack
- op_tag(op_stack) -> op

Implementation:
- fallback, if any
- ...that's it?

ops:
- to_string
- to_stream, from_stream
- memory_usage
- less, equal_to, hash


We can generalize the base class if
- we know op_tag -> compound_op
- we can define 



- default impl, or compiler error if not available
- impl (possibly templatized) for known types
- ability to inject custom impl
*/


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
