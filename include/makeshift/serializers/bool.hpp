
#ifndef MAKESHIFT_SERIALIZERS_BOOL_HPP_
#define MAKESHIFT_SERIALIZERS_BOOL_HPP_


#include <string>
#include <string_view>
#include <vector>
#include <iosfwd>      // for istream, ostream
#include <utility>     // for move()

#include <makeshift/detail/cfg.hpp>  // for MAKESHIFT_DLLFUNC

#include <makeshift/serialize.hpp>   // for define_serializer<>
#include <makeshift/streamable.hpp>  // for streamable()

#include <makeshift/serializers/hint.hpp> // for hint_options


namespace makeshift
{

inline namespace serialize
{


    //ᅟ
    // Options for serializing and deserializing booleans.
    //
struct bool_serializer_options
{
    struct bool_string
    {
        std::string str;
        bool value;
    };

    MAKESHIFT_DLLFUNC static std::vector<bool_string> default_strings(void);

    std::vector<bool_string> strings = default_strings();
    std::string_view true_string = "true";
    std::string_view false_string = "false";
    string_comparison comparison = string_comparison::ordinal_ignore_case;

    bool_serializer_options(void) = default;
    bool_serializer_options(std::string_view _true_string, std::string_view _false_string) : true_string(_true_string), false_string(_false_string) { }
    bool_serializer_options(std::string_view _true_string, std::string_view _false_string, std::vector<bool_string> _strings)
        : strings(std::move(_strings)), true_string(_true_string), false_string(_false_string)
    {
    }
    bool_serializer_options(std::vector<bool_string> _strings)
        : strings(std::move(_strings))
    {
    }
};


} // inline namespace serialize


namespace detail
{


MAKESHIFT_DLLFUNC void bool_from_stream(const bool_serializer_options& options, bool& value, const std::string& str);


} // namespace detail


inline namespace serialize
{


    //ᅟ
    // Stream serializer which serializes booleans from and to a number of common string representations
    // (yes/no, true/false, on/off, enabled/disabled, 1/0, y/n).
    //
template <typename BaseT = void>
    struct bool_serializer : define_serializer<bool_serializer, BaseT, bool_serializer_options>
{
    using base = define_serializer<makeshift::bool_serializer, BaseT, bool_serializer_options>;
    using base::base;

    template <typename T, typename SerializerT>
        friend std::enable_if_t<std::is_same<T, bool>::value>
        to_stream_impl(const T& value, std::ostream& stream, const bool_serializer& boolSerializer, SerializerT&& serializer)
    {
        stream << streamable(std::string(value ? boolSerializer.data.true_string : boolSerializer.data.false_string), serializer);
    }
    template <typename SerializerT>
        friend void from_stream_impl(bool& value, std::istream& stream, const bool_serializer& boolSerializer, SerializerT&& serializer)
    {
        std::string str;
        stream >> streamable(str, serializer);
        makeshift::detail::bool_from_stream_impl(boolSerializer.data, value, str);
    }

    template <typename SerializerT>
        friend std::string get_hint_impl(tag<bool>, const bool_serializer& boolSerializer, SerializerT&& serializer)
    {
        return boolSerializer.data.false_string + get_data(serializer, tag_v<hint_options>).option_separator + boolSerializer.data.true_string;
    }
};
bool_serializer(void) -> bool_serializer<>;
bool_serializer(const bool_serializer_options&) -> bool_serializer<>;
bool_serializer(bool_serializer_options&&) -> bool_serializer<>;


} // inline namespace serialize

} // namespace makeshift


#endif // MAKESHIFT_SERIALIZERS_BOOL_HPP_
