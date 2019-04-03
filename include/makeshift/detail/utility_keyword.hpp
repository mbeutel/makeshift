
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_


#include <type_traits> // for decay<>
#include <utility>     // for forward<>()
#include <cstdint>     // for uint32_t
#include <cstddef>     // for size_t


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

enum class keyword_crc : std::uint32_t { none = 0 };

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
    // Define a named type with `named_t<>` and the `""_k` literal operator:
    //ᅟ
    //ᅟ    using Width = named_t<int, "width"_k>;
    //ᅟ
    // Construct an object of a named type either with the explicit constructor, or by using `name_v<>`
    // with assignment syntax:
    //ᅟ
    //ᅟ    Width w1 { 42 };
    //ᅟ    Width w2 = { name_v<"width"_k> = 42 };
    //ᅟ
    // Concatenate keywords with '+':
    //ᅟ
    //ᅟ    "make"_k + "shift"_k == "makeshift"_k
    //ᅟ
    // This is currently implemented using CRC-32 to work around the inability to pass strings as template
    // arguments. This may change in C++20, cf. P0732. I hope to be able to switch to a P0732-based
    // implementation while maintaining source compatibility.


    //ᅟ
    // Wraps a value of type `T` with a compile-time keyword name and a scope:
    //ᅟ
    //ᅟ    using Width = named_t<int, "width"_k>;
    //ᅟ    using LDist = scoped_t<int, "left"_k>;
    //ᅟ    using RDist = scoped_t<int, "right"_k>;
    //
template <typename T, makeshift::detail::keyword_crc Name, makeshift::detail::keyword_crc... Scopes>
    struct contextual
{
    T value_;

    explicit constexpr contextual(const T& _value) : value_(_value) { }
    explicit constexpr contextual(T&& _value) : value_(std::move(_value)) { }

    friend constexpr const T& get(const contextual& self) noexcept { return self.value_; }
    friend constexpr T& get(contextual& self) noexcept { return self.value_; }
    friend constexpr T&& get(contextual&& self) noexcept { return std::move(self.value_); }
    constexpr const T& get(void) const & noexcept { return value_; }
    constexpr T& get(void) & noexcept { return value_; }
    constexpr T&& get(void) && noexcept { return std::move(value_); }
};


    //ᅟ
    // Wraps a value of type `T` with a compile-time keyword scope:
    //ᅟ
    //ᅟ    using SrcFile = scoped_t<File, "src"_k>;
    //ᅟ    using DstFile = scoped_t<File, "dst"_k>;
    //
template <typename T, makeshift::detail::keyword_crc... Scopes> struct scoped { using type = contextual<T, makeshift::detail::keyword_crc{ }, Scopes...>; };
template <typename T, makeshift::detail::keyword_crc Name, makeshift::detail::keyword_crc... OldScopes, makeshift::detail::keyword_crc... NewScopes> struct scoped<contextual<T, Name, OldScopes...>, NewScopes...> { using type = contextual<T, Name, NewScopes..., OldScopes...>; };

    //ᅟ
    // Wraps a value of type `T` with a compile-time keyword scope:
    //ᅟ
    //ᅟ    using SrcFile = scoped_t<File, "src"_k>;
    //ᅟ    using DstFile = scoped_t<File, "dst"_k>;
    //
template <typename T, makeshift::detail::keyword_crc... Scopes> using scoped_t = typename scoped<T, Scopes...>::type;


    //ᅟ
    // Wraps a value of type `T` with a compile-time keyword name:
    //ᅟ
    //ᅟ    using Width = named_t<int, "width"_k>;
    //
template <typename T, makeshift::detail::keyword_crc Name> struct named { using type = contextual<T, Name>; };
template <typename T, makeshift::detail::keyword_crc OldName, makeshift::detail::keyword_crc... Scopes, makeshift::detail::keyword_crc NewName> struct named<contextual<T, OldName, Scopes...>, NewName>
{
    static_assert(OldName == NewName || OldName == makeshift::detail::keyword_crc{ }, "cannot assign a different name to a named type");
    using type = contextual<T, NewName, Scopes...>;
};

    //ᅟ
    // Wraps a value of type `T` with a compile-time keyword name:
    //ᅟ
    //ᅟ    using Width = named_t<int, "width"_k>;
    //
template <typename T, makeshift::detail::keyword_crc Name> using named_t = typename named<T, Name>::type;


    //ᅟ
    // Retrieves the value type of a possibly contextual type.
    //ᅟ
    //ᅟ    using Width = named_t<int, "width"_k>;
    //ᅟ    using RawWidth = unwrap_contextual_t<Width>; // RawWidth == int
    //
