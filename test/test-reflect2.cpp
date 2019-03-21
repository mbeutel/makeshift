
#include <makeshift/metadata2.hpp>
#include <makeshift/reflect2.hpp>
#include <makeshift/utility2.hpp>  // for define_flags<>
#include <makeshift/compound.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


enum class BoundaryCondition
{
    periodic,  // axis wraps around
    dirichlet  // constant boundary value (currently read from halo)
};
constexpr inline auto reflect(mk::type<BoundaryCondition>)
{
    return mk::define_metadata(
        mk::named_values<BoundaryCondition> = {
            { BoundaryCondition::periodic, "periodic" },
            { BoundaryCondition::dirichlet, "dirichlet" }
        }
    );
}

struct PartitionFlag : mk::define_flags<PartitionFlag>
{
    static constexpr flag enclosedBoundary { 0x010u };
    static constexpr flag nodeCut          { 0x020u };
    static constexpr flag threadCut        { 0x040u };
};
using PartitionFlags = PartitionFlag::flags;
constexpr inline auto reflect(mk::type<PartitionFlag>)
{
    return mk::define_metadata(
        mk::values<PartitionFlags> = {
            PartitionFlag::enclosedBoundary,
            PartitionFlag::nodeCut,
            PartitionFlag::threadCut
        }
    );
}

TEST_CASE("reflect2", "[flags]")
{
    (void) (mk::values<BoundaryCondition> = { });
}
