
#include <string>
#include <string_view>
#include <stdexcept>

#include <makeshift/serialize.hpp>


namespace makeshift
{

inline namespace serialize
{


static std::string mark_column_position(std::string_view string, std::size_t column)
{
    if (column > string.size())
        return std::string(string);
    if (column == string.size())
        return std::string(string) + " ^";
    if (column == 0)
        return "^ " + std::string(string);
    return std::string(string.substr(0, column)) + " ^ " + std::string(string.substr(column));
}

static std::string concat_message(std::string_view error, std::string_view context, std::size_t column)
{
    return std::string(error) + "\ncontext: \"" + mark_column_position(context, column) + "\"";
}
static std::string concat_message(std::string_view error, std::size_t column)
{
    if (column == std::size_t(-1))
        return std::string(error);
    return std::string(error) + "\nat column " + std::to_string(column);
}

parse_error::parse_error(std::string_view _error)
    : std::runtime_error(std::string(_error)), error_(_error), column_(std::size_t(-1))
{
}
parse_error::parse_error(std::string_view _error, std::size_t _column)
    : std::runtime_error(concat_message(_error, _column)), error_(_error), column_(_column)
{
}
parse_error::parse_error(std::string_view _error, std::string_view _context, std::size_t _column)
    : std::runtime_error(concat_message(_error, _context, _column)), error_(_error), context_(_context), column_(_column)
{
}


} // inline namespace serialize

} // namespace makeshift
