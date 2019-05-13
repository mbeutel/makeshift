
#include <iostream>
#include <string_view>

#include <makeshift/variant2.hpp>
#include <makeshift/metadata2.hpp>
#include <makeshift/utility2.hpp>


namespace mk = makeshift;

using namespace std::literals;


using FloatType = mk::type_enum<float, double>;


enum class Operation
{
    copy,
    axpy
};
constexpr inline auto reflect(mk::type<Operation>)
{
    return mk::define_metadata(
        mk::values<Operation> = { Operation::copy, Operation::axpy }
    );
}

template <typename FloatT>
    void doWork(mk::type<FloatT>)
{
}

void demo(void)
{
    bool log = false;
    int logLevel = 1;
    Operation operation = Operation::copy;
    FloatType floatType = mk::type_c<float>;

    auto logLevelCV = mk::expand2_or_throw(logLevel, [] { return mk::values<int> = { 1, 2, 3 }; });
    auto floatTypeCV = mk::expand2(floatType);

    mk::visit(
        [](auto logC, auto logLevelC, auto operationC, auto floatTypeC)
        {
            constexpr bool log = logC();
            constexpr int logLevel = logLevelC();
            constexpr Operation operation = operationC();
            constexpr auto floatType = floatTypeC(); // either type<float> or type<double>

            for (int i = 0; i < 10; ++i)
            {
                if constexpr (log)
                {
                    if constexpr (logLevel >= 2)
                        std::clog << "inside loop: ";
                    std::clog << "i=" << i << '\n';
                }

                //using Float = decltype(floatType)::type;

                doWork(floatType);
            }
        },
        log, logLevelCV, operation, floatTypeCV);
}
