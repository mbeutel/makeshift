
#ifndef MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_
#define MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_


#include <type_traits> // for decay<>
#include <utility>     // for forward<>()
#include <cstdint>     // for uint32_t
#include <cstddef>     // for size_t
#include <tuple>       // for tuple_element<>


namespace makeshift
{

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

enum class keyword_crc : std::uint32_t { };

struct keyword
{
    const char* data;
    std::size_t size;

    constexpr operator keyword_crc(void) const noexcept
    {
        return keyword_crc(crc32c(0, data, size));
    }
};

constexpr inline keyword_crc operator +(keyword_crc lhs, keyword rhs) noexcept
{
    return keyword_crc(crc32c(std::uint32_t(lhs), rhs.data, rhs.size));
}
constexpr inline keyword_crc operator +(keyword lhs, keyword rhs) noexcept
{
    return keyword_crc(lhs) + rhs;
}
constexpr inline keyword_crc operator /(keyword_crc lhs, keyword rhs) noexcept
{
    char sep[] = { '/' };
    auto sc = keyword_crc(crc32c(std::uint32_t(lhs), sep, 1));
    return sc + rhs;
}
constexpr inline keyword_crc operator /(keyword lhs, keyword rhs) noexcept
{
    return keyword_crc(lhs) / rhs;
}

constexpr inline bool operator ==(keyword_crc lhs, keyword rhs) noexcept { return lhs == keyword_crc(rhs); }
constexpr inline bool operator !=(keyword_crc lhs, keyword rhs) noexcept { return !(lhs == rhs); }
constexpr inline bool operator ==(keyword lhs, keyword_crc rhs) noexcept { return keyword_crc(lhs) == rhs; }
constexpr inline bool operator !=(keyword lhs, keyword_crc rhs) noexcept { return !(lhs == rhs); }
constexpr inline bool operator ==(keyword lhs, keyword rhs) noexcept { return keyword_crc(lhs) == keyword_crc(rhs); }
constexpr inline bool operator !=(keyword lhs, keyword rhs) noexcept { return !(lhs == rhs); }

} // namespace detail


inline namespace types
{


    // Typed keyword args:
    // Define a named type with named<> and the ""_kw literal operator:
    //
    //     using NamedInt = named<int, "width"_kw>;
    //
    // Construct an object of a named type either with the explicit constructor, or by using name<>
    // with assignment syntax:
    //
    //     NamedInt val1 { 42 };
    //     NamedInt val2 = { name<"width"_kw> = 42 };
    //
    // Concatenate keywords with '+' (without separator) or `/` (with hierarchical separator):
    //
    //     "make"_kw + "shift"_kw == "makeshift"_kw
    //     "foo"_kw / "bar"_kw == "foo/bar"_kw
    //
    // This is currently implemented using CRC-32 to work around the inability to pass strings as template
    // arguments. This may change in C++20, cf. P0732. I hope to be able to switch to a P0732-based
    // implementation while maintaining source compatibility.


    // Wraps a value of type T with a compile-time keyword name:
    //
    //     using NamedInt = named<int, "width"_kw>;
template <typename T, makeshift::detail::keyword_crc KeywordCRC>
    struct named
{
    static constexpr makeshift::detail::keyword_crc keyword = KeywordCRC;

    T value_;

    explicit constexpr named(const T& _value)
    noexcept(noexcept(T(_value)))
        : value_(_value)
    {
    }
    explicit constexpr named(T&& _value)
    noexcept(noexcept(T(std::move(_value))))
        : value_(std::move(_value))
    {
    }

    friend constexpr const T& get(const named& self) noexcept { return self.value_; }
    friend constexpr T& get(named& self) noexcept { return self.value_; }
    friend constexpr T&& get(named&& self) noexcept { return std::move(self.value_); }
};

} // inline namespace types

namespace detail
{

template <keyword_crc KeywordCRC>
    struct name_t
{
    static constexpr keyword_crc keyword = KeywordCRC;

    template <typename T>
        constexpr named<std::decay_t<T>, keyword> operator =(T&& rhs) const
        noexcept(noexcept(std::decay_t<T>(std::forward<T>(rhs))))
    {
        return named<std::decay_t<T>, keyword> { std::forward<T>(rhs) };
    }
};

template <keyword_crc KeywordCRC, typename T>
    struct has_name : std::false_type
{
};
template <keyword_crc KeywordCRC, typename T>
    struct has_name<KeywordCRC, named<T, KeywordCRC>> : std::true_type
{
    using named_type = named<T, KeywordCRC>;
    using element_type = T;
};

template <makeshift::detail::keyword_crc KeywordCRC, typename TupleT, std::size_t... Is>
    constexpr std::size_t tuple_kw_index(std::index_sequence<Is...>) noexcept
{
    constexpr int numMatches = (0 + ... + (has_name<KeywordCRC, std::tuple_element_t<Is, TupleT>>::value ? 1 : 0));
    constexpr std::size_t matchIndex = (0 + ... + (has_name<KeywordCRC, std::tuple_element_t<Is, TupleT>>::value ? Is : 0));
    static_assert(numMatches >= 1, "argument with given keyword not found in tuple");
    static_assert(numMatches <= 1, "more than one keyword arguments match the given keyword");
    return matchIndex;
}

} // namespace detail

inline namespace types
{

    // Permits constructing an object of named type with familiar assignment syntax:
    //
    //     name<"width"_kw> = 42
template <makeshift::detail::keyword_crc KeywordCRC>
    constexpr inline makeshift::detail::name_t<KeywordCRC> name { };

} // inline namespace types

inline namespace literals
{

constexpr inline makeshift::detail::keyword operator ""_kw(const char* data, std::size_t size) noexcept
{
    return { data, size };
}

} // inline namespace literals

namespace detail
{


} // namespace detail

inline namespace types
{

template <makeshift::detail::keyword_crc KeywordCRC, typename TupleT>
    constexpr decltype(auto) get(TupleT&& tuple) noexcept
{
    using DTuple = std::decay_t<TupleT>;
    constexpr std::size_t matchIndex = makeshift::detail::tuple_kw_index<KeywordCRC, DTuple>(std::make_index_sequence<std::tuple_size<DTuple>::value>{ });
    return get(std::get<matchIndex>(std::forward<TupleT>(tuple)));
}

} // inline namespace types

} // namespace makeshift

#endif // MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_
