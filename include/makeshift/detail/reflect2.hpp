
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <cstdint>     // for [u]int[8|16|32|64]_t
#include <type_traits> // for is_base_of<>, integral_constant<>, conjunction<>, disjunction<>

#include <makeshift/type_traits2.hpp> // for can_apply<>, type<>
#include <makeshift/metadata2.hpp>    // for named_t<>
#include <makeshift/tuple2.hpp>       // for array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_CXX17, MAKESHIFT_NODISCARD, MAKESHIFT_EMPTY_BASES

#ifdef MAKESHIFT_CXX17
 #include <variant>
#endif // MAKESHIFT_CXX17

namespace makeshift
{

namespace detail
{


template <typename T> using raw_metadata_of_r = decltype(reflect(type<T>{ }));
template <typename T> struct have_raw_metadata : can_apply<makeshift::detail::raw_metadata_of_r, T> { };


template <typename T>
    struct metadata_of_;

template <typename T, typename = void>
    struct default_metadata
{
};
struct no_name_base
{
    template <typename T>
        constexpr no_name_base(const T&)
    {
    }
};
template <typename T>
    using name_base = typename std::conditional<std::is_base_of<name_tag, typename metadata_of_<T>::type>::value, name_t, no_name_base>::type;
template <typename T>
    struct MAKESHIFT_EMPTY_BASES default_metadata<type_t<T>>
        : name_base<T>, values_tag
{
    constexpr default_metadata(void)
        : name_base<T>(metadata_of_<T>{ }())
    {
    }

    MAKESHIFT_NODISCARD static constexpr std::array<type_t<T>, 1> values(void) noexcept { return { { } }; }
};
#ifdef MAKESHIFT_CXX17
template <typename... Ts>
    struct default_metadata<std::variant<Ts...>, std::enable_if_t<std::conjunction_v<std::is_base_of<values_tag, typename metadata_of_<Ts>::type>...>>>
        : values_tag
{
    MAKESHIFT_NODISCARD static constexpr auto values(void) noexcept
    {
        return array_cat(); // TODO: array_cat() should not need to default-construct anything
    }
};
#endif // MAKESHIFT_CXX17
template <>
    struct default_metadata<float> : name_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "float"; }
};
template <>
    struct default_metadata<double> : name_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "double"; }
};
template <>
    struct default_metadata<std::int8_t> : name_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "int8"; }
};
template <>
    struct default_metadata<std::int16_t> : name_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "int16"; }
};
template <>
    struct default_metadata<std::int32_t> : name_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "int32"; }
};
template <>
    struct default_metadata<std::int64_t> : name_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "int64"; }
};
template <>
    struct default_metadata<bool> : name_tag, values_tag, value_names_tag
{
    MAKESHIFT_NODISCARD static constexpr string_view name(void) noexcept { return "bool"; }
    MAKESHIFT_NODISCARD static constexpr std::array<bool, 2> values(void) noexcept { return { false, true }; }
    MAKESHIFT_NODISCARD static constexpr std::array<string_view, 2> value_names(void) noexcept { return { "false", "true" }; }
};

template <typename T, bool RawMetadata>
    struct metadata_of_0_;
template <typename T>
    struct metadata_of_0_<T, true>
{
    using type = raw_metadata_of_r<T>;
    constexpr type operator ()(void) { return reflect(type<T>{ }); }
};
template <typename T>
    struct metadata_of_0_<T, false>
{
    constexpr default_metadata<T> operator ()(void) { return { }; }
};
template <typename T>
    struct metadata_of_ : metadata_of_0_<T, have_raw_metadata<T>::value>
{
};


    // metadata of type<> -> maybe name
    // metadata of variant<> -> values, if available
    // metadata of type_enum<> -> values


struct strip_names_functor
{
    template <typename T>
        constexpr T operator ()(const named_t<T> v) const
    {
        return v.value;
    }
};
struct add_names_functor
{
    template <typename T>
        constexpr named_t<T> operator ()(const T& v) const
    {
        return { v, { } };
    }
};

template <typename T, typename = void>
    struct values_inferrer;
template <typename T> struct can_infer_values_ : can_apply<values_inferrer, T> { };

template <typename T> struct have_value_metadata_ : std::disjunction<can_infer_values_<T>, std::conjunction<have_raw_metadata<T>, is_value_metadata<T>>> { };

template <>
    struct values_inferrer<bool>
{
    constexpr std::array<bool, 2> operator ()(void) const noexcept
    {
        return { false, true };
    }
};
template <typename T>
    struct values_inferrer<T, std::enable_if_t<std::conjunction_v<std::is_empty<T>, std::is_trivially_default_constructible<T>>>>
{
    constexpr std::array<T, 1> operator ()(void) const noexcept
    {
        return { { } };
    }
};
template <typename... Ts> // TODO: optional and tuple?
    struct values_inferrer<std::variant<Ts...>, std::enable_if_t<std::conjunction_v<have_value_metadata_<Ts>...>>>
{
private:
    using V = std::variant<Ts...>;
    template <std::size_t... Is>
        static constexpr std::array<V, sizeof...(Ts)> invoke(std::index_sequence<Is...>) noexcept
    {
        return { V{ std::in_place_index<Is> }... };
    }
public:
    constexpr std::array<V, sizeof...(Ts)> operator ()(void) const noexcept
    {
        return invoke(std::index_sequence_for<Ts...>{ });
    }
};

template <typename ValuesT>
    struct values_reflector1;
template <typename T, std::size_t N>
    struct values_reflector1<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return reflect(type<T>{ }).values;
    }
};
template <typename T, std::size_t N>
    struct values_reflector1<values_t<named_t<T>, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(strip_names_functor{ }, reflect(type<T>{ }).values);
    }
};

template <typename T, bool HaveRawMetadata>
    struct values_reflector0;
template <typename T>
    struct values_reflector0<T, false> : values_inferrer<T>
{
};
template <typename T>
    struct values_reflector0<T, true> : values_reflector1<raw_metadata_of_r<T>>
{
};

template <typename T, typename = void>
    struct values_reflector : values_reflector0<T, have_raw_metadata<T>::value>
{
};

template <typename T>
    struct named_values_inferrer
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(add_names_functor{ }, values_inferrer<T>{ }());
    }
};
template <>
    struct named_values_inferrer<bool>
{
    constexpr std::array<named_t<bool>, 2> operator ()(void) const noexcept
    {
        return { named_t<bool>{ false, "false" }, named_t<bool>{ true, "true" } };
    }
};

template <typename ValuesT>
    struct named_values_reflector1;
template <typename T, std::size_t N>
    struct named_values_reflector1<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(add_names_functor{ }, reflect(type<T>{ }).values);
    }
};
template <typename T, std::size_t N>
    struct named_values_reflector1<values_t<named_t<T>, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return reflect(type<T>{ }).values;
    }
};

template <typename T, bool HaveRawMetadata>
    struct named_values_reflector0;
template <typename T>
    struct named_values_reflector0<T, false> : named_values_inferrer<T>
{
};
template <typename T>
    struct named_values_reflector0<T, true> : named_values_reflector1<raw_metadata_of_r<T>>
{
};

template <typename T, typename = void>
    struct named_values_reflector : named_values_reflector0<T, have_raw_metadata<T>::value>
{
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
