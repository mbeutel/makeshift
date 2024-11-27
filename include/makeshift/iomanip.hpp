
#ifndef INCLUDED_MAKESHIFT_IOMANIP_HPP_
#define INCLUDED_MAKESHIFT_IOMANIP_HPP_


#include <istream>      // for ws
#include <type_traits>  // for remove_reference<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/metadata.hpp>

#include <makeshift/detail/iomanip.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


template <typename ToStreamFuncT>
[[nodiscard]] constexpr detail::manipulator<ToStreamFuncT>
make_manipulator(ToStreamFuncT toStreamFunc)
{
    return detail::manipulator<ToStreamFuncT>{ std::move(toStreamFunc) };
}
template <typename ToStreamFuncT, typename FromStreamFuncT>
[[nodiscard]] constexpr detail::manipulator<ToStreamFuncT, FromStreamFuncT>
make_manipulator(ToStreamFuncT toStreamFunc, FromStreamFuncT fromStreamFunc)
{
    return detail::manipulator<ToStreamFuncT, FromStreamFuncT>{ std::move(toStreamFunc), std::move(fromStreamFunc) };
}


template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
as_enum(T&& value, ReflectorT = { })
{
    return makeshift::make_manipulator(
        [](std::ostream& stream, T const& value)
        {
            stream << detail::enum_to_string(value, detail::static_enum_metadata<std::remove_cv_t<std::remove_reference_t<T>>, ReflectorT>::value);
        },
        [](std::istream& stream, auto& value)
        {
            std::string str;
            int ci;
            stream >> std::ws;
            while (stream.good() && (ci = stream.peek()) != std::char_traits<char>::eof() && !detail::enum_forbidden_char_set.contains(static_cast<char>(ci)))
            {
                str += static_cast<char>(stream.get());
            }
            if (str.empty() || detail::try_enum_from_string(value, str, detail::static_enum_metadata<std::remove_cv_t<std::remove_reference_t<T>>, ReflectorT>::value) != 0)
            {
                stream.setstate(std::ios_base::failbit);
            }
        })(std::forward<T>(value));
}


template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
as_flags(T&& value, ReflectorT = { })
{
    return makeshift::make_manipulator(
        [](std::ostream& stream, T const& value)
        {
            stream << detail::flags_to_string(value, detail::static_flags_metadata<std::remove_cv_t<std::remove_reference_t<T>>, ReflectorT>::value);
        },
        [](std::istream& stream, auto& value)
        {
            std::string str;
            int ci;
            stream >> std::ws;
            while (stream.good() && (ci = stream.peek()) != std::char_traits<char>::eof() && !detail::flags_forbidden_char_set.contains(static_cast<char>(ci)))
            {
                str += static_cast<char>(stream.get());
            }
            if (detail::flags_from_string(value, str, detail::static_flags_metadata<std::remove_cv_t<std::remove_reference_t<T>>, ReflectorT>::value, false) != 0)
            {
                stream.setstate(std::ios_base::failbit);
            }
        })(std::forward<T>(value));
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOMANIP_HPP_
