
#include <algorithm> // for find()

#include <makeshift/string.hpp>           // for string_equal_to
#include <makeshift/serializers/bool.hpp>


namespace makeshift
{

inline namespace serialize
{


std::vector<bool_serialization_options_t::bool_string> bool_serialization_options_t::default_strings(void)
{
    return {
        { "true",    true }, { "false",    false },
        { "yes",     true }, { "no",       false },
        { "1",       true }, { "0",        false },
        { "y",       true }, { "n",        false },
        { "on",      true }, { "off",      false },
        { "enabled", true }, { "disabled", false }
    };
}

[[noreturn]] static void raise_bool_error(const std::string& str, const bool_serialization_options_t& options)
{
    std::string msg = "invalid boolean value; expected one of: ";
    bool first = true;
    
        // check if true_string and false_string are duplicates, don't print them if they are
    string_equal_to comparer{ options.comparison };
    auto append = [&first, &msg](std::string_view valueStr)
    {
        if (first)
            first = false;
        else
            msg += ", ";
        msg += std::string(valueStr);
    };
    auto haveStr = [comparer, &options](std::string_view needle)
    {
        return std::find_if(options.strings.begin(), options.strings.end(),
            [needle, comparer] (const auto& entry)
            {
                return comparer(entry.str, needle);
            }) == options.strings.end();
    };
    if (!haveStr(options.false_string))
        append(options.false_string);
    if (!haveStr(options.true_string))
        append(options.true_string);
    for (auto& value : options.strings)
        append(value.str);

    throw parse_error(msg, str, 0);
}

void bool_serializer_args::from_stream_impl_(bool& value, const std::string& str) const
{
    string_equal_to comparer{ bool_options.comparison };
    if (comparer(str, bool_options.true_string))
        value = true;
    else if (comparer(str, bool_options.false_string))
        value = false;
    else
    {
        auto it = std::find_if(bool_options.strings.begin(), bool_options.strings.end(),
            [&str, comparer]
            (const auto& entry)
            {
                return comparer(entry.str, str);
            });
        if (it != bool_options.strings.end())
            value = it->value;
        else
            raise_bool_error(str, bool_options);
    }
}


} // inline namespace serialize

} // namespace makeshift