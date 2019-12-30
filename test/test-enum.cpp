
#include <makeshift/enum.hpp>
#include <makeshift/experimental/enum.hpp>

#include <gsl-lite/gsl-lite.hpp>

#include <catch2/catch.hpp>


namespace {


namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


using FloatTypes = mk::type_enum<float, double>;


TEST_CASE("enum")
{
    SECTION("type-enum")
    {
        FloatTypes floatTypes = mk::type_c<float>;
        CHECK(floatTypes != mk::type_c<double>);

            // The following switch statement should be warning-free. Omitting one of the cases should yield a warning.
        switch (floatTypes)
        {
        case mk::type<float>{ }:
            break;
        case mk::type_c<double>:
            break;
        }
    }
}


} // anonymous namespace
