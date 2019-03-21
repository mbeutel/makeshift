
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_


#include <array>
#include <cstddef> // for size_t
#include <utility> // for move(), integer_sequence<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD, MAKESHIFT_EMPTY_BASES

#include <makeshift/detail/string_view.hpp>

#ifdef MAKESHIFT_CXX17
 #include <string_view>
#endif // MAKESHIFT_CXX17


namespace makeshift
{

namespace detail
{


template <std::size_t... Is, typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2_impl(std::index_sequence<Is...>, const T (&array)[N])
{
    return { array[Is]... };
}
template <typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2(const T (&array)[N])
{
    return to_array2_impl(std::make_index_sequence<N>{ }, array);
}


struct no_values_tag { };

    // These are not the best abstractions ever but at least they save us from having to repeat ourselves.
template <typename ParamT, typename T = ParamT>
    struct parameter
{
    MAKESHIFT_NODISCARD constexpr ParamT operator =(T arg) const
    {
        return { arg };
    }
};
template <typename ParamT>
    struct parameter<ParamT, ParamT>
{
    MAKESHIFT_NODISCARD constexpr ParamT operator =(ParamT arg) const
    {
        return std::move(arg);
    }
};
template <template <typename, std::size_t> class ParamT, typename T>
    struct array_parameter
{
    MAKESHIFT_NODISCARD constexpr ParamT<T, 0> operator =(no_values_tag) const
    {
        return { { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr ParamT<T, N> operator =(T (&&vals)[N]) const
    {
        return { makeshift::detail::to_array2(vals) };
    }
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr ParamT<T, N> operator =(const std::array<T, N>& vals) const
    {
        return { vals };
    }
};


template <typename... ParamsT>
    struct MAKESHIFT_EMPTY_BASES metadata_t : ParamsT...
{
    constexpr metadata_t(ParamsT... params)
        : ParamsT(params)...
    {
    }
};


struct name_tag { };

struct name_t : name_tag
{
private:
    string_view name_;

public:
    constexpr name_t(void) noexcept = default;

    constexpr name_t(const name_t&) noexcept = default;
    constexpr name_t& operator =(const name_t&) noexcept = default;

    template <std::size_t N>
        constexpr name_t(const char (&str)[N]) noexcept // permit construction from literals only
        : name_(str)
    {
    }

    MAKESHIFT_NODISCARD constexpr string_view name(void) const noexcept { return name_; }
};

template <typename T>
    struct named_t
{
    T value;
    name_t name;
};

struct values_tag { };

template <typename T, std::size_t N>
    struct values_t : values_tag
{
private:
    std::array<T, N> values_;

public:
    constexpr values_t(const std::array<T, N>& _values)
        : values_(_values)
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::array<T, N>& values(void) const noexcept { return values_; }
};

struct value_names_tag { };

template <typename T, std::size_t N>
    struct named_values_t;
template <typename T, std::size_t N>
    struct MAKESHIFT_EMPTY_BASES named_values_t<named_t<T>, N> : values_t<T, N>, value_names_tag
{
private:
    static constexpr std::array<string_view, N> extractNames(const std::array<named_t<T>, N>& _namedValues)
    {
        auto result = std::array<string_view, N>{ };
        for (std::size_t i = 0; i < N; ++i)
            result[i] = _namedValues[i].name;
        return result;
    }
    static constexpr std::array<T, N> extractValues(const std::array<named_t<T>, N>& _namedValues)
    {
        auto result = std::array<T, N>{ };
        for (std::size_t i = 0; i < N; ++i)
            result[i] = _namedValues[i].value;
        return result;
    }

    std::array<string_view, N> valueNames_;

public:
    constexpr named_values_t(const std::array<named_t<T>, N>& _namedValues)
        : values_t<T, N>(extractValues(_namedValues)),
          valueNames_(extractNames(_namedValues))
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::array<string_view, N>& value_names(void) const noexcept { return valueNames_; }
};

    // TODO: remove
template <typename T, std::size_t N>
    MAKESHIFT_NODISCARD constexpr std::array<T, N> to_array(const values_t<T, N>& values)
{
    return values.values;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
