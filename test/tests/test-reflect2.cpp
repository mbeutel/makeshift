
#include <makeshift/reflect2.hpp>
#include <makeshift/utility.hpp>  // for define_flags<>

#include <catch.hpp>


namespace mk = makeshift;


enum class BoundaryCondition
{
    periodic,  // axis wraps around
    dirichlet  // constant boundary value (currently read from halo)
};
constexpr auto reflect(mk::tag<BoundaryCondition>)
{
    return mk::enumeration(
        mk::with_name(BoundaryCondition::periodic, "periodic"),
        mk::with_name(BoundaryCondition::dirichlet, "dirichlet")
    );
}

struct PartitionFlag : mk::define_flags<PartitionFlag>
{
    static constexpr flag enclosedBoundary   { 0x010u }; // also set by ThreadParallelogramProcessor<>
    static constexpr flag nodeCut            { 0x020u }; // indicates that the domain was cut into nodes along this axis
    static constexpr flag threadCut          { 0x040u }; // indicates that the node partition was cut into threads along this axis
};
using PartitionFlags = PartitionFlag::flags;
constexpr auto reflect(mk::tag<PartitionFlag>)
{
    return mk::flags_enumeration(
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
constexpr auto reflect(mk::tag<PartitionAxis>)
{
    return mk::compound(
        mk::with_name(&PartitionAxis::first, "first"),
        mk::with_name(&PartitionAxis::last, "last"),
        mk::with_name(&PartitionAxis::flags, "flags")
    );
}

TEST_CASE("reflect2", "[flags]")
{
    
}
