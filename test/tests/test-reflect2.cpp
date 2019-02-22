
#include <makeshift/reflect2.hpp>
#include <makeshift/utility.hpp>  // for define_flags<>

#include <catch.hpp>


namespace mk = makeshift;


enum class BoundaryCondition
{
    periodic,  // axis wraps around
    dirichlet  // constant boundary value (currently read from halo)
};
constexpr auto reflect(mk::type<BoundaryCondition>)
{
    return mk::reflect_enum_values(
        mk::with_name(BoundaryCondition::periodic, "periodic"),
        mk::with_name(BoundaryCondition::dirichlet, "dirichlet")
    );
}

struct PartitionFlag : mk::define_flags<PartitionFlag>
{
    static constexpr flag enclosedBoundary { 0x010u };
    static constexpr flag nodeCut          { 0x020u };
    static constexpr flag threadCut        { 0x040u };
};
using PartitionFlags = PartitionFlag::flags;
constexpr auto reflect(mk::type<PartitionFlag>)
{
    return mk::reflect_enum_values(
        PartitionFlag::enclosedBoundary,
        PartitionFlag::nodeCut,
        PartitionFlag::threadCut
    );
}

struct PartitionAxis // : regular_compound<regular_category::equivalence | regular_category::hash | regular_category::ordering>
{
    mk::index_t first;
    mk::index_t last;
    PartitionFlags flags;
};
constexpr auto reflect(mk::type<PartitionAxis>)
{
    return mk::reflect_compound_members(
        mk::with_name(&PartitionAxis::first, "first"),
        mk::with_name(&PartitionAxis::last, "last"),
        mk::with_name(&PartitionAxis::flags, "flags")
    );
}

TEST_CASE("reflect2", "[flags]")
{
    
}
