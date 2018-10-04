
#ifndef INCLUDED_MAKESHIFT_QUANTITY_HPP_
#define INCLUDED_MAKESHIFT_QUANTITY_HPP_


#include <cstdint>     // for uint64_t
#include <type_traits> // for enable_if<>, is_nothrow_default_constructible<>, is_nothrow_copy_constructible<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/detail/export.hpp> // for MAKESHIFT_DLLFUNC


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

MAKESHIFT_DLLFUNC [[noreturn]] void raise_quantity_conversion_error(std::uint64_t from, std::uint64_t to);


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

public:
    constexpr quantity(void) noexcept(std::is_nothrow_default_constructible<T>::value) : value_{ } { }
    explicit constexpr quantity(T _value) noexcept(std::is_nothrow_copy_constructible<T>::value) : value_(_value) { }

    constexpr T value(void) const noexcept(std::is_nothrow_copy_constructible<T>::value) { return value_; }
    constexpr static quantity_unit unit(void) noexcept { return Unit; }

    template <typename DstT, quantity_unit DstUnit,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value && ConverterT::is_convertible(Unit, DstUnit)>>
        constexpr operator quantity<DstT, DstUnit, ConverterT>(void) const noexcept(std::is_nothrow_copy_constructible<T>::value)
    {
        return { ConverterT::convert(value_, Unit, DstUnit) };
    }
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
    template <typename SrcT, quantity_unit SrcUnit,
              typename = std::enable_if_t<std::is_convertible<SrcT, T>::value>>
        explicit constexpr dynamic_quantity(quantity<SrcT, SrcUnit, ConverterT> q) noexcept(std::is_nothrow_copy_constructible<T>::value)
            : value_(q.value()), unit_(SrcUnit)
    {
    }

    constexpr T value(void) const noexcept(std::is_nothrow_copy_constructible<T>::value) { return value_; }
    constexpr quantity_unit unit(void) const noexcept { return unit_; }

    template <typename DstT, quantity_unit DstUnit,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value>>
        constexpr operator quantity<DstT, DstUnit, ConverterT>(void) const
    {
        if (!ConverterT::is_convertible(unit_, DstUnit))
            makeshift::detail::raise_quantity_conversion_error(std::uint64_t(unit_), std::uint64_t(DstUnit));
        return quantity<DstT, DstUnit, ConverterT>{ ConverterT::convert(value_, unit_, DstUnit) };
    }
    template <typename DstT, typename DstConverterT,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value>>
        operator dynamic_quantity<DstT, ConverterT>(void) const noexcept(std::is_nothrow_copy_constructible<T>::value)
    {
        return dynamic_quantity<DstT, ConverterT>{ value_, unit_ };
    }
};
template <typename T, quantity_unit Unit, typename ConverterT = default_unit_converter>
    dynamic_quantity(quantity<T, Unit, ConverterT>) -> dynamic_quantity<T, ConverterT>;
template <typename T>
    dynamic_quantity(T&&, quantity_unit) -> dynamic_quantity<std::decay_t<T>, default_unit_converter>;


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
