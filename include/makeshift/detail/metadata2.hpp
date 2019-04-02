
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_


#include <array>
#include <cstddef> // for size_t
#include <utility> // for move(), integer_sequence<>

#include <makeshift/type_traits2.hpp> // for type_sequence<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD, MAKESHIFT_CXX17, MAKESHIFT_EMPTY_BASES

#include <makeshift/detail/string_view.hpp>
#include <makeshift/detail/workaround.hpp>  // for cand()

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

    constexpr parameter_set(ParamsT... params)
        : ParamsT(std::move(params))...
    {
    }
};


template <typename ParamT, typename... TagsT>
    struct define_parameter : TagsT...
{
    using parameter_categories = type_sequence2<TagsT...>;
    MAKESHIFT_NODISCARD constexpr friend const ParamT& select_parameter(const ParamT& param, any_tag_of<TagsT...>) noexcept { return param; }
};


template <typename ParamT, std::size_t N>
    struct parameter_array
{
private:
    std::array<ParamT, N> parameters_;

public:
    constexpr parameter_array(std::array<ParamT, N> _parameters)
        : parameters_(std::move(_parameters))
    {
    }

    MAKESHIFT_NODISCARD constexpr friend const std::array<ParamT, N>& select_parameters(const parameter_array& self, typename apply_<any_tag_of, typename ParamT::parameter_categories>::type) noexcept { return parameters_; }
};


template <typename T, typename... ParamsT>
    struct value_t : private parameter_set<ParamsT...>
{
private:
    T value_;

public:
    constexpr value_t(T _value, ParamsT... params)
        : parameter_set<ParamsT...>(std::move(params))..., value_(std::move(_value))
    {
    }

    MAKESHIFT_NODISCARD constexpr const T& value(void) const noexcept { return value_; }
    MAKESHIFT_NODISCARD constexpr const parameter_set<ParamsT...>& parameters(void) const noexcept { return *this; }
};


struct name_tag { };

struct name_t : define_parameter<name_t, name_tag>
{
private:
    string_view name_;

public:
    constexpr name_t(string_view str) noexcept
        : name_(str)
    {
    }

    MAKESHIFT_NODISCARD constexpr string_view name(void) const noexcept { return name_; }
};


template <typename TagsT, typename T, std::size_t N, typename... ParamsT>
    struct values_parameter;
template <typename... TagsT, typename T, std::size_t N, typename... ParamsT>
    struct MAKESHIFT_EMPTY_BASES values_parameter<type_sequence2<TagsT...>, T, N, ParamsT...>
        : define_parameter<TagsT...>,
          parameter_array<ParamsT, N>...
{
private:
    template <std::size_t... Is>
        static constexpr std::array<T, N> extract_values(std::index_sequence<Is...>, const std::array<value_t<T, ParamsT...>, N>& _values)
    {
        return { _values[Is].value()... };
    }
    template <typename ParamT, std::size_t... Is>
        static constexpr std::array<ParamT, N> extract_params(std::index_sequence<Is...>, const std::array<value_t<T, ParamsT...>, N>& _values)
    {
        return { static_cast<const ParamT&>(_values[Is].parameters())... };
    }

    std::array<T, N> values_;

public:
    constexpr values_parameter(const std::array<value_t<T, ParamsT...>, N>& _values)
        : parameter_array<ParamsT, N>(extract_params<ParamsT>(std::make_index_sequence<N>{ }, _values)),
          values_(extract_values(std::make_index_sequence<N>{ }, _values))
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::array<T, N>& values(void) const noexcept { return values_; }
};

struct values_tag { };

struct homogeneous_values_tag { };

template <typename T, std::size_t N, typename... ParamsT>
    using values_t = values_parameter<type_sequence2<values_tag, homogeneous_values_tag>, T, N, ParamsT...>;


template <typename ValuesParamT, typename TagsT, typename Ts, typename... ParamsT>
    struct define_heterogeneous_values_parameter;
template <typename ValuesParamT, typename... TagsT, typename... Ts, typename... ParamsT>
    struct MAKESHIFT_EMPTY_BASES define_heterogeneous_values_parameter<ValuesParamT, type_sequence2<TagsT...>, std::tuple<Ts...>, ParamsT...>
        : define_parameter<ValuesParamT, TagsT...>,
          parameter_array<ParamsT, sizeof...(Ts)>...
{
private:
    template <std::size_t... Is>
        static constexpr std::tuple<Ts...> extract_values(std::index_sequence<Is...>, const std::tuple<value_t<Ts, ParamsT...>...>& _values)
    {
        return { std::get<Is>(_values).value()... };
    }
    template <typename ParamT, std::size_t... Is>
        static constexpr std::array<ParamT, sizeof...(Ts)> extract_params(std::index_sequence<Is...>, const std::tuple<value_t<Ts, ParamsT...>...>& _values)
    {
        return { static_cast<const ParamT&>(std::get<Is>(_values).parameters())... };
    }

    std::tuple<Ts...> values_;

public:
    constexpr define_heterogeneous_values_parameter(std::tuple<value_t<Ts, ParamsT...>...> _values)
        : parameter_array<ParamsT, sizeof...(Ts)>(extract_params<ParamsT>(std::make_index_sequence<sizeof...(Ts)>{ }, _values)),
          values_(extract_values(std::make_index_sequence<sizeof...(Ts)>{ }, _values))
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::tuple<Ts...>& values(void) const noexcept { return values_; }
};

template <typename T, std::size_t N, typename... ParamsT>
    struct values_t : define_values_parameter<values_t<T, N, ParamsT...>, type_sequence2<values_tag, homogeneous_values_tag>, T, N, ParamsT...>
{
    using base = define_values_parameter<values_t<T, N, ParamsT...>, type_sequence2<values_tag, homogeneous_values_tag>, T, N, ParamsT...>;
    using base::base;
};


template <typename... Ts>
    constexpr inline values_t<typename equal_types_<Ts...>::common_type, sizeof...(Ts)> values_raw_impl(std::true_type /*equalTypes*/, Ts... values)
{
    return { /*std::array<typename equal_types_<Ts...>::common_type, sizeof...(Ts)>*/{ value_t<Ts>{ std::move(values) }... } };
}
template <typename... Ts>
    constexpr inline heterogeneous_values_t<std::tuple<Ts...>> values_raw_impl(std::false_type /*equalTypes*/, Ts... values)
{
    return { /*std::tuple<Ts...>*/{ value_t<Ts>(std::move(values))... } };
}
template <typename... Ts, typename... ParamsT>
    constexpr inline values_t<typename equal_types_<Ts...>::common_type, sizeof...(Ts), ParamsT...> values_impl(std::true_type /*equalTypes*/, value_t<Ts, ParamsT...>... values)
{
    return { /*std::array<typename equal_types_<Ts...>::common_type, sizeof...(Ts)>*/{ std::move(values)... } };
}
template <typename... Ts, typename... ParamsT>
    constexpr inline heterogeneous_values_t<std::tuple<Ts...>> values_impl(std::false_type /*equalTypes*/, value_t<Ts, ParamsT...>... values)
{
    return { /*std::tuple<Ts...>*/{ std::move(values)... } };
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
