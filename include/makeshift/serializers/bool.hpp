﻿
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


namespace makeshift
{

inline namespace serialize
{


template <typename BaseT = void>
    struct bool_serializer_t;


    //ᅟ
    // Options for serializing and deserializing booleans.
    //
struct bool_serialization_options_t
{
    struct bool_string
    {
        std::string_view str;
        bool value;
    };

    MAKESHIFT_DLLFUNC static std::vector<bool_string> default_strings(void);

    std::vector<bool_string> strings = default_strings();
    std::string_view true_string = "true";
    std::string_view false_string = "false";
    string_comparison comparison = string_comparison::ordinal_ignore_case;

    bool_serialization_options_t(void) = default;
    bool_serialization_options_t(std::string_view _true_string, std::string_view _false_string) : true_string(_true_string), false_string(_false_string) { }
    bool_serialization_options_t(std::string_view _true_string, std::string_view _false_string, std::vector<bool_string> _strings)
        : strings(std::move(_strings)), true_string(_true_string), false_string(_false_string)
    {
    }
    bool_serialization_options_t(std::vector<bool_string> _strings)
        : strings(std::move(_strings))
    {
    }
};


    //ᅟ
    // Runtime arguments for `bool_serializer_t`.
    //
struct bool_serializer_args
{
    template <typename BaseT>
        friend struct bool_serializer_t;

    bool_serialization_options_t bool_options;
    
    bool_serializer_args(void) = default;
    bool_serializer_args(bool_serialization_options_t _bool_options) noexcept : bool_options(std::move(_bool_options)) { }

private:
    MAKESHIFT_DLLFUNC void from_stream_impl_(bool& value, const std::string& str) const;
};


    //ᅟ
    // Stream serializer which serializes booleans from and to a number of common string representations
    // (yes/no, true/false, on/off, enabled/disabled, 1/0, y/n).
    //
template <typename BaseT>
    struct bool_serializer_t : define_serializer<bool_serializer_t, BaseT, bool_serializer_args>
{
    using base = define_serializer<makeshift::bool_serializer_t, BaseT, bool_serializer_args>;
    using base::base;

    template <typename T, typename SerializerT>
        friend std::enable_if_t<std::is_same<T, bool>::value>
        to_stream_impl(const T& value, std::ostream& stream, const bool_serializer_t& boolSerializer, SerializerT& serializer)
    {
        stream << streamable(std::string(value ? boolSerializer.true_string : boolSerializer.false_string), serializer);
    }
    template <typename SerializerT>
        friend void from_stream_impl(bool& value, std::istream& stream, const bool_serializer_t& boolSerializer, SerializerT& serializer)
    {
        std::string str;
        stream >> streamable(str, serializer);
        this->from_stream_impl_(value, str);
    }

    template <typename SerializerT>
        friend std::string get_hint_impl(tag<bool>, const bool_serializer_t& boolSerializer, SerializerT& serializer)
    {
        return boolSerializer.bool_options.false_string + serializer.hint_options.option_separator + boolSerializer.bool_options.true_string;
    }
};

    //ᅟ
    // Stream serializer which serializes booleans from and to a number of common string representations
    // (yes/no, true/false, on/off, enabled/disabled, 1/0, y/n).
    //
inline const bool_serializer_t<> bool_serializer{ };


} // inline namespace serialize

} // namespace makeshift


#endif // MAKESHIFT_SERIALIZERS_BOOL_HPP_