
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_ENUM_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_ENUM_HPP_


#include <makeshift/experimental/detail/enum.hpp>


namespace makeshift {


    //
    // Inherit from `define_type_enum<>` to define a named type enumeration.
    //ᅟ
    //ᅟ    struct FloatTypes : define_type_enum<FloatTypes, float, double> { using base::base; };
    //ᅟ
    //ᅟ    FloatTypes floatType = ...;
    //ᅟ    switch (floatType)
    //ᅟ    {
    //ᅟ    case type_c<float>:  ...; break;
    //ᅟ    case type_c<double>: ...; break;
    //ᅟ    }
    //
template <typename TypeEnumT, typename... Ts>
struct define_type_enum : detail::define_type_enum_base<TypeEnumT, Ts...>
{
    using _base_base = detail::define_type_enum_base<TypeEnumT, Ts...>;
    using _base_base::_base_base;
    using base = define_type_enum;
};


    //
    // Anonymous type enumeration.
    //ᅟ
    //ᅟ    using FloatTypes = type_enum<float, double>;
    //ᅟ
    //ᅟ    FloatTypes floatType = ...;
    //ᅟ    switch (floatType)
    //ᅟ    {
    //ᅟ    case type_c<float>:  ...; break;
    //ᅟ    case type_c<double>: ...; break;
    //ᅟ    }
    //
template <typename... Ts>
class type_enum final : public define_type_enum<type_enum<Ts...>, Ts...>
{
    using _base_base = define_type_enum<type_enum<Ts...>, Ts...>;
    using _base_base::_base_base;
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_ENUM_HPP_
