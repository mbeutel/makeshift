
#include <list>
#include <array>
#include <iterator>

#include <gsl-lite/gsl-lite.hpp>

#if gsl_CPP20_OR_GREATER
# include <ranges>
#endif // gsl_CPP20_OR_GREATER

#include <makeshift/ranges.hpp>

#include <catch2/catch_test_macros.hpp>


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


#if gsl_CPP20_OR_GREATER

//static_assert(std::default_initializable<mk::index_iterator>);
//static_assert(std::copyable<mk::index_iterator>);
//static_assert(std::semiregular<mk::index_iterator>);
//static_assert(std::regular<mk::index_iterator>);
//static_assert(std::movable<mk::index_iterator>);
//static_assert(std::weakly_incrementable<mk::index_iterator>);
//static_assert(std::indirectly_readable<mk::index_iterator>);
//static_assert(std::input_or_output_iterator<mk::index_iterator>);
//static_assert(std::input_iterator<mk::index_iterator>);
//static_assert(std::forward_iterator<mk::index_iterator>);
//static_assert(std::bidirectional_iterator<mk::index_iterator>);
static_assert(std::random_access_iterator<mk::index_iterator>);

static_assert(std::ranges::random_access_range<mk::index_range>);
static_assert(std::ranges::bidirectional_range<mk::range<std::list<int>::iterator>>);
static_assert(std::ranges::random_access_range<mk::range<std::array<int, 1>::iterator>>);
static_assert(std::ranges::random_access_range<mk::range<std::array<int, 1>::iterator, std::array<int, 1>::iterator, 1>>);

#endif // gsl_CPP20_OR_GREATER

// TODO: add tests


} // anonymous namespace
