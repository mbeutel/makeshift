
#include <list>
#include <tuple>
#include <array>
#include <vector>
#include <iterator>

#include <makeshift/algorithm.hpp>

#include <iterator>

#include <gsl-lite/gsl-lite.hpp>

#include <catch2/catch.hpp>


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


struct MyInputIteratorTag : std::input_iterator_tag { };
struct MyOutputIteratorTag : std::output_iterator_tag { };
struct MyForwardIteratorTag : std::forward_iterator_tag { };
struct MyBidirectionalIteratorTag : std::bidirectional_iterator_tag { };

static_assert( std::is_base_of<std::input_iterator_tag, makeshift::detail::common_iterator_tag<MyInputIteratorTag, MyForwardIteratorTag>>::value, "static assertion failed");
static_assert( std::is_base_of<std::input_iterator_tag, makeshift::detail::common_iterator_tag<MyInputIteratorTag, MyForwardIteratorTag>>::value, "static assertion failed");
static_assert( std::is_base_of<std::forward_iterator_tag, makeshift::detail::common_iterator_tag<MyBidirectionalIteratorTag, MyForwardIteratorTag>>::value, "static assertion failed");
static_assert(!std::is_base_of<std::forward_iterator_tag, makeshift::detail::common_iterator_tag<MyInputIteratorTag, MyForwardIteratorTag>>::value, "static assertion failed");
static_assert(!std::is_base_of<std::input_iterator_tag, makeshift::detail::common_iterator_tag<std::input_iterator_tag, std::output_iterator_tag>>::value, "static assertion failed");
static_assert(!std::is_base_of<std::output_iterator_tag, makeshift::detail::common_iterator_tag<std::input_iterator_tag, std::output_iterator_tag>>::value, "static assertion failed");


TEST_CASE("range_zip()")
{
    auto vec0 = std::vector<int>{ };
    auto list0 = std::list<int>{ };

    auto arr3 = std::array<int, 3>{ 21, 22, 23 };
    auto vec3 = std::vector<int>{ 1, 2, 3 };
    auto list3 = std::list<int>{ 11, 12, 13 };

    auto vec4 = std::vector<int>{ 1, 2, 3, 4 };

    SECTION("empty")
    {
        auto i_v_l = mk::range_zip(mk::range_index, list0, vec0);
        using It = std::decay_t<decltype(i_v_l.begin())>;
        static_assert(std::is_base_of<std::bidirectional_iterator_tag, std::iterator_traits<It>::iterator_category>::value, "static assertion failed");
        CHECK(i_v_l.size() == 0);
        for (auto&& e : i_v_l)
        {
            (void) e;
            FAIL_CHECK();
        }
    }
    SECTION("basic use")
    {
        auto l_v = mk::range_zip(list3, vec3);
        int i = 0;
        CHECK(l_v.size() == 3);
        for (auto&& e : l_v)
        {
            using std::get;
            CHECK(get<0>(e) == i + 11);
            CHECK(get<1>(e) == i + 1);
            ++i;
        }
        CHECK(i == 3);
        auto it = l_v.begin();
        auto end = l_v.end();
        CHECK(it != end);
        CHECK(it++ != end);
        CHECK(--it == l_v.begin());
        auto oldIt = it;
        CHECK(++it != oldIt);
    }
    SECTION("basic use with index")
    {
        auto i_v_l = mk::range_zip(mk::range_index, vec3, list3);
        int i = 0;
        for (auto&& e : i_v_l)
        {
            using std::get;
            CHECK(get<0>(e) == i);
            CHECK(get<1>(e) == i + 1);
            CHECK(get<2>(e) == i + 11);
            ++i;
        }
        CHECK(i == 3);
    }
    SECTION("random access")
    {
        auto a_v_i = mk::range_zip(arr3, vec3, mk::range_index);
        CHECK(a_v_i.size() == 3);
        CHECK(a_v_i[1] == std::make_tuple(22, 2, 1));
        auto it = a_v_i.begin();
        auto end = a_v_i.end();
        CHECK(it != end);
        CHECK(it <= a_v_i.begin());
        it += 1;
        CHECK(it > a_v_i.begin());
        CHECK(it[1] == std::make_tuple(23, 3, 2));
        CHECK(it[-1] == std::make_tuple(21, 1, 0));
        it -= 0;
        CHECK(it - a_v_i.begin() == 1);
    }
    SECTION("tuple access")
    {
        // TODO: make zipped ranges tuple-like
        //auto i_v_l = mk::range_zip(mk::range_index, vec3, list3);
    }
    SECTION("error when trying to combine ranges with different sizes")
    {
        CHECK_THROWS(mk::range_zip(vec3, vec0));

        // This doesn't compile because mismatching compile-time sizes are detected at compile time.
        //CHECK_THROWS(mk::range_zip(arr0, arr3));
    }
    SECTION("error if size cannot be inferred")
    {
        // This doesn't compile because the inability to infer a size is detected at compile time.
        //CHECK_THROWS(mk::range_zip(mk::range_index));
    }
}

TEST_CASE("range_for()")
{
    auto vec3 = std::vector<int>{ 1, 2, 3 };
    auto list3 = std::list<int>{ 11, 12, 13 };

    SECTION("basic use with index")
    {
        int i = 0;
        mk::range_for(
            [&](gsl::index iv, int& vv, int& lv)
            {
                CHECK(iv == i);
                CHECK(vv == i + 1);
                CHECK(lv == i + 11);
                ++i;
            },
            mk::range_index, vec3, list3);
        CHECK(i == 3);
    }
}

// TODO: add more tests


} // anonymous namespace
