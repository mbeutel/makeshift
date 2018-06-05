
#include <stdexcept>
#include <tuple>
#include <cctype>    // for isspace()
#include <optional>

#include <makeshift/serialize.hpp>


namespace makeshift
{

namespace detail
{

[[noreturn]] void raiseInvalidValueError(void)
{
    throw std::logic_error("invalid value");
}
[[noreturn]] void raiseInvalidStringError(const std::string& str, std::string_view typeDesc = { }, std::string_view typeName = { })
{
    std::string msg;
    if (!typeDesc.empty())
        msg = "'" + str + "' is not a valid " + std::string(typeDesc);
    else if (!typeName.empty())
        msg = "'" + str + "' is not a valid value of type '" + std::string(typeName) + "'";
    else
        msg = "unrecognized value '" + str + "'";
    throw std::runtime_error(msg); // TODO: use more appropriate exception class
}

std::string enum_to_string(gsl::span<const enum_value_serialization_context> knownValues,
    std::uint64_t enumValue)
{
    for (auto& knownValue : knownValues)
        if (knownValue.value == enumValue)
            return std::string(knownValue.string);
    raiseInvalidValueError();
}

constexpr bool isPowerOf2(std::uint64_t value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}

std::string flags_enum_to_string(gsl::span<const enum_value_serialization_context> knownValues,
    std::uint64_t enumValue)
{
    std::string result;

        // match value to combined flags, then to non-combined flags, then to remaining combined flags permitting overlap
    std::uint64_t matchedEnumValue = enumValue;
    for (auto [combined, permitOverlap] : { std::make_tuple(true, false), std::make_tuple(false, false), std::make_tuple(true, true) })
    {
        for (auto& knownValue : knownValues)
            if (isPowerOf2(knownValue.value) != combined)
            {
                std::uint64_t testValue = permitOverlap ? enumValue : matchedEnumValue;
                if ((testValue & knownValue.value) == knownValue.value)
                {
                    matchedEnumValue &= ~knownValue.value;
                    if (!result.empty())
                        result += ", ";
                    result += std::string(knownValue.string);
                }
            }
        if (matchedEnumValue == 0)
            break;
    }
    if (matchedEnumValue != 0)
        raiseInvalidValueError();

    return result;
}
bool try_string_to_enum(std::uint64_t& enumValue,
    gsl::span<const enum_value_serialization_context> knownValues,
    const std::string& string) noexcept
{
    for (auto& knownValue : knownValues)
        if (knownValue.string == string)
        {
            enumValue = knownValue.value;
            return true;
        }
    return false;
}
std::uint64_t string_to_enum(
    gsl::span<const enum_value_serialization_context> knownValues, std::string_view typeName, std::string_view typeDesc,
    const std::string& string)
{
    std::uint64_t enumValue;
    if (!try_string_to_enum(enumValue, knownValues, string))
        raiseInvalidStringError(string);
    return enumValue;
}
static std::string_view skipWhitespace(std::string_view s) noexcept
{
    while (!s.empty() && std::isspace(s.front()))
        s = s.substr(1);
    return s;
}
static bool isSeparator(char c) noexcept
{
    return c = ',' || c == '|';
}
static std::optional<std::string_view> expectSeparator(std::string_view s) noexcept
{
    s = skipWhitespace(s);
    if (s.empty() || isSeparator(s[0]))
        return std::nullopt;
    s = s.substr(1);
    return s;
}
bool try_string_to_flags_enum(std::uint64_t& enumValue,
    gsl::span<const enum_value_serialization_context> knownValues,
    const std::string& string) noexcept
{
    std::string_view sv = string;
    enumValue = 0;
    bool first = true;
    while (!sv.empty())
    {
        if (first)
            first = false;
        else
        {
            auto nsv = expectSeparator(sv);
            if (nsv)
                sv = *nsv;
            else
                return false; // syntax error: expected separator
        }
        sv = skipWhitespace(sv);
        bool haveMatch = false;
        for (auto& knownValue : knownValues)
        {
            auto len = knownValue.string.size();
            if (sv.size() >= len // does it fit?
                && knownValue.string == sv.substr(0, len) // does it match?
                && (sv.size() == len || std::isspace(sv[len]) || isSeparator(sv[len]))) // is it followed by whitespace, separator or EOS?
            {
                enumValue |= knownValue.value;
                sv = sv.substr(len);
                haveMatch = true;
                break;
            }
        }
        if (!haveMatch)
            return false;
    }
    return true;
}
std::uint64_t string_to_flags_enum(
    gsl::span<const enum_value_serialization_context> knownValues, std::string_view typeName, std::string_view typeDesc,
    const std::string& string)
{
    std::uint64_t enumValue;
    if (!try_string_to_flags_enum(enumValue, knownValues, string))
        raiseInvalidStringError(string);
    return enumValue;
}

} // namespace detail

} // namespace makeshift
