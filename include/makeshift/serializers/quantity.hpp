
#ifndef MAKESHIFT_SERIALIZERS_QUANTITY_HPP_
#define MAKESHIFT_SERIALIZERS_QUANTITY_HPP_


#include <string>
#include <string_view>
#include <chrono>      // for duration<>

#include <makeshift/detail/cfg.hpp>  // for MAKESHIFT_DLLFUNC

#include <makeshift/serialize.hpp>   // for define_serializer<>
#include <makeshift/streamable.hpp>  // for streamable()

#include <makeshift/quantity.hpp> // for quantity<>, dynamic_quantity<>


namespace makeshift
{

namespace detail
{


    // defined in serializer_stream.cpp
MAKESHIFT_DLLFUNC void unit_to_stream(std::ostream& stream, quantity_unit unit);
MAKESHIFT_DLLFUNC void unit_from_stream(std::istream& stream, quantity_unit& unit);

    // defined in serializer_quantity.cpp
MAKESHIFT_DLLFUNC void time_unit_to_stream(std::ostream& stream, std::intmax_t num, std::intmax_t den);
MAKESHIFT_DLLFUNC void time_unit_from_stream(std::istream& stream, std::intmax_t& num, std::intmax_t& den);
MAKESHIFT_DLLFUNC void time_unit_combine(std::intmax_t& num, std::intmax_t& den, std::intmax_t dstNum, std::intmax_t dstDen);


} // namespace detail


inline namespace serialize
{


struct quantity_serializer_options
{
    std::string_view unit_separator = " ";

    constexpr quantity_serializer_options(void) = default;
};


template <typename BaseT = void>
    struct quantity_serializer : define_serializer<quantity_serializer, BaseT, quantity_serializer_options>
{
    using base = define_serializer<makeshift::quantity_serializer, BaseT, quantity_serializer_options>;
    using base::base;

    template <typename RepT, typename PeriodT, typename SerializerT>
        friend void to_stream_impl(const std::chrono::duration<RepT, PeriodT>& value, std::ostream& stream, const quantity_serializer& quantitySerializer, SerializerT& serializer)
    {
        stream << streamable(value.count(), serializer)
               << data(quantitySerializer).unit_separator;
        makeshift::detail::time_unit_to_stream(stream, PeriodT::type::num, PeriodT::type::den);
    }
    template <typename T, quantity_unit Unit, typename ConverterT, typename SerializerT>
        friend void to_stream_impl(const quantity<T, Unit, ConverterT>& value, std::ostream& stream, const quantity_serializer&, SerializerT& serializer)
    {
        stream << streamable(dynamic_quantity{ value }, serializer);
    }
    template <typename T, typename ConverterT, typename SerializerT>
        friend void to_stream_impl(const dynamic_quantity<T, ConverterT>& value, std::ostream& stream, const quantity_serializer& quantitySerializer, SerializerT& serializer)
    {
        stream << streamable(value.value(), serializer)
               << data(quantitySerializer).unit_separator;
        makeshift::detail::unit_to_stream(stream, value.unit());
    }
    template <typename RepT, typename PeriodT, typename SerializerT>
        friend void from_stream_impl(std::chrono::duration<RepT, PeriodT>& value, std::istream& stream, const quantity_serializer&, SerializerT& serializer)
    {
        RepT lvalue;
        stream >> streamable(lvalue, serializer);
        std::intmax_t num, den;
        makeshift::detail::time_unit_from_stream(stream, num, den);
        makeshift::detail::time_unit_combine(num, den, PeriodT::type::num, PeriodT::type::den);
        value = std::chrono::duration<RepT, PeriodT>{ RepT((lvalue * num) / den) };
    }
    template <typename T, quantity_unit Unit, typename ConverterT, typename SerializerT>
        friend void from_stream_impl(quantity<T, Unit, ConverterT>& value, std::istream& stream, const quantity_serializer&, SerializerT& serializer)
    {
        dynamic_quantity<T, ConverterT> lvalue;
        stream >> streamable(lvalue, serializer);
        value = lvalue;
    }
    template <typename T, typename ConverterT, typename SerializerT>
        friend void from_stream_impl(dynamic_quantity<T, ConverterT>& value, std::istream& stream, const quantity_serializer&, SerializerT& serializer)
    {
        T lvalue;
        quantity_unit unit;
        stream >> streamable(lvalue, serializer);
        makeshift::detail::unit_from_stream(stream, unit);
        value = dynamic_quantity<T, ConverterT>{ lvalue, unit };
    }
};
quantity_serializer(void) -> quantity_serializer<>;
quantity_serializer(const quantity_serializer_options&) -> quantity_serializer<>;
quantity_serializer(quantity_serializer_options&&) -> quantity_serializer<>;


} // inline namespace serialize

} // namespace makeshift


#endif // MAKESHIFT_SERIALIZERS_QUANTITY_HPP_
