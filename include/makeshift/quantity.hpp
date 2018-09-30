
#ifndef INCLUDED_MAKESHIFT_QUANTITY_HPP_
#define INCLUDED_MAKESHIFT_QUANTITY_HPP_


#include <cstdint>     // for uint64_t
#include <type_traits> // for enable_if<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/detail/cfg.hpp> // for MAKESHIFT_DLLFUNC


namespace makeshift
{

namespace detail
{


static constexpr std::uint64_t unit_invalid = std::uint64_t(-1);

static constexpr std::uint64_t min_char_base = 2*26 + 4; // '/', '-', '1'
static constexpr std::size_t min_char_max = 11;
constexpr inline int to_min_char(char ch) noexcept
{
    if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 1;
    if (ch >= 'a' && ch <= 'z') return (ch - 'a') + 26 + 1;
    if (ch == '/') return 2*26 + 1;
    if (ch == '-') return 2*26 + 3;
    if (ch == '1') return 2*26 + 3;
    return -1;
}
constexpr inline char from_min_char(int mc) noexcept
{
    if (mc == 0) return '\0';
    if (mc >= 1 && mc <= 26) return char('A' + mc);
    if (mc >= 26+1 && mc <= 2*26) return char('a' + mc);
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
        string[i] = from_min_char(int(value % min_char_base));
    string[min_char_max] = '\0';
}

MAKESHIFT_DLLFUNC [[noreturn]] void raise_quantity_conversion_error(std::uint64_t from, std::uint64_t to);


} // namespace detail


inline namespace types
{


enum class quantity_unit : std::uint64_t { };


struct default_unit_converter
{
    constexpr bool is_convertible(quantity_unit from, quantity_unit to) noexcept { return from == to; }

    template <typename T>
        constexpr T operator ()(T value, quantity_unit from, quantity_unit to) noexcept
    {
        Expects(is_convertible(from, to));
        return value;
    }
};


template <typename T, typename ConverterT>
    struct dynamic_quantity;


template <typename T, quantity_unit Unit, typename ConverterT = default_unit_converter>
    struct quantity
{
    static constexpr quantity_unit unit = Unit;

    T value;

    template <typename DstT, quantity_unit DstUnit,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value && ConverterT::is_convertible(Unit, DstUnit)>>
        operator quantity<DstT, DstUnit, ConverterT>(void) const noexcept
    {
        return { value };
    }
    template <typename DstT, typename DstConverterT,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value>>
        operator dynamic_quantity<DstT, ConverterT>(void) const noexcept
    {
        return { value, Unit };
    }

    constexpr operator T(void) const noexcept { return value; }
};


template <typename T, typename ConverterT = default_unit_converter>
    struct dynamic_quantity
{
    T value;
    quantity_unit unit;

    template <typename DstT, quantity_unit DstUnit,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value>>
        operator quantity<DstT, DstUnit, ConverterT>(void) const
    {
        if (!ConverterT::is_convertible(unit, DstUnit))
            makeshift::detail::raise_quantity_conversion_error(unit, DstUnit);
        return { value };
    }
    template <typename DstT, typename DstConverterT,
              typename = std::enable_if_t<std::is_convertible<T, DstT>::value>>
        operator dynamic_quantity<DstT, ConverterT>(void) const noexcept
    {
        return { value, unit };
    }
    
    constexpr operator T(void) const noexcept { return value; }
};
template <typename T, quantity_unit Unit, typename ConverterT = default_unit_converter>
    dynamic_quantity(quantity<T, Unit, ConverterT>) -> dynamic_quantity<T, ConverterT>;


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
