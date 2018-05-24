
#ifndef MAKESHIFT_TYPES_HPP_
#define MAKESHIFT_TYPES_HPP_


#include <type_traits> // for declval<>(), decay_t<>, reference_wrapper<>
#include <utility>     // for forward<>()
#include <cstdint>     // for uint32_t
#include <cstddef>     // for size_t
#include <tuple>


namespace makeshift
{

namespace detail
{

struct flags_base { };

} // namespace detail


inline namespace types
{


    // Helper for type dispatching.
template <typename T = void>
    struct tag_t
{
    using type = T;
};
template <typename T = void>
    constexpr tag_t<T> tag { };



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
    struct define_flags : makeshift::detail::flags_base
{
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants

        // We just forward the metadata defined for the derived type.
    friend constexpr auto reflect(flag*, tag_t<>)
    {
        return reflect((FlagsT*) nullptr, tag<>);
    }

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


    // Type sequence (strictly for compile-time purposes).
template <typename... Ts>
    struct type_sequence
{
};


} // inline namespace types


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

struct default_overload_tag { };

template <typename... Fs>
    struct overload_base : Fs...
{
    constexpr overload_base(Fs&&... fs) : Fs(std::move(fs))... { }
    using Fs::operator ()...;
    template <typename T>
        constexpr decltype(auto) operator()(std::reference_wrapper<T> arg)
        //noexcept(noexcept(std::declval<overload_base>()(arg.get())))
    {
        return (*this)(arg.get());
    }
    template <typename T>
        constexpr decltype(auto) operator()(std::reference_wrapper<T> arg) const
        //noexcept(noexcept(std::declval<overload_base>()(arg.get())))
    {
        return (*this)(arg.get());
    }
};

template <typename F>
    struct default_overload_wrapper : F
{
    constexpr default_overload_wrapper(F&& func)
        : F(std::move(func))
    {
    }
    template <typename... Ts>
#ifdef MAKESHIFT_FANCY_DEFAULT
        constexpr decltype(auto) operator ()(default_overload_tag, Ts&&... args) const
#else // MAKESHIFT_FANCY_DEFAULT
        constexpr decltype(auto) operator ()(Ts&&... args) const
#endif // MAKESHIFT_FANCY_DEFAULT
        //noexcept(noexcept(F::operator ()(std::forward<Ts>(args)...)))
    {
        return F::operator ()(std::forward<Ts>(args)...);
    }
};
struct ignore_overload_wrapper
{
    template <typename... Ts>
#ifdef MAKESHIFT_FANCY_DEFAULT
        constexpr void operator ()(default_overload_tag, Ts&&...) const noexcept
#else // MAKESHIFT_FANCY_DEFAULT
        constexpr void operator ()(Ts&&...) const noexcept
#endif // MAKESHIFT_FANCY_DEFAULT
    {
    }
};

template <std::size_t N, typename T, std::size_t... Is>
    constexpr std::array<std::remove_cv_t<T>, N> to_array_impl(T array[], std::index_sequence<Is...>)
    noexcept(noexcept(std::remove_cv_t<T>(std::declval<T>())))
{
    return {{ array[Is]... }};
}

template <typename F, template <typename...> class T>
    struct match_template_func : F
{
    constexpr match_template_func(F&& func)
        : F(std::move(func))
    {
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(const T<Ts...>& arg)
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(const T<Ts...>& arg) const
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>& arg)
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>& arg) const
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>&& arg)
    {
        return F::operator ()(std::move(arg));
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>&& arg) const
    {
        return F::operator ()(std::move(arg));
    }
};

template <typename T, template <typename...> class U> struct is_same_template_ : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_same_template_<U<Ts...>, U> : std::true_type { };

} // namespace detail

inline namespace types
{

template <std::size_t N, typename T>
    constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&array)[N])
    noexcept(noexcept(std::remove_cv_t<T>(std::declval<T>())))
{
    return makeshift::detail::to_array_impl(array, std::make_index_sequence<N>{ });
}

struct ignore_t { };
constexpr inline ignore_t ignore { };

template <typename F>
    constexpr makeshift::detail::default_overload_wrapper<std::decay_t<F>> otherwise(F&& func)
    noexcept(noexcept(F(std::forward<F>(func))))
{
    return { std::forward<F>(func) };
}
constexpr inline makeshift::detail::ignore_overload_wrapper otherwise(ignore_t) noexcept
{
    return { };
}