template <typename T> struct unwrap_contextual { using type = T; };
template <typename T, makeshift::detail::keyword_crc Name, makeshift::detail::keyword_crc... Scopes> struct unwrap_contextual<contextual<T, Name, Scopes...>> { using type = T; };

    //ᅟ
    // Retrieves the value type of a possibly contextual type.
    //ᅟ
    //ᅟ    using Width = named_t<int, "width"_k>;
    //ᅟ    using RawWidth = unwrap_contextual_t<Width>; // RawWidth == int
    //
template <typename T> using unwrap_contextual_t = typename unwrap_contextual<T>::type;


    //ᅟ
    // Retrieves the name of a possibly contextual type. The name of non-contextual types is `""_k`.
    //
template <typename T> struct name_of { static constexpr makeshift::detail::keyword_crc value = { }; };
template <typename T, makeshift::detail::keyword_crc Name, makeshift::detail::keyword_crc... Scopes> struct name_of<contextual<T, Name, Scopes...>> { static constexpr makeshift::detail::keyword_crc value = Name; };

    //ᅟ
    // Retrieves the name of a possibly contextual type. The name of non-contextual types is `""_k`.
    //
template <typename T> constexpr makeshift::detail::keyword_crc name_of_v = name_of<T>::value;


    //ᅟ
    // Determines whether `T` is a contextual type.
    //
template <typename T> struct is_contextual : std::false_type { };
template <typename T, makeshift::detail::keyword_crc Name, makeshift::detail::keyword_crc... Scopes> struct is_contextual<contextual<T, Name, Scopes...>> : std::true_type { };

    //ᅟ
    // Determines whether `T` is a contextual type.
    //
template <typename T> constexpr bool is_contextual_v = is_contextual<T>::value;


    //ᅟ
    // Retrieves the underlying value from an object of contextual type, or returns the value itself if it is not of contextual type.
    //
template <typename T>
    constexpr unwrap_contextual_t<std::decay_t<T>> contextual_value(T&& value)
{
    if constexpr (is_contextual_v<std::decay_t<T>>)
        return std::forward<T>(value).get();
    else
        return std::forward<T>(value);
}


} // inline namespace types


namespace detail
{


template <keyword_crc Name, keyword_crc... Scopes>
    struct key
{
    template <typename T>
        constexpr contextual<std::decay_t<T>, Name, Scopes...> operator =(T&& rhs) const
    {
        return contextual<std::decay_t<T>, Name, Scopes...>{ std::forward<T>(rhs) };
    }
};


} // namespace detail


inline namespace types
{


    //ᅟ
    // Permits constructing an object of contextual type with familiar assignment syntax:
    //ᅟ
    //ᅟ    scope_v<"src"_k> = srcFile
    //ᅟ    name_v<"filename"_k> | scope_v<"src"_k> = "/src/file/path"
    //ᅟ    scope_v<"outer"_k> / scope_v<"inner"_k> = 42
    //ᅟ    scope_v<"outer"_k, "inner"_k> = 42
    //
template <makeshift::detail::keyword_crc... Scopes>
    struct scope : makeshift::detail::key<makeshift::detail::keyword_crc::none, Scopes...>
{
    using makeshift::detail::key<makeshift::detail::keyword_crc::none, Scopes...>::operator =;
    template <makeshift::detail::keyword_crc LName, makeshift::detail::keyword_crc... LScopes>
        friend constexpr makeshift::detail::key<LName, LScopes..., Scopes...> operator /(makeshift::detail::key<LName, LScopes...>, scope) noexcept
    {
        return { };
    }
};
template <makeshift::detail::keyword_crc... Scopes> constexpr scope<Scopes...> scope_v = { };


    //ᅟ
    // Permits constructing an object of contextual type with familiar assignment syntax:
    //ᅟ
    //ᅟ    name_v<"width"_k> = 42
    //ᅟ    name_v<"filename"_k> | scope_v<"src"_k> = "/src/file/path"
    //
template <makeshift::detail::keyword_crc Name>
    struct name_ : makeshift::detail::key<Name>
{
    using makeshift::detail::key<Name>::operator =;
    template <makeshift::detail::keyword_crc... RScopes>
        friend constexpr makeshift::detail::key<Name, RScopes...> operator |(name_, scope<RScopes...>) noexcept
    {
        return { };
    }
    operator makeshift::detail::keyword_crc(void) const noexcept { return Name; }
};
template <makeshift::detail::keyword_crc Name> constexpr name_<Name> name_v = { };


} // inline namespace types


inline namespace literals
{


constexpr inline makeshift::detail::keyword operator ""_k(const char* data, std::size_t size) noexcept
{
    return { data, size };
}


} // inline namespace literals

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_TUPLE_HPP_
 #include <makeshift/detail/utility_keyword-tuple.hpp>
#endif // INCLUDED_MAKESHIFT_TUPLE_HPP_


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_
