
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_


#include <array>
#include <cstddef> // for size_t
#include <utility> // for move(), integer_sequence<>

#include <makeshift/type_traits2.hpp> // for type_sequence<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD, MAKESHIFT_CXX17, MAKESHIFT_EMPTY_BASES

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

    // These are not the most thought-out abstractions ever, but at least they save us from having to repeat ourselves.
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
template <template <std::size_t> class ParamT, typename T>
    struct array_parameter
{
    MAKESHIFT_NODISCARD constexpr ParamT<0> operator =(no_values_tag) const
    {
        return { { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr ParamT<N> operator =(T (&&vals)[N]) const
    {
        return { makeshift::detail::to_array2(vals) };
    }
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr ParamT<N> operator =(const std::array<T, N>& vals) const
    {
        return { vals };
    }
};

template <template <typename, std::size_t> class ParamT, typename T>
    struct array_parameter_of
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
    using parameter_categories = type_sequence2_cat_t<typename ParamsT::parameter_categories...>;

    constexpr metadata_t(const ParamsT&... params)
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
    using parameter_categories = type_sequence2<name_tag>;
    MAKESHIFT_NODISCARD constexpr friend const name_t& select_parameter(const name_t& self, name_tag) noexcept { return self; }

    constexpr name_t(void) noexcept = default;

    constexpr name_t(const name_t&) noexcept = default;
    constexpr name_t& operator =(const name_t&) noexcept = default;

    constexpr name_t(const char* str) noexcept
        : name_(str)
    {
    }
    constexpr name_t(string_view str) noexcept
        : name_(str)
    {
    }

    MAKESHIFT_NODISCARD constexpr string_view name(void) const noexcept { return name_; }
};


struct values_tag { };

template <typename T, std::size_t N>
    struct values_t : values_tag
{
private:
    std::array<T, N> values_;

public:
    using parameter_categories = type_sequence2<values_tag>;
    MAKESHIFT_NODISCARD constexpr friend const values_t& select_parameter(const values_t& self, values_tag) noexcept { return self; }

    constexpr values_t(const std::array<T, N>& _values)
        : values_(_values)
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::array<T, N>& values(void) const noexcept { return values_; }
};


struct value_names_tag { };

template <std::size_t N>
    struct value_names_t : value_names_tag
{
private:
    std::array<string_view, N> valueNames_;

public:
    using parameter_categories = type_sequence2<value_names_tag>;
    MAKESHIFT_NODISCARD constexpr friend const value_names_t& select_parameter(const value_names_t& self, value_names_tag) noexcept { return self; }

    constexpr value_names_t(const std::array<string_view, N>& _valueNames)
        : valueNames_(_valueNames)
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::array<string_view, N>& value_names(void) const noexcept { return valueNames_; }
};


template <typename T>
    struct named_t
{
    T value;
    string_view name;
};

template <typename T, std::size_t N>
    struct named_values_t;
template <typename T, std::size_t N>
    struct MAKESHIFT_EMPTY_BASES named_values_t<named_t<T>, N> : values_t<T, N>, value_names_t<N>
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

public:
    using parameter_categories = type_sequence2<values_tag, value_names_tag>;

    constexpr named_values_t(const std::array<named_t<T>, N>& _namedValues)
        : values_t<T, N>(extractValues(_namedValues)),
          value_names_t<N>(extractNames(_namedValues))
    {
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
