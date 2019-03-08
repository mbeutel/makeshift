
#include <variant>

#include <makeshift/metadata2.hpp>
#include <makeshift/variant2.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


enum class Precision
{
    single,
    double_
};
constexpr inline auto reflect(mk::type<Precision>)
{
    return mk::values<Precision> = {
        Precision::single,
        Precision::double_
    };
}

using FloatType = std::variant<mk::type<float>, mk::type<double>>;

struct Params
{
    Precision precision;
    int gangSize;
    int numThreadsX;
    int numThreadsY;

    friend constexpr bool operator ==(const Params& lhs, const Params& rhs) noexcept
    {
        return lhs.precision == rhs.precision
            && lhs.gangSize == rhs.gangSize
            && lhs.numThreadsX == rhs.numThreadsX
            && lhs.numThreadsY == rhs.numThreadsY;
    }
    friend constexpr bool operator !=(const Params& lhs, const Params& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

template <typename...> class TD;

TEST_CASE("variant2")
{
    
    SECTION("product")
    {
        constexpr auto a1 = mk::values<Params> = { { Precision::single, 4, 32, 32 }, { Precision::double_, 2, 32, 32 } };
        (void) a1;
        constexpr auto a2 = mk::member_values(&Params::precision) = { Precision::single, Precision::double_ };
        (void) a2;
        constexpr auto a3 =  mk::member_values(&Params::precision)
                          * (mk::member_values(&Params::gangSize) = { 1, 2, 4 })
                          * (mk::member_values(&Params::numThreadsX, &Params::numThreadsY) = { { 16, 16 }, { 32, 32 } });
        constexpr auto allValues = to_array(a3);
        (void) allValues;
    }
    
    SECTION("expand")
    {
        auto p1 = Precision::double_;
        auto p1V = mk::expand2(p1);
        std::visit([p1](auto p1R)
            {
                constexpr Precision p1C = mk::retrieve(p1R);
                CHECK(p1C == p1);
            },
            p1V);

        auto p2 = Precision::double_;
        auto p2VO = mk::try_expand2(p2,
            []
            {
                return mk::values<Precision> = { Precision::single, Precision::double_ };
            });
        CHECK(p2VO.has_value());
        std::visit([p2](auto p2R)
            {
                constexpr Precision p2C = mk::retrieve(p2R);
                CHECK(p2C == p2);
            },
            *p2VO);

        auto p3 = Precision::double_;
        auto p3VO = mk::try_expand2(p3,
            []
            {
                return mk::values<Precision> = { Precision::single };
            });
        CHECK_FALSE(p3VO.has_value());

        auto s1 = Params{ Precision::double_, 2, 32, 32 };
        auto s1VO = mk::try_expand2(s1,
            []
            {
                return mk::values<Params> = {
                    { Precision::single, 4, 32, 32 },
                    { Precision::double_, 2, 32, 32 }
                };
            });
        CHECK(s1VO.has_value());
        std::visit([s1](auto s1R)
            {
                constexpr Params s1C = mk::retrieve(s1R);
                CHECK(s1C == s1);
            },
            *s1VO);

        auto s2 = Params{ Precision::double_, 2, 32, 32 };
        auto s2VO = mk::try_expand2(s2,
            []
            {
                return mk::member_values(&Params::precision)
                    * (mk::member_values(&Params::gangSize) = { 1, 2, 4 })
                    * (mk::member_values(&Params::numThreadsX, &Params::numThreadsY) = { { 16, 16 }, { 32, 32 } });
            });
        CHECK(s2VO.has_value());
        std::visit([s2](auto s2R)
            {
                constexpr Params s2C = mk::retrieve(s2R);
                CHECK(s2C == s2);
            },
            *s2VO);

        auto v1 = FloatType{ mk::type_v<float> };
        auto v1VO = mk::try_expand2(v1,
            []
            {
                return mk::values<FloatType> = {
                    FloatType{ mk::type_v<float> },
                    FloatType{ mk::type_v<double> }
                };
            });
        CHECK(v1VO.has_value());
        std::visit([v1](auto v1R)
            {
                constexpr auto v1CV = mk::retrieve(v1R);
                constexpr auto v1C = mk::retrieve_variant(v1R);
                //constexpr std::size_t v1CI = v1CV.index();
                //constexpr auto v1C = std::get<v1CI>(v1CV);
                //constexpr auto v1C = std::get<v1CV.index()>(v1CV);
                //CHECK(v1C == v1);
            },
            *v1VO);
    }
}
