
#include <stdexcept>
#include <tuple>
#include <cctype>    // for isspace(), tolower()
#include <optional>
#include <algorithm> // for equal()

#include <makeshift/detail/serialize-enum.hpp>

#include <makeshift/arithmetic.hpp> // for checked_cast<>()


namespace makeshift
{

inline namespace utility
{


bool string_equals_case_insensitive(std::string_view lhs, std::string_view rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}


} // inline namespace utility


namespace detail
{


[[noreturn]] static void raiseInvalidValueError(void)
{
    throw std::logic_error("invalid value");
}
[[noreturn]] static void raiseInvalidStringError(const std::string& str, std::string_view typeDesc = { }, std::string_view typeName = { })
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


unsigned scalar_from_string(tag_t<unsigned>, const std::string& string)
{
    return checked_cast<unsigned>(std::stoul(string));
}


static bool string_equals(std::string_view lhs, std::string_view rhs)
{
    return lhs == rhs;
}

std::string enum_to_string(std::uint64_t enumValue, const enum_serialization_data_ref& sdata, const enum_serialization_options_t& /*options*/)
{
    for (auto& value : sdata.values)
        if (value.value == enumValue)
            return std::string(value.string);
    raiseInvalidValueError();
}
bool try_string_to_enum(std::uint64_t& enumValue, const std::string& string, const enum_serialization_data_ref& sdata, const enum_serialization_options_t& options) noexcept
{
    auto equalsFunc = options.case_sensitive
        ? string_equals
        : string_equals_case_insensitive;
    for (auto& value : sdata.values)
        if (equalsFunc(value.string, string))
        {
            enumValue = value.value;
            return true;
        }
    return false;
}
std::uint64_t string_to_enum(const std::string& string, const enum_serialization_data_ref& sdata, const enum_serialization_options_t& options)
{
    std::uint64_t enumValue;
    if (!try_string_to_enum(enumValue, string, sdata, options))
        raiseInvalidStringError(string, sdata.typeDesc, sdata.typeName);
    return enumValue;
}

static constexpr bool isPowerOf2(std::uint64_t value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}
std::string flags_enum_to_string(std::uint64_t enumValue, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options_t& /*options*/)
{
    std::string result;

        // match value to combined flags, then to non-combined flags, then to remaining combined flags permitting overlap
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
                    if (!result.empty())
                        result += ", ";
                    result += std::string(value.string);
                }
            }
        if (matchedEnumValue == 0)
            break;
    }
    if (matchedEnumValue != 0)
        raiseInvalidValueError();

    return result;
}

static std::string_view skipWhitespace(std::string_view s) noexcept
{
    while (!s.empty() && std::isspace(s.front()))
        s = s.substr(1);
    return s;
}
static bool isSeparator(char c) noexcept
{
    return c == ',' || c == '|';
}
static std::optional<std::string_view> expectSeparator(std::string_view s) noexcept
{
    s = skipWhitespace(s);
    if (s.empty() || isSeparator(s[0]))
        return std::nullopt;
    s = s.substr(1);
    return s;
}
bool try_string_to_flags_enum(std::uint64_t& enumValue, const std::string& string, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options_t& options) noexcept
{
    auto equalsFunc = options.case_sensitive
        ? string_equals
        : string_equals_case_insensitive;
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
        for (auto& value : sdata.values)
        {
            auto len = value.string.size();
            if (sv.size() >= len // does it fit?
                && equalsFunc(value.string, sv.substr(0, len)) // does it match?
                && (sv.size() == len || std::isspace(sv[len]) || isSeparator(sv[len]))) // is it followed by whitespace, separator or EOS?
            {
                enumValue |= value.value;
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
std::uint64_t string_to_flags_enum(const std::string& string, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options_t& options)
{
    std::uint64_t enumValue;
    if (!try_string_to_flags_enum(enumValue, string, sdata, options))
        raiseInvalidStringError(string, sdata.typeDesc, sdata.flagTypeName);
    return enumValue;
}


} // namespace detail

} // namespace makeshift
