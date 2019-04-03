
#include <variant>
#include <iostream>
#include <exception>

#include <makeshift/utility2.hpp>
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
    return mk::define_metadata(
        mk::values(
            Precision::single,
            Precision::double_
        )
    );
}
std::ostream& operator <<(std::ostream& stream, Precision precision)
{
    switch (precision)
    {
    case Precision::single:
        return stream << "single";
    case Precision::double_:
        return stream << "double";
    default:
        std::terminate();
    }
}

struct ExhaustibleParams
{
    Precision precision = Precision::single;
    bool transmogrify = false;

    friend constexpr bool operator ==(const ExhaustibleParams& lhs, const ExhaustibleParams& rhs) noexcept
    {
        return lhs.precision == rhs.precision
            && lhs.transmogrify == rhs.transmogrify;
    }
    friend constexpr bool operator !=(const ExhaustibleParams& lhs, const ExhaustibleParams& rhs) noexcept
    {
        return !(lhs == rhs);
    }
    friend std::ostream& operator <<(std::ostream& stream, const ExhaustibleParams& self)
    {
        return stream << "{ precision=" << self.precision << ", transmogrify=" << self.transmogrify << " }";
    }
};

using FloatType = mk::type_enum<float, double>;
using FloatTypeVariant = std::variant<mk::type<float>, mk::type<double>>;

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
    friend std::ostream& operator <<(std::ostream& stream, const Params& self)
    {
        return stream << "{ precision=" << self.precision << ", gangSize=" << self.gangSize << ", numThreadsX=" << self.numThreadsX << ", numThreadsY=" << self.numThreadsY << " }";
    }
};


TEST_CASE("variant2")
{
    
    SECTION("product")
    {
        constexpr auto a1 = mk::values(Params{ Precision::single, 4, 32, 32 }, Params{ Precision::double_, 2, 32, 32 });
        (void) a1;
        constexpr auto a2 = mk::member_values(&Params::precision) = { Precision::single, Precision::double_ };
        (void) a2;
        constexpr auto a3 =  mk::member_values(&Params::precision)
                          * (mk::member_values(&Params::gangSize) = { 1, 2, 4 })
                          * (mk::member_values(&Params::numThreadsX, &Params::numThreadsY) = { { 16, 16 }, { 32, 32 } });
        (void) a3;
    }
    
    SECTION("expand")
    {
        auto p1 = Precision::double_;
        auto p1V = mk::expand2(p1);
        mk::visit(
            [p1](auto p1CV)
            {
                constexpr Precision p1C = p1CV();
                CHECK(p1C == p1);
            },
            p1V);

        auto p2 = Precision::double_;
        auto p2VO = mk::try_expand2(p2,
            []
            {
                return mk::values(Precision::single, Precision::double_);
            });
        CHECK(p2VO.has_value());
        mk::visit(
            [p2](auto p2CV)
            {
                constexpr Precision p2C = p2CV();
                CHECK(p2C == p2);
            },
            *p2VO);

        auto p3 = Precision::double_;
        auto p3VO = mk::try_expand2(p3,
            []
            {
                return mk::values(Precision::single);
            });
        CHECK_FALSE(p3VO.has_value());

        auto s1 = Params{ Precision::double_, 2, 32, 32 };
        auto s1VO = mk::try_expand2(s1,
            []
            {
                return mk::values(
                    Params{ Precision::single, 4, 32, 32 },
                    Params{ Precision::double_, 2, 32, 32 }
                );
            });
        CHECK(s1VO.has_value());
        mk::visit(
            [s1](auto s1CV)
            {
                constexpr Params s1C = s1CV();
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
        mk::visit(
            [s2](auto s2CV)
            {
                constexpr Params s2C = s2CV();
                CHECK(s2C == s2);
            },
            *s2VO);

        auto e1 = ExhaustibleParams{ Precision::single, false };
        auto e1V = mk::expand2(e1,
            []
            {
                return mk::member_values(&ExhaustibleParams::precision);
            });
        mk::visit(
            [e1](auto e1CV)
            {
                constexpr ExhaustibleParams e1C = e1CV();
                CHECK(e1 == e1C);
            },
            e1V);

        auto e2 = ExhaustibleParams{ Precision::single, true };
        auto e2V = mk::expand2(e2,
            []
            {
                return mk::member_values(&ExhaustibleParams::precision)
                     * mk::member_values(&ExhaustibleParams::transmogrify);
            });
        mk::visit(
            [e2](auto e2CV)
            {
                constexpr ExhaustibleParams e2C = e2CV();
                CHECK(e2 == e2C);
            },
            e2V);

        auto te1 = FloatType{ mk::type_v<double> };
        auto te1VO = mk::try_expand2(te1,
            []
            {
                return mk::values(
                    mk::type_v<float>,
                    mk::type_v<double>
                );
            });
        CHECK(te1VO.has_value());
        auto te1V = *te1VO;

        auto te1V2 = mk::expand2(te1);
        mk::visit(
            [](auto te1VCV, auto te1V2CV)
            {
                CHECK(te1VCV() == te1V2CV());
            },
            te1V, te1V2);

        auto v1 = FloatTypeVariant{ mk::type_v<double> };
        auto v1VO = mk::try_expand2(v1,
            []
            {
                return mk::values(
                    FloatTypeVariant{ mk::type_v<float> },
                    FloatTypeVariant{ mk::type_v<double> }
                );
            });
        CHECK(v1VO.has_value());
        auto v1V = *v1VO;
        mk::visit(
            [](auto v1VCV)
            {
#ifndef _MSC_VER // bug: https://developercommunity.visualstudio.com/content/problem/483944/vc2017vc2019-stdvariant-stdvisit-constexpr-error-c.html
                constexpr auto v1VC = v1VCV();
                constexpr auto v1C = std::get<v1VC.index()>(v1VC);
                CHECK(v1C == mk::type_v<double>);
                auto v1CV = mk::constexpr_extend(
                    [](auto _v1CV)
                    {
                        constexpr auto v1 = _v1CV();
                        return std::get<v1.index()>(v1);
                    },
                    v1VCV);
                constexpr auto v1C2 = v1CV();
                CHECK(v1C2 == mk::type_v<double>);
#else // _MSC_VER
                (void) v1VCV;
#endif // _MSC_VER
            },
            v1V);

        bool b1 = false;
        mk::visit(
            [b1](auto b1C)
            {
                CHECK(b1C() == b1);
            },
            b1);
    }
}
