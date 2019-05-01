
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
    using parameter_categories = type_sequence_cat_t<typename ParamsT::parameter_categories...>;

    constexpr parameter_set(ParamsT... params)
        : ParamsT(std::move(params))...
    {
    }
};


template <typename ParamT, typename... TagsT>
    struct define_parameter : TagsT...
{
    using parameter_categories = type_sequence<TagsT...>;
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

    MAKESHIFT_NODISCARD constexpr friend const std::array<ParamT, N>& select_parameters(const parameter_array& self, typename apply_<any_tag_of, typename ParamT::parameter_categories>::type) noexcept { return self.parameters_; }
};


template <typename T, typename... ParamsT>
    struct value_t : private parameter_set<ParamsT...>
{
private:
    T value_;

public:
    constexpr value_t(T _value, ParamsT... params)
        : parameter_set<ParamsT...>(std::move(params)...), value_(std::move(_value))
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
    struct MAKESHIFT_EMPTY_BASES values_parameter<type_sequence<TagsT...>, T, N, ParamsT...>
        : define_parameter<values_parameter<type_sequence<TagsT...>, T, N, ParamsT...>, TagsT...>,
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
    using value_type = T; // TODO: required?
    static constexpr std::size_t value_count = N; // TODO: required?

    constexpr values_parameter(const std::array<value_t<T, ParamsT...>, N>& _values)
        : parameter_array<ParamsT, N>(extract_params<ParamsT>(std::make_index_sequence<N>{ }, _values))...,
          values_(extract_values(std::make_index_sequence<N>{ }, _values))
    {
    }

    MAKESHIFT_NODISCARD constexpr const std::array<T, N>& values(void) const noexcept { return values_; }
};

struct heterogeneous_values_tag { };

template <typename TagsT, typename Ts, typename... ParamsT>
    struct heterogeneous_values_parameter;
template <typename... TagsT, template <typename...> class TupleT, typename... Ts, typename... ParamsT>
    struct MAKESHIFT_EMPTY_BASES heterogeneous_values_parameter<type_sequence<TagsT...>, TupleT<Ts...>, ParamsT...>
        : define_parameter<heterogeneous_values_parameter<type_sequence<TagsT...>, TupleT<Ts...>, ParamsT...>, TagsT..., heterogeneous_values_tag>,
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

    TupleT<Ts...> values_;

public:
    using value_types = type_sequence<Ts...>; // TODO: required?
    static constexpr std::size_t value_count = sizeof...(Ts); // TODO: required?

    constexpr heterogeneous_values_parameter(std::tuple<value_t<Ts, ParamsT...>...> _values)
        : parameter_array<ParamsT, sizeof...(Ts)>(extract_params<ParamsT>(std::make_index_sequence<sizeof...(Ts)>{ }, _values))...,
          values_(extract_values(std::make_index_sequence<sizeof...(Ts)>{ }, _values))
    {
    }
    template <typename = std::enable_if_t<sizeof...(ParamsT) == 0>>
        constexpr heterogeneous_values_parameter(TupleT<Ts...> _values) noexcept
            : values_(_values)
    {
    }

    MAKESHIFT_NODISCARD constexpr const TupleT<Ts...>& values(void) const noexcept { return values_; }
};


struct values_tag { };

template <typename T, std::size_t N, typename... ParamsT>
    using values_t = values_parameter<type_sequence<values_tag>, T, N, ParamsT...>;

template <typename Ts, typename... ParamsT>
    using heterogeneous_values_t = heterogeneous_values_parameter<type_sequence<values_tag>, Ts, ParamsT...>;


template <typename T>
    constexpr value_t<T> wrap_value(T value)
{
    return { std::move(value) };
}
template <typename T, typename... ParamsT>
    constexpr value_t<T, ParamsT...> wrap_value(value_t<T, ParamsT...> value)
{
    return std::move(value);
}

template <typename TagsT, typename... Ts, typename... ParamsT>
    constexpr inline values_parameter<TagsT, typename equal_types_<Ts...>::common_type, sizeof...(Ts), ParamsT...> values_impl_2(std::true_type /*equalTypes*/, value_t<Ts, ParamsT...>&&... values)
{
    return { { std::move(values)... } };
}
template <typename TagsT, typename... Ts, typename... ParamsT>
    constexpr inline heterogeneous_values_parameter<TagsT, std::tuple<Ts...>, ParamsT...> values_impl_2(std::false_type /*equalTypes*/, value_t<Ts, ParamsT...>&&... values)
{
    return { { std::move(values)... } };
}

/*

Thoughts:
- values() and known_values() should support products
- should values() support only exhaustive products? probably...
- we need to be able to generate member accessors from value lists so we can compare objects; perhaps [het'_]values_parameter should already expose a comparer/hasher which is then used by expand()?
- this vastly simplifies expand() because product-to-values flattening then happens here
- perhaps get rid of compound_hash usage, as we don't currently need it?

*/

template <typename TagsT, typename... Ts, typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr auto values_impl_1(value_t<Ts, ParamsT...>&&... values)
{
    return makeshift::detail::values_impl_2<TagsT>(equal_types_<Ts...>{ }, std::move(values)...);
}
template <typename TagsT, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto values_impl_0(Ts&&... values)
{
    return makeshift::detail::values_impl_1<TagsT>(makeshift::detail::wrap_value(std::forward<Ts>(values))...);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
