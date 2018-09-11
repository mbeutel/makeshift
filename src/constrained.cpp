
#include <cassert>
#include <string>
#include <sstream>
#include <stdexcept> // for invalid_argument, runtime_error

#include <gsl/gsl_assert>

#include <makeshift/constrained.hpp>
#include <makeshift/serializers/hint.hpp> // for hint_serializer_args


namespace makeshift
{

namespace detail
{


template <typename IntT>
    static void constrained_integer_hint_impl(std::ostream& stream, ConstraintType constraintType, const IntT values[], std::size_t numValues, const hint_serializer_args& args)
{
    switch (constraintType)
    {
    case ConstraintType::sequence:
        {
            bool first = true;
            for (std::size_t i = 0; i < numValues; ++i)
            {
                if (first)
                    first = false;
                else
                    stream << args.option_separator;
                stream << values[i];
            }
        }
        break;
    case ConstraintType::range:
        Expects(numValues == 2);
        stream << "[" << values[0] << "," << values[1] << ")";
        break;
    case ConstraintType::inclusiveRange:
        Expects(numValues == 2);
        stream << values[0] << ".." << values[1];
        break;
    case ConstraintType::upperHalfRange:
        Expects(numValues == 1);
        stream << values[0] << "..";
        break;
    case ConstraintType::lowerHalfRange:
        Expects(numValues == 1);
        stream << "[," << values[0] << ")";
        break;
    case ConstraintType::lowerHalfInclusiveRange:
        Expects(numValues == 1);
        stream << ".." << values[0];
        break;
    default:
        throw std::invalid_argument("invalid constraint type");
    }
}
template <typename IntT>
    static std::string constrained_integer_hint_impl(ConstraintType constraintType, const IntT values[], std::size_t numValues, const hint_serializer_args& args)
{
    std::ostringstream sstr;
    constrained_integer_hint_impl(sstr, constraintType, values, numValues, args);
    return sstr.str();
}

std::string constrained_integer_hint(ConstraintType constraintType, const std::int64_t values[], std::size_t numValues, const hint_serializer_args& args)
{
    return constrained_integer_hint_impl(constraintType, values, numValues, args);
}
std::string constrained_integer_hint(ConstraintType constraintType, const std::uint64_t values[], std::size_t numValues, const hint_serializer_args& args)
{
    return constrained_integer_hint_impl(constraintType, values, numValues, args);
}

static void constrained_integer_error_msg(std::ostream& stream, const constrained_integer_metadata& metadata)
{
    if (!metadata.caption.empty())
        stream << " is not a valid " << metadata.caption;
    else if (!metadata.typeName.empty())
        stream << " is not a valid value of type '" << metadata.typeName << "'";
    else
        stream << " is not a valid constrained integer value";
}
template <typename IntT>
    [[noreturn]] static void raise_constrained_integer_error_impl(IntT value, ConstraintType constraintType, const IntT values[], std::size_t numValues, const constrained_integer_metadata& metadata, bool isContractual)
{
    std::ostringstream sstr;
    sstr << value;
    constrained_integer_error_msg(sstr, metadata);
    hint_serializer_args hint_args;
    hint_args.option_separator = ", ";
    switch (constraintType)
    {
    case ConstraintType::sequence:
        sstr << "; admissible values: ";
        constrained_integer_hint_impl(sstr, constraintType, values, numValues, hint_args);
        break;
    case ConstraintType::range:
    case ConstraintType::inclusiveRange:
        sstr << "; value must be in range ";
        constrained_integer_hint_impl(sstr, constraintType, values, numValues, hint_args);
        break;
    case ConstraintType::upperHalfRange:
        Expects(numValues == 1);
        sstr << "; value must be >= " << values[0];
        break;
    case ConstraintType::lowerHalfRange:
        Expects(numValues == 1);
        sstr << "; value must be < " << values[0];
        break;
    case ConstraintType::lowerHalfInclusiveRange:
        Expects(numValues == 1);
        sstr << "; value must be <= " << values[0];
        break;
    default:
        throw std::invalid_argument("invalid constraint type");
    }
    if (isContractual)
        throw std::invalid_argument(sstr.str());
    else
        throw std::runtime_error(sstr.str());
}

[[noreturn]] void raise_constrained_integer_error(std::int64_t value, ConstraintType constraintType, const std::int64_t values[], std::size_t numValues, const constrained_integer_metadata& metadata, bool isContractual)
{
    raise_constrained_integer_error_impl(value, constraintType, values, numValues, metadata, isContractual);
}
[[noreturn]] void raise_constrained_integer_error(std::uint64_t value, ConstraintType constraintType, const std::uint64_t values[], std::size_t numValues, const constrained_integer_metadata& metadata, bool isContractual)
{
    raise_constrained_integer_error_impl(value, constraintType, values, numValues, metadata, isContractual);
}


} // namespace detail

} // namespace makeshift
