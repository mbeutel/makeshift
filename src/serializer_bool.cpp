
#include <algorithm> // for find()

#include <makeshift/string.hpp>           // for string_equal_to
#include <makeshift/serializers/bool.hpp>


namespace makeshift
{

inline namespace serialize
{


std::vector<bool_serializer_options::bool_string> bool_serializer_options::default_strings(void)
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

[[noreturn]] static void raise_bool_error(const std::string& str, const bool_serializer_options& options)
{
    std::string msg = "invalid boolean value; expected one of: ";
    bool first = true;
    
        // check if true_string and false_string are duplicates, don't print them if they are
    auto comparer = equal_to(string_comparer(string_comparer_options{ options.comparison }));
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
                return comparer(std::string_view(entry.str), needle);
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

void bool_from_stream(const bool_serializer_options& options, bool& value, const std::string& str)
{
    auto comparer = equal_to(string_comparer(string_comparer_options{ options.comparison }));
    if (comparer(std::string_view(str), options.true_string))
        value = true;
    else if (comparer(std::string_view(str), options.false_string))
        value = false;
    else
    {
        auto it = std::find_if(options.strings.begin(), options.strings.end(),
            [&str, comparer]
            (const auto& entry)
            {
                return comparer(entry.str, str);
            });
        if (it != options.strings.end())
            value = it->value;
        else
            raise_bool_error(str, options);
    }
}


} // inline namespace serialize

} // namespace makeshift
