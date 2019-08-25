
#ifndef INCLUDED_MAKESHIFT_QUANTITY_HPP_
#define INCLUDED_MAKESHIFT_QUANTITY_HPP_


#include <cstdint>     // for uint64_t
#include <type_traits> // for enable_if<>, is_nothrow_default_constructible<>, is_nothrow_copy_constructible<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/type_traits.hpp> // for is_instantiation_of<>

#include <makeshift/detail/export.hpp> // for MAKESHIFT_PUBLIC


namespace makeshift
{

namespace detail
{


static constexpr std::uint64_t unit_invalid = std::uint64_t(-1);

static constexpr std::uint64_t min_char_base = 2*26 + 4; // '/', '-', '1'
static constexpr std::size_t min_char_max = 11;
constexpr inline int to_min_char(char ch) noexcept
{
    if (ch >= 'A' && ch <= 'Z') return (ch - 'A') + 1;
    if (ch >= 'a' && ch <= 'z') return (ch - 'a') + (26+1);
    if (ch == '/') return 2*26 + 1;
    if (ch == '-') return 2*26 + 2;
    if (ch == '1') return 2*26 + 3;
    return -1;
}
constexpr inline char from_min_char(int mc) noexcept
{
    if (mc == 0) return '\0';
    if (mc >= 1 && mc <= 26) return char('A' + (mc - 1));
    if (mc >= 26+1 && mc <= 2*26) return char('a' + (mc - (26+1)));
    if (mc == 2*26 + 1) return '/';
    if (mc == 2*26 + 2) return '-';
    if (mc == 2*26 + 3) return '1';
    return ' '; // should not happen
}

constexpr inline std::uint64_t parse_quantity_unit(const char* data, std::size_t size) noexcept
{
    std::uint64_t result = 0;
    std::uint64_t base = 1;
    if (size > min_char_max)
        return unit_invalid;
    for (std::size_t i = 0; i < size; ++i)
    {
        int mc = to_min_char(data[i]);
        if (mc < 0)
            return unit_invalid;
        result += std::uint64_t(mc) * base;
        base *= min_char_base;
    }
    return result;
}
constexpr inline void quantity_unit_to_string(std::uint64_t unitValue, char (&string)[min_char_max+1]) noexcept
{
    std::uint64_t value = std::uint64_t(unitValue);
    for (std::size_t i = 0; i < min_char_max; ++i)
    {
        string[i] = from_min_char(int(value % min_char_base));
        value /= min_char_base;
    }
    string[min_char_max] = '\0';
}

[[noreturn]] MAKESHIFT_PUBLIC void raise_quantity_conversion_error(std::uint64_t from, std::uint64_t to);


} // namespace detail


inline namespace types
{


enum class quantity_unit : std::uint64_t { };


struct default_unit_converter
{
    constexpr static bool is_convertible(quantity_unit from, quantity_unit to) noexcept { return from == to; }

    template <typename T>
        constexpr static T convert(T value, quantity_unit from, quantity_unit to) noexcept
    {
        Expects(is_convertible(from, to));
        return value;
    }
};


template <typename T, typename ConverterT>
    class dynamic_quantity;


template <typename T, quantity_unit Unit, typename ConverterT = default_unit_converter>
    class quantity
{
private:
    T value_;

    template <quantity_unit SrcUnit, typename SrcConverterT, typename SrcT>
        static constexpr SrcT do_convert_from(SrcT arg)
    {
        if constexpr (SrcConverterT::is_convertible(SrcUnit, Unit))
            return SrcConverterT::convert(arg, SrcUnit, Unit);
        else
            return ConverterT::convert(arg, SrcUnit, Unit);
    }
    template <typename SrcConverterT, typename SrcT>
        static constexpr SrcT do_convert_from(SrcT arg, quantity_unit srcUnit)
    {
        if (SrcConverterT::is_convertible(srcUnit, Unit))
            return SrcConverterT::convert(arg, srcUnit, Unit);
        else if (ConverterT::is_convertible(srcUnit, Unit))
            return ConverterT::convert(arg, srcUnit, Unit);
        else
            makeshift::detail::raise_quantity_conversion_error(std::uint64_t(srcUnit), std::uint64_t(Unit));
    }

public:
    constexpr quantity(void) noexcept(std::is_nothrow_default_constructible<T>::value) : value_{ } { }
    explicit constexpr quantity(T _value) noexcept(std::is_nothrow_copy_constructible<T>::value) : value_(_value) { }
    template <typename SrcT, quantity_unit SrcUnit, typename SrcConverterT,
              typename = std::enable_if_t<std::is_convertible<SrcT, T>::value && (SrcConverterT::is_convertible(SrcUnit, Unit) || ConverterT::is_convertible(SrcUnit, Unit))>>
        constexpr quantity(quantity<SrcT, SrcUnit, SrcConverterT> arg)
        : value_(do_convert_from<SrcUnit, SrcConverterT>(arg.value()))
    {
    }
    template <typename SrcT, typename SrcConverterT,
              typename = std::enable_if_t<std::is_convertible<SrcT, T>::value>>
        explicit constexpr quantity(dynamic_quantity<SrcT, SrcConverterT> arg)
            : value_(do_convert_from<SrcConverterT>(arg.value(), arg.unit()))
    {
    }

    constexpr T value(void) const noexcept(std::is_nothrow_copy_constructible<T>::value) { return value_; }
    constexpr static quantity_unit unit(void) noexcept { return Unit; }
};


template <typename T, typename ConverterT = default_unit_converter>
    class dynamic_quantity
{
private:
    T value_;
    quantity_unit unit_;

public:
    constexpr dynamic_quantity(void) noexcept(std::is_nothrow_default_constructible<T>::value) : value_{ }, unit_{ } { }
    explicit constexpr dynamic_quantity(T _value, quantity_unit _unit) noexcept(std::is_nothrow_copy_constructible<T>::value) : value_(_value), unit_(_unit) { }
    template <typename SrcT, typename SrcConverterT,
              typename = std::enable_if_t<std::is_convertible<SrcT, T>::value>>
        explicit constexpr dynamic_quantity(dynamic_quantity<SrcT, SrcConverterT> q)
            : value_(q.value()), unit_(q.unit())
    {
    }
    template <typename SrcT, quantity_unit SrcUnit, typename SrcConverterT,
              typename = std::enable_if_t<std::is_convertible<SrcT, T>::value>>
        explicit constexpr dynamic_quantity(quantity<SrcT, SrcUnit, SrcConverterT> q)
            : value_(q.value()), unit_(SrcUnit)
    {
    }

    constexpr T value(void) const noexcept(std::is_nothrow_copy_constructible<T>::value) { return value_; }
    constexpr quantity_unit unit(void) const noexcept { return unit_; }
};
template <typename T, quantity_unit Unit, typename ConverterT = default_unit_converter>
    dynamic_quantity(quantity<T, Unit, ConverterT>) -> dynamic_quantity<T, ConverterT>;
template <typename T>
    dynamic_quantity(T&&, quantity_unit) -> dynamic_quantity<std::decay_t<T>, default_unit_converter>;


template <quantity_unit Unit, typename T,
          typename = std::enable_if_t<!is_instantiation_of_v<std::decay_t<T>, dynamic_quantity>>>
    constexpr quantity<std::decay_t<T>, Unit> with_unit(T&& value) noexcept(std::is_nothrow_copy_constructible<std::decay_t<T>>::value)
{
    return quantity<std::decay_t<T>, Unit>{ std::forward<T>(value) };
}
template <quantity_unit Unit, typename T, typename ConverterT>
    constexpr quantity<std::decay_t<T>, Unit, ConverterT> with_unit(dynamic_quantity<T, ConverterT> value)
{
    return quantity<std::decay_t<T>, Unit>{ std::move(value) };
}


} // inline namespace types


inline namespace literals
{


constexpr inline quantity_unit operator ""_unit(const char* data, std::size_t size) noexcept
{
    return quantity_unit(makeshift::detail::parse_quantity_unit(data, size));
}


} // inline namespace literals

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_STRING_HPP_
 #include <makeshift/detail/quantity-string.hpp>
#endif // INCLUDED_MAKESHIFT_STRING_HPP_


#endif // INCLUDED_MAKESHIFT_QUANTITY_HPP_
