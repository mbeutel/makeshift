
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


template <typename... ParamsT>
    struct MAKESHIFT_EMPTY_BASES parameter_set : ParamsT...
{
    using parameter_categories = type_sequence2_cat_t<typename ParamsT::parameter_categories...>;

    constexpr parameter_set(const ParamsT&... params)
        : ParamsT(params)...
    {
    }
};


template <typename ParamT, typename TagT>
    struct define_parameter : TagT
{
    using parameter_categories = type_sequence2<TagT>;
    MAKESHIFT_NODISCARD constexpr friend const ParamT& select_parameter(const ParamT& param, TagT) noexcept { return param; }
};


struct name_tag { };

struct name_t : define_parameter<name_t, name_tag>
{
private:
    string_view name_;

public:
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
    struct values_t : define_parameter<values_t<T, N>, values_tag>
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

template <std::size_t N>
    struct value_names_t : define_parameter<value_names_t<N>, value_names_tag>
{
private:
    std::array<string_view, N> valueNames_;

public:
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


struct no_values_tag { };

struct name_parameter_name
{
    MAKESHIFT_NODISCARD constexpr name_t operator =(std::string_view arg) const
    {
        return { arg };
    }
};

template <typename T>
    struct values_parameter_name
{
    MAKESHIFT_NODISCARD constexpr values_t<T, 0> operator =(no_values_tag) const
    {
        return { { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr values_t<T, N> operator =(T (&&vals)[N]) const
    {
        return { makeshift::detail::to_array2(vals) };
    }
};

struct value_names_parameter_name
{
    MAKESHIFT_NODISCARD constexpr value_names_t<0> operator =(no_values_tag) const
    {
        return { { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr value_names_t<N> operator =(std::string_view (&&vals)[N]) const
    {
        return { makeshift::detail::to_array2(vals) };
    }
};

template <typename T>
    struct named_values_parameter_name
{
    MAKESHIFT_NODISCARD constexpr parameter_set<values_t<T, 0>, value_names_t<0>> operator =(no_values_tag) const
    {
        return { { }, { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr parameter_set<values_t<T, N>, value_names_t<N>> operator =(T (&&namedValues)[N]) const
    {
        auto values = std::array<T, N>{ };
        auto names = std::array<T, N>{ };
        for (std::size_t i = 0; i < N; ++i)
        {
            values[i] = std::move(namedValues[i]).value;
            names[i] = namedValues[i].name;
        }
        return { std::move(values), names };
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
