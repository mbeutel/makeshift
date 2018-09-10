
#include <limits>
#include <sstream>
#include <stdexcept> // for invalid_argument, runtime_error

#include <makeshift/constrained.hpp>


namespace makeshift
{

namespace detail
{


template <typename IntT>
    static void raise_constrained_integer_error_impl(IntT value, const IntT permittedValues[], std::size_t numPermittedValues, std::string_view caption, bool isContractual)
{
    std::ostringstream sstr;
    if (!caption.empty())
        sstr << "Invalid " << caption << " value ";
    else
        sstr << "Invalid constrained integer value ";
    sstr << value
         << "; admissible values: ";
    bool first = true;
    for (std::size_t i = 0; i < numPermittedValues; ++i)
    {
        if (first)
            first = false;
        else
            sstr << ", ";
        sstr << permittedValues[i];
    }
    if (isContractual)
        throw std::invalid_argument(sstr.str());
    else
        throw std::runtime_error(sstr.str());
}

template <typename IntT>
    static void raise_constrained_integer_error_impl(IntT value, RangeType rangeType, IntT first, IntT last, std::string_view caption, bool isContractual)
{
    std::ostringstream sstr;
    if (!caption.empty())
        sstr << "Invalid " << caption << " value ";
    else
        sstr << "Invalid constrained integer value ";

    sstr << value << "; ";

    if (rangeType == RangeType::lowerHalf)
        sstr << "values must be < " << last;
    else if (rangeType == RangeType::upperHalf)
        sstr << "values must be >= " << first;
    else
        sstr << "values must be in range [" << first << ", " << last << ")";
    if (isContractual)
        throw std::invalid_argument(sstr.str());
    else
        throw std::runtime_error(sstr.str());
}

[[noreturn]] void raise_constrained_integer_error(std::int64_t value, RangeType rangeType, std::int64_t first, std::int64_t last, std::string_view caption, bool isContractual)
{
    raise_constrained_integer_error_impl(value, rangeType, first, last, caption, isContractual);
}

[[noreturn]] void raise_constrained_integer_error(std::uint64_t value, RangeType rangeType, std::uint64_t first, std::uint64_t last, std::string_view caption, bool isContractual)
{
    raise_constrained_integer_error_impl(value, rangeType, first, last, caption, isContractual);
}

[[noreturn]] void raise_constrained_integer_error(std::int64_t value, const std::int64_t permittedValues[], std::size_t numPermittedValues, std::string_view caption, bool isContractual)
{
    raise_constrained_integer_error_impl(value, permittedValues, numPermittedValues, caption, isContractual);
}

[[noreturn]] void raise_constrained_integer_error(std::uint64_t value, const std::uint64_t permittedValues[], std::size_t numPermittedValues, std::string_view caption, bool isContractual)
{
    raise_constrained_integer_error_impl(value, permittedValues, numPermittedValues, caption, isContractual);
}


} // namespace detail

} // namespace makeshift
