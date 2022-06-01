
#ifndef INCLUDED_MAKESHIFT_DETAIL_IOMANIP_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_IOMANIP_HPP_


#include <ios>
#include <iosfwd>
#include <string>
#include <string_view>
#include <type_traits>  // for remove_reference<>, remove_cv<>

#include <makeshift/detail/serialize.hpp>


namespace makeshift {

namespace detail {


template <typename T, typename ReflectorT>
struct enum_manipulator;
template <typename T, typename ReflectorT>
struct enum_manipulator<T&, ReflectorT>
{
    T& value_;

    friend std::istream&
    operator >>(std::istream& stream, enum_manipulator&& self)
    {
        std::string str;
        int ci;
        stream >> std::ws;
        while (stream.good() && (ci = stream.peek()) != std::char_traits<char>::eof() && !enum_forbidden_char_set.contains(static_cast<char>(ci)))
        {
            str += static_cast<char>(stream.get());
        }
        if (str.empty() || detail::try_enum_from_string(self.value_, str, static_enum_metadata<T, ReflectorT>::value) != 0)
        {
            stream.setstate(std::ios_base::failbit);
        }
        return stream;
    }
};
template <typename T, typename ReflectorT>
struct enum_manipulator<T const&, ReflectorT>
{
    T value_;
};
template <typename T, typename ReflectorT>
struct enum_manipulator
{
    T value_;
};
template <typename T, typename ReflectorT>
std::ostream& operator <<(std::ostream& stream, enum_manipulator<T, ReflectorT>&& self)
{
    return stream << detail::enum_to_string(self.value_, static_enum_metadata<std::remove_cv_t<std::remove_reference_t<T>>, ReflectorT>::value);
}


template <typename T, typename ReflectorT>
struct flags_manipulator;
template <typename T, typename ReflectorT>
struct flags_manipulator<T&, ReflectorT>
{
    T& value_;

    friend std::istream&
    operator >>(std::istream& stream, flags_manipulator&& self)
    {
        std::string str;
        int ci;
        stream >> std::ws;
        while (stream.good() && (ci = stream.peek()) != std::char_traits<char>::eof() && !flags_forbidden_char_set.contains(static_cast<char>(ci)))
        {
            str += static_cast<char>(stream.get());
        }
        if (detail::flags_from_string(self.value_, str, static_flags_metadata<T, ReflectorT>::value, false) != 0)
        {
            stream.setstate(std::ios_base::failbit);
        }
        return stream;
    }
};
template <typename T, typename ReflectorT>
struct flags_manipulator<T const&, ReflectorT>
{
    T value_;
};
template <typename T, typename ReflectorT>
struct flags_manipulator
{
    T value_;
};
template <typename T, typename ReflectorT>
std::ostream& operator <<(std::ostream& stream, flags_manipulator<T, ReflectorT>&& self)
{
    return stream << detail::flags_to_string(self.value_, static_flags_metadata<std::remove_cv_t<std::remove_reference_t<T>>, ReflectorT>::value);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_IOMANIP_HPP_
