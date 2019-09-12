
#include <iostream>

#include <makeshift/new.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("new", "[new]")
{
    std::cout << "Large page size: " << mk::hardware_large_page_size() << " B\n";
    std::cout << "Page size: " << mk::hardware_page_size() << " B\n";
    std::cout << "Cache line size: " << mk::hardware_cache_line_size() << " B\n";
}
