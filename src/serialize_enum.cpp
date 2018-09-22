
#include <stdexcept>
#include <sstream>
#include <string>
#include <tuple>
#include <cctype>    // for isspace()
#include <optional>
#include <exception> // for terminate()

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/detail/serialize_enum.hpp>

#include <makeshift/hint.hpp>      // for enum_hint_options
#include <makeshift/serialize.hpp> // for enum_serialization_options


namespace makeshift
{

namespace detail
{


static void enum_hint(std::ostream& stream, const enum_serialization_data_ref& sdata, const enum_hint_options& options)
{
    bool first = true;
    for (auto& value : sdata.values)
    {
        if (first)
            first = false;
        else
            stream << options.option_separator;
        stream << value.string;
    }
}
static void flags_enum_hint(std::ostream& stream, const flags_enum_serialization_data_ref& sdata, const enum_hint_options& options)
{
    bool first = true;
    for (auto& value : sdata.values)
    {
        if (first)
            first = false;
        else
            stream << options.flags_separator;
        stream << value.string;
    }
}

std::string enum_hint(const enum_serialization_data_ref& sdata, const enum_hint_options& options)
{
    std::ostringstream sstr;
    enum_hint(sstr, sdata, options);
    return sstr.str();
}
std::string flags_enum_hint(const flags_enum_serialization_data_ref& sdata, const enum_hint_options& options)
{
    std::ostringstream sstr;
    flags_enum_hint(sstr, sdata, options);
    return sstr.str();
}

[[noreturn]] static void raise_invalid_value_error(void)
{
    std::terminate();
}

static void enum_error_msg(std::ostream& stream, std::string_view typeDesc, std::string_view typeName)
{
    if (!typeDesc.empty())
        stream << "invalid " << typeDesc;
    else if (!typeName.empty())
        stream << "invalid value of type '" << typeName << "'";
    else
        stream << "unrecognized value";
}
[[noreturn]] static void raise_invalid_string_error(std::string_view string, const enum_serialization_data_ref& sdata)
{
    std::ostringstream sstr;
    enum_error_msg(sstr, sdata.typeDesc, sdata.typeName);
    sstr << "; expected one of: ";
    enum_hint_options options;
    options.option_separator = ", ";
    enum_hint(sstr, sdata, options);
    throw parse_error(sstr.str(), string);
}
[[noreturn]] static void raise_invalid_string_error(std::string_view string, std::string_view sv, const flags_enum_serialization_data_ref& sdata)
{
    std::ostringstream sstr;
    enum_error_msg(sstr, sdata.typeDesc, sdata.flagTypeName);
    sstr << "; expected a ','-joined subset of: none, ";
    enum_hint_options options;
    options.flags_separator = ", ";
    flags_enum_hint(sstr, sdata, options);
    std::size_t offset = std::size_t(sv.data() - string.data());
    throw parse_error(sstr.str(), string, offset);
}


std::string_view enum_to_string(std::uint64_t enumValue, const enum_serialization_data_ref& sdata, const enum_serialization_options&)
{
    for (auto& value : sdata.values)
        if (value.value == enumValue)
            return value.string;
    raise_invalid_value_error();
}
void enum_from_string(std::string_view string, std::uint64_t& enumValue, const enum_serialization_data_ref& sdata, const enum_serialization_options& options)
{
    auto stringComparer = equal_to(string_comparer(string_comparer_options{ options.enum_string_comparison_mode }));
    for (auto& value : sdata.values)
        if (stringComparer(value.string, string))
        {
            enumValue = value.value;
            return;
        }
    raise_invalid_string_error(string, sdata);
}

static constexpr bool isPowerOf2(std::uint64_t value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}
std::string flags_enum_to_string(std::uint64_t enumValue, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options& options)
{
        // match value to combined flags, then to non-combined flags, then to remaining combined flags permitting overlap
    std::string result;
    bool first = true;
    std::uint64_t matchedEnumValue = enumValue;
    for (auto [combined, permitOverlap] : { std::make_tuple(true, false), std::make_tuple(false, false), std::make_tuple(true, true) })
    {
        for (auto& value : sdata.values)
            if (isPowerOf2(value.value) != combined)
            {
                std::uint64_t testValue = permitOverlap ? enumValue : matchedEnumValue;
                if ((testValue & value.value) == value.value)
                {
                    matchedEnumValue &= ~value.value;
                    if (first)
                        first = false;
                    else
                        result += options.flags_separator;
                    result += std::string(value.string);
                }
            }
        if (matchedEnumValue == 0)
            break;
    }

    if (enumValue == 0 && result.empty())
        return std::string(options.none_string);
    else if (matchedEnumValue != 0)
        raise_invalid_value_error();

    return result;
}

static std::string_view skipWhitespace(std::string_view s) noexcept
{
    while (!s.empty() && std::isspace(s.front()))
        s = s.substr(1);
    return s;
}
static std::string_view trim(std::string_view s) noexcept
{
    s = skipWhitespace(s);
    while (!s.empty() && std::isspace(s.back()))
        s = s.substr(0, s.size() - 1);
    return s;
}
char trim_delim_char(std::string_view s)
{
    std::string_view st = trim(s);
    if (st.size() != 1)
        throw std::invalid_argument("invalid single-character delimiter '" + std::string(st) + "'");
    return st[0];
}
static std::optional<std::string_view> expectSeparator(std::string_view s, std::string_view sep) noexcept
{
    s = skipWhitespace(s);
    if (s.substr(0, sep.size()) != sep)
        return std::nullopt;
    return s.substr(sep.size());
}
void flags_enum_from_string(std::string_view string, std::uint64_t& enumValue, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options& options)
{
    auto stringComparer = equal_to(string_comparer(string_comparer_options{ options.enum_string_comparison_mode }));
    std::string_view sv = string;
    auto sep = trim(options.flags_separator);
    enumValue = 0;
    bool first = true;
    while (!sv.empty())
    {
        if (first)
            first = false;
        else
        {
            auto nsv = expectSeparator(sv, sep);
            if (nsv)
                sv = *nsv;
            else
                throw parse_error("syntax error: expected separator", string, std::size_t(sv.data() - string.data()));
        }

        sv = skipWhitespace(sv);
        bool haveMatch = false;
        for (auto& value : sdata.values)
        {
            auto len = value.string.size();
            if (stringComparer(value.string, sv.substr(0, len)) // does it match?
                && (sv.size() == len || std::isspace(sv[len]) || sv.substr(len, sep.size()) == sep)) // is it followed by EOS, whitespace, or separator?
            {
                enumValue |= value.value;
                sv = sv.substr(len);
                haveMatch = true;
                break;
            }
        }
        if (!haveMatch)
        {
            if (stringComparer(options.none_string, sv.substr(0, options.none_string.size())))
            {
                sv = sv.substr(options.none_string.size());
                haveMatch = true;
            }
        }
        if (!haveMatch && sv.size() != 0)
            raise_invalid_string_error(string, sv, sdata);
    }
}


} // namespace detail

} // namespace makeshift
