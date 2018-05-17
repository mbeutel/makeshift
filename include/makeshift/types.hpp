
#ifndef MAKESHIFT_TYPES_HPP_
#define MAKESHIFT_TYPES_HPP_


#include <type_traits> // for decay_t<>
#include <utility> // for forward<>()
#include <cstdint> // for uint32_t
#include <cstddef> // for size_t


namespace makeshift
{

inline namespace types
{


    // Inherit from define_flags<> to define a flag enum type:
    // 
    //     struct Vegetable : define_flags<Vegetable>
    //     {
    //         static constexpr flag tomato { 1 };
    //         static constexpr flag onion { 2 };
    //         static constexpr flag eggplant { 4 };
    //         static constexpr flag garlic { 8 };
    //     };
    //     using Vegetables = Vegetable::flags;
template <typename FlagsT, typename UnderlyingTypeT = unsigned>
    struct define_flags
{
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants
    
    friend constexpr flags operator |(flags lhs, flags rhs) noexcept { return flags(UnderlyingTypeT(lhs) | UnderlyingTypeT(rhs)); }
    friend constexpr flags operator &(flags lhs, flags rhs) noexcept { return flags(UnderlyingTypeT(lhs) & UnderlyingTypeT(rhs)); }
    friend constexpr flags operator ^(flags lhs, flags rhs) noexcept { return flags(UnderlyingTypeT(lhs) ^ UnderlyingTypeT(rhs)); }
    friend constexpr flags operator ~(flags arg) noexcept { return flags(~UnderlyingTypeT(arg)); }
    friend constexpr flags& operator |=(flags& lhs, flags rhs) noexcept { lhs = lhs | rhs; return lhs; }
    friend constexpr flags& operator &=(flags& lhs, flags rhs) noexcept { lhs = lhs & rhs; return lhs; }
    friend constexpr flags& operator ^=(flags& lhs, flags rhs) noexcept { lhs = lhs ^ rhs; return lhs; }
    friend constexpr bool hasFlag(flags _flags, flag _flag) noexcept { return (UnderlyingTypeT(_flags) & UnderlyingTypeT(_flag)) != 0; }
    friend constexpr bool hasAnyOf(flags _flags, flags desiredFlags) noexcept { return (UnderlyingTypeT(_flags) & UnderlyingTypeT(desiredFlags)) != 0; }
    friend constexpr bool hasAllOf(flags _flags, flags desiredFlags) noexcept { return flags(UnderlyingTypeT(_flags) & UnderlyingTypeT(desiredFlags)) == desiredFlags; }
};


    // Helper for type dispatching.
template <typename T>
    struct tag
{
    using type = T;
};



namespace detail
{

    // adapted from Mark Adler's post at https://stackoverflow.com/a/27950866
static constexpr std::uint32_t crc32c(std::uint32_t crc, const char* buf, std::size_t len) noexcept
{
        // CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order.
    constexpr std::uint32_t poly = 0xedb88320u;

    crc = ~crc;
    while (len--)
    {
        crc ^= (unsigned char) *buf++;
        for (int k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
    }
    return ~crc;
}

enum class key_crc : std::uint32_t { };

struct key_name
{
    const char* data;
    std::size_t size;

    constexpr operator key_crc(void) const noexcept
    {
        return key_crc(crc32c(0, data, size));
    }
};

constexpr inline key_crc operator +(key_crc lhs, key_name rhs) noexcept
{
    return key_crc(crc32c(std::uint32_t(lhs), rhs.data, rhs.size));
}
constexpr inline key_crc operator +(key_name lhs, key_name rhs) noexcept
{
    return key_crc(lhs) + rhs;
}
constexpr inline key_crc operator /(key_crc lhs, key_name rhs) noexcept
{
    char sep[] = { '/' };
    auto sc = key_crc(crc32c(std::uint32_t(lhs), sep, 1));
    return sc + rhs;
}
constexpr inline key_crc operator /(key_name lhs, key_name rhs) noexcept
{
    return key_crc(lhs) / rhs;
}

} // namespace detail


    // Named object wrapper.
    // Use with the ""_kn literal defined below:
    //
    //     using NamedInt = named<int, "width"_kn>;
    //
    // Construct an object of a named type either with the explicit constructor, or by using name<>
    // with assignment syntax:
    //
    //     NamedInt val1 { 42 };
    //     NamedInt val2 = { name<"width"_kn> = 42 };
    //
    // This is currently implemented using CRC-32 to work around the inability to pass strings as template
    // arguments. This may change in C++20, cf. P0732. I hope to be able to switch to a P0732-based
    // implementation while maintaining source compatibility.
template <typename T, makeshift::detail::key_crc KeyCRC>
    struct named
{
    T value;

    explicit constexpr named(const T& _value)
    noexcept(noexcept(T(_value)))
        : value(_value)
    {
    }
    explicit constexpr named(T&& _value)
    noexcept(noexcept(T(std::move(_value))))
        : value(std::move(_value))
    {
    }
};

namespace detail
{

template <key_crc KeyCRC>
    struct key
{
    static constexpr key_crc value = KeyCRC;

    template <typename T>
        constexpr named<std::decay_t<T>, value> operator =(T&& rhs) const
        noexcept(noexcept(std::decay_t<T>(std::forward<T>(rhs))))
    {
        return named<std::decay_t<T>, value> { std::forward<T>(rhs) };
    }
};

} // namespace detail

template <makeshift::detail::key_crc KeyCRC>
    constexpr inline makeshift::detail::key<KeyCRC> name { };

} // inline namespace types

inline namespace literals
{

constexpr inline makeshift::detail::key_name operator ""_kn(const char* data, std::size_t size) noexcept
{
    return { data, size };
}

} // inline namespace literals

} // namespace makeshift

#endif // MAKESHIFT_TYPES_HPP_