template <typename... Fs>
    struct overload : makeshift::detail::overload_base<Fs...>
{
    using base = makeshift::detail::overload_base<Fs...>;
    using base::base;

#ifdef MAKESHIFT_FANCY_DEFAULT
private:
    struct test : base
    {
        using base::operator ();
        makeshift::detail::default_overload_tag operator ()(...) const;
    };

public:
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args)
    {
        using ResultType = decltype(std::declval<test>()(std::forward<Ts>(args)...));
        constexpr bool isDefaultOverload = std::is_same<ResultType, makeshift::detail::default_overload_tag>::value;
        if constexpr (isDefaultOverload)
            return base::operator ()(makeshift::detail::default_overload_tag{ }, std::forward<Ts>(args)...);
        else
            return base::operator ()(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args) const
    {
        using ResultType = decltype(std::declval<const test>()(std::forward<Ts>(args)...));
        constexpr bool isDefaultOverload = std::is_same<ResultType, makeshift::detail::default_overload_tag>::value;
        if constexpr (isDefaultOverload)
            return base::operator ()(makeshift::detail::default_overload_tag{ }, std::forward<Ts>(args)...);
        else
            return base::operator ()(std::forward<Ts>(args)...);
    }
#else // MAKESHIFT_FANCY_DEFAULT
    using base::operator ();
#endif // MAKESHIFT_FANCY_DEFAULT
};
template <typename... Ts>
    overload(Ts&&...) -> overload<std::decay_t<Ts>...>;

template <template <typename...> class T, typename F>
    constexpr makeshift::detail::match_template_func<std::decay_t<F>, T> match_template(F&& func)
{
    return { std::forward<F>(func) };
}

template <typename T, template <typename...> class U>
    using is_same_template_t = makeshift::detail::is_same_template_<T, U>;
template <typename T, template <typename...> class U>
    constexpr bool is_same_template = is_same_template_t<T, U>::value;

} // inline namespace types

namespace detail
{

template <std::size_t... Is, typename TupleT, typename F>
    constexpr void tuple_foreach_impl(std::index_sequence<Is...>, TupleT&& tuple, F&& func)
{
    (func(std::get<Is>(std::forward<TupleT>(tuple))), ...);
}
template <std::size_t... Is, typename TupleT, typename F>
    constexpr auto tuple_map_impl(std::index_sequence<Is...>, TupleT&& tuple, F&& func)
{
    return std::make_tuple(func(std::get<Is>(std::forward<TupleT>(tuple)))...);
}
template <typename F, typename T>
    struct AccumulatorWrapper
{
private:
    F& func_;
    T value_;
public:
    constexpr AccumulatorWrapper(F& _func, T _value)
        : func_(_func), value_(std::move(_value))
    {
    }
    template <typename U>
        friend constexpr auto operator +(AccumulatorWrapper<F, T>&& lhs, U&& rhs)
    {
        return AccumulatorWrapper(lhs.func_, lhs.func_(std::move(lhs.value_), std::forward<U>(rhs)));
    }
    constexpr T&& get(void) && noexcept { return std::move(value_); }
};

template <std::size_t... Is, typename TupleT, typename T, typename F>
    constexpr auto tuple_reduce_impl(std::index_sequence<Is...>, TupleT&& tuple, T&& initialValue, F&& func)
{
    return (AccumulatorWrapper(func, std::forward<T>(initialValue)) + ... + std::get<Is>(std::forward<TupleT>(tuple))).get();
}

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

template <typename TupleT, typename F,
          typename = std::enable_if_t<is_same_template<std::decay_t<TupleT>, std::tuple>>>
    constexpr void tuple_foreach(TupleT&& tuple, F&& func)
{
    makeshift::detail::tuple_foreach_impl(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<TupleT>(tuple), std::forward<F>(func));
}
template <typename TupleT, typename F,
          typename = std::enable_if_t<is_same_template<std::decay_t<TupleT>, std::tuple>>>
    constexpr auto tuple_map(TupleT&& tuple, F&& func)
{
    return makeshift::detail::tuple_map_impl(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<TupleT>(tuple), std::forward<F>(func));
}
template <typename TupleT, typename T, typename F,
          typename = std::enable_if_t<is_same_template<std::decay_t<TupleT>, std::tuple>>>
    constexpr auto tuple_reduce(TupleT&& tuple, T&& initialValue, F&& func)
{
    return makeshift::detail::tuple_reduce_impl(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ },
        std::forward<TupleT>(tuple), std::forward<T>(initialValue), std::forward<F>(func));
}

template <makeshift::detail::keyword_crc KeywordCRC, typename TupleT>
    constexpr decltype(auto) get(TupleT&& tuple) noexcept
{
    using DTuple = std::decay_t<TupleT>;
    constexpr std::size_t matchIndex = makeshift::detail::tuple_kw_index<KeywordCRC, DTuple>(std::make_index_sequence<std::tuple_size<DTuple>::value>{ });
    return get(std::get<matchIndex>(std::forward<TupleT>(tuple)));
}

} // inline namespace types

} // namespace makeshift

#endif // MAKESHIFT_TYPES_HPP_
