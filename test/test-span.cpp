
#include <makeshift/span.hpp> // for soa_span<>

#include <gsl-lite/gsl-lite.hpp> // for index, dim

#include <catch2/catch.hpp>


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


TEST_CASE("soa_span<>")
{
    std::size_t n = 42;
    auto ivals = std::vector<int>(n);
    auto uvals = std::vector<unsigned>(n);

    for (gsl::index i = 0, c = gsl::dim(n); i < c; ++i)
    {
        ivals[i] = int(i);
    }

    auto s = mk::make_soa_span(gsl::make_span(ivals), gsl::make_span(uvals));

    auto ss = s.subspan(2, 3);
    for (auto&& ref : ss)
    {
        using std::get;
        get<1>(ref) = unsigned(get<0>(ref));
    }
    for (gsl::index i = 0, c = gsl::dim(n); i < c; ++i)
    {
        CHECK(uvals[i] == (i >= 2 && i < 5 ? unsigned(i) : 0));
    }

    auto d = s.end() - s.begin();
    CHECK(d == std::ptrdiff_t(n));
}


} // anonymous namespace
