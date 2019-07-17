
#include <makeshift/new.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("new", "[new]")
{
    mk::hardware_page_size();
    mk::hardware_cache_line_size();
}
