
#ifndef INCLUDED_MAKESHIFT_CONSTRAINED_HPP_
#define INCLUDED_MAKESHIFT_CONSTRAINED_HPP_


#include <cstddef>     // for size_t
#include <cstdint>     // for [u]int64_t
#include <utility>     // for integer_sequence<>
#include <string>
#include <string_view>
#include <type_traits> // for is_signed<>, conditional<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/metadata.hpp>
#include <makeshift/tuple.hpp>       // for get_or_default<>(), get_or_none<>()
#include <makeshift/type_traits.hpp> // for sequence<>, tag<>

#include <makeshift/detail/cfg.hpp>             // for MAKESHIFT_DLLFUNC
#include <makeshift/detail/workaround.hpp>      // for cor()
#include <makeshift/detail/functional_hash.hpp>


namespace makeshift
{


inline namespace serialize
{


struct hint_options_t; // defined in makeshift/serializers/hint.hpp


} // inline namespace serialize


namespace detail
{


enum class ConstraintType
{
    sequence,
    range,
    inclusiveRange,
    upperHalfRange,
    lowerHalfRange,
    lowerHalfInclusiveRange
};

struct constrained_integer_metadata
{
    std::string_view typeName;
    std::string_view caption;
};

MAKESHIFT_DLLFUNC std::string constrained_integer_hint(ConstraintType constraintType, const std::int64_t values[], std::size_t numValues, const hint_options_t& options);
MAKESHIFT_DLLFUNC std::string constrained_integer_hint(ConstraintType constraintType, const std::uint64_t values[], std::size_t numValues, const hint_options_t& options);

[[noreturn]] MAKESHIFT_DLLFUNC void raise_constrained_integer_error(std::int64_t value, ConstraintType constraintType, const std::int64_t values[], std::size_t numValues, const constrained_integer_metadata& metadata);
[[noreturn]] MAKESHIFT_DLLFUNC void raise_constrained_integer_error(std::uint64_t value, ConstraintType constraintType, const std::uint64_t values[], std::size_t numValues, const constrained_integer_metadata& metadata);


template <typename T, T... Vs>
    constexpr bool is_constrained_integer_valid(T value, std::integer_sequence<T, Vs...>) noexcept
{
    return cor((value == Vs)...);
}
template <typename T, T... Vs>
    constexpr bool is_constrained_integer_valid(T value, sequence<T, Vs...>) noexcept
{
    return cor((value == Vs)...);
}
template <typename T, T First, T Last>
    constexpr bool is_constrained_integer_valid(T value, integer_range<T, First, Last>) noexcept
{
    return value >= First && value < Last;
}
template <typename T, T First, T Last>
    constexpr bool is_constrained_integer_valid(T value, integer_inclusive_range<T, First, Last>) noexcept
{
    return value >= First && value <= Last;
}
template <typename T, T First>
    constexpr bool is_constrained_integer_valid(T value, integer_upper_half_range<T, First>) noexcept
{
    return value >= First;
}
template <typename T, T Last>
    constexpr bool is_constrained_integer_valid(T value, integer_lower_half_range<T, Last>) noexcept
{
    return value < Last;
}
template <typename T, T Last>
    constexpr bool is_constrained_integer_valid(T value, integer_lower_half_inclusive_range<T, Last>) noexcept
{
    return value <= Last;
}

template <typename T, T... Vs>
    constexpr sequence<T, Vs...> get_valid_constrained_integer_values(std::integer_sequence<T, Vs...>) noexcept
{
    return { };
}
template <typename T, T... Vs>
    constexpr sequence<T, Vs...> get_valid_constrained_integer_values(sequence<T, Vs...>) noexcept
{
    return { };
}
template <typename T, T Base, long long... Offsets>
    constexpr sequence<T, (Base + T(Offsets))...> get_valid_constrained_integer_values_impl(std::integral_constant<T, Base>, std::integer_sequence<long long, Offsets...>) noexcept
{
    return { };
}
template <typename T, T First, T Last>
    constexpr auto get_valid_constrained_integer_values(integer_range<T, First, Last>) noexcept
{
    return get_valid_constrained_integer_values_impl(std::integral_constant<T, First>{ }, std::make_integer_sequence<long long, (long long)(Last - First)>);
}
template <typename T, T First, T Last>
    constexpr auto get_valid_constrained_integer_values(integer_inclusive_range<T, First, Last>) noexcept
{
    return get_valid_constrained_integer_values_impl(std::integral_constant<T, First>{ }, std::make_integer_sequence<long long, (long long)(Last - First + 1)>);
}


template <typename T> using common_int = std::conditional_t<std::is_signed<T>::value, std::int64_t, std::uint64_t>;

template <typename T, T... Vs>
    std::string get_constrained_integer_hint(sequence<T, Vs...>, const hint_options_t& options)
{
    common_int<T> permittedValues[] = { common_int<T>(Vs)... };
    return constrained_integer_hint(ConstraintType::sequence, permittedValues, sizeof...(Vs), options);
}
template <typename T, T... Vs>
    std::string get_constrained_integer_hint(std::integer_sequence<T, Vs...>, const hint_options_t& options)
{
    return get_constrained_integer_hint(sequence<T, Vs...>{ }, options);
}
template <typename T, T First, T Last>
    std::string get_constrained_integer_hint(integer_range<T, First, Last>, const hint_options_t& options)
{
    common_int<T> values[] = { common_int<T>(First), common_int<T>(Last) };
    return constrained_integer_hint(ConstraintType::range, values, 2, options);
}
template <typename T, T First, T Last>
    std::string get_constrained_integer_hint(integer_inclusive_range<T, First, Last>, const hint_options_t& options)
{
    common_int<T> values[] = { common_int<T>(First), common_int<T>(Last) };
    return constrained_integer_hint(ConstraintType::inclusiveRange, values, 2, options);
}
template <typename T, T First>
    std::string get_constrained_integer_hint(integer_upper_half_range<T, First>, const hint_options_t& options)
{
    common_int<T> values[] = { common_int<T>(First) };
    return constrained_integer_hint(ConstraintType::upperHalfRange, values, 1, options);
}
template <typename T, T Last>
    std::string get_constrained_integer_hint(integer_lower_half_range<T, Last>, const hint_options_t& options)
{
    common_int<T> values[] = { common_int<T>(Last) };
    return constrained_integer_hint(ConstraintType::lowerHalfRange, values, 1, options);
}
template <typename T, T Last>
    std::string get_constrained_integer_hint(integer_lower_half_inclusive_range<T, Last>, const hint_options_t& options)
{
    common_int<T> values[] = { common_int<T>(Last) };
    return constrained_integer_hint(ConstraintType::lowerHalfInclusiveRange, values, 1, options);
}


template <typename T>
    constexpr constrained_integer_metadata get_constrained_integer_metadata(tag<T> = { }) noexcept
{
    if constexpr (have_metadata_v<T, serialization_metadata_tag>)
    {
        auto theMetadata = metadata_of<T, serialization_metadata_tag>;
        return { get_or_default<std::string_view>(theMetadata.attributes), get_or_default<caption_t>(theMetadata.attributes).value };
    }
    else
        return { };
}

template <typename T, typename ConstrainedIntT, T... Vs>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, sequence<T, Vs...>)
{
    common_int<T> permittedValues[] = { common_int<T>(Vs)... };
    /*constexpr*/ constrained_integer_metadata metadata = get_constrained_integer_metadata<ConstrainedIntT>(); // TODO: currently not constexpr due to VC++ ICE
    raise_constrained_integer_error(common_int<T>(value), ConstraintType::sequence, permittedValues, sizeof...(Vs), metadata);
}
template <typename T, typename ConstrainedIntT, T... Vs>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, std::integer_sequence<T, Vs...>)
{
    return raise_constrained_integer_error(value, tag<ConstrainedIntT>{ }, sequence<T, Vs...>{ });
}
template <typename T, typename ConstrainedIntT, T First, T Last>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, integer_range<T, First, Last>)
{
    /*constexpr*/ constrained_integer_metadata metadata = get_constrained_integer_metadata<ConstrainedIntT>(); // TODO: currently not constexpr due to VC++ ICE
    common_int<T> values[] { common_int<T>(First), common_int<T>(Last) };
    raise_constrained_integer_error(common_int<T>(value), ConstraintType::range, values, 2, metadata);
}
template <typename T, typename ConstrainedIntT, T First, T Last>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, integer_inclusive_range<T, First, Last>)
{
    /*constexpr*/ constrained_integer_metadata metadata = get_constrained_integer_metadata<ConstrainedIntT>(); // TODO: currently not constexpr due to VC++ ICE
    common_int<T> values[] { common_int<T>(First), common_int<T>(Last) };
    raise_constrained_integer_error(common_int<T>(value), ConstraintType::inclusiveRange, values, 2, metadata);
}
template <typename T, typename ConstrainedIntT, T First>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, integer_upper_half_range<T, First>)
{
    /*constexpr*/ constrained_integer_metadata metadata = get_constrained_integer_metadata<ConstrainedIntT>(); // TODO: currently not constexpr due to VC++ ICE
    common_int<T> values[] { common_int<T>(First) };
    raise_constrained_integer_error(common_int<T>(value), ConstraintType::upperHalfRange, values, 1, metadata);
}
template <typename T, typename ConstrainedIntT, T Last>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, integer_lower_half_range<T, Last>)
{
    /*constexpr*/ constrained_integer_metadata metadata = get_constrained_integer_metadata<ConstrainedIntT>(); // TODO: currently not constexpr due to VC++ ICE
    common_int<T> values[] { common_int<T>(Last) };
    raise_constrained_integer_error(common_int<T>(value), ConstraintType::lowerHalfRange, values, 1, metadata);
}
template <typename T, typename ConstrainedIntT, T Last>
    void raise_constrained_integer_error(T value, tag<ConstrainedIntT>, integer_lower_half_inclusive_range<T, Last>)
{
    /*constexpr*/ constrained_integer_metadata metadata = get_constrained_integer_metadata<ConstrainedIntT>(); // TODO: currently not constexpr due to VC++ ICE
    common_int<T> values[] { common_int<T>(Last) };
    raise_constrained_integer_error(common_int<T>(value), ConstraintType::lowerHalfInclusiveRange, values, 1, metadata);
}


} // namespace detail


inline namespace types
{


    //ᅟ
    // Default constraint verifier for constrained integer types. Looks up the functions `ìs_constrained_integer_valid()`
    // and `raise_constrained_integer_error()` via ADL. Contains default implementations for constraints of type `sequence<>`,
    // `std::integer_sequence<>`, and `integer_[upper_half_|lower_half_][inclusive_]range<>`.
    //
struct default_integer_constraint_verifier
{
    template <typename T, typename ConstraintT>
        static constexpr bool is_valid(T value, ConstraintT = { }) noexcept
    {
        using makeshift::detail::is_constrained_integer_valid; // permit ADL, fall back to default implementations above

        return is_constrained_integer_valid(value, ConstraintT{ });
    }
    template <typename ConstraintT>
        static constexpr auto get_valid_values(ConstraintT = { }) noexcept
    {
        using makeshift::detail::get_valid_constrained_integer_values; // permit ADL, fall back to default implementations above

        return get_valid_constrained_integer_values(ConstraintT{ });
    }
    template <typename ConstraintT>
        static std::string get_hint(const hint_options_t& options, ConstraintT = { }) noexcept
    {
        using makeshift::detail::get_constrained_integer_hint; // permit ADL, fall back to default implementations above

        return get_constrained_integer_hint(ConstraintT{ }, options);
    }
    template <typename T, typename ConstrainedIntT>
        static void raise_error(T value, tag<ConstrainedIntT> = { })
    {
        using makeshift::detail::raise_constrained_integer_error; // permit ADL, fall back to default implementations above

        return raise_constrained_integer_error(value, tag_v<ConstrainedIntT>, typename ConstrainedIntT::constraint{ });
    }
};


template <typename T> using non_negative = integer_upper_half_range<T, 0>;
template <typename T> using non_positive = integer_lower_half_inclusive_range<T, 0>;
template <typename T> using negative = integer_lower_half_range<T, 0>;
template <typename T> using positive = integer_upper_half_range<T, 1>;


    //ᅟ
    // Inherit from `define_constrained_integer<>` to define an integer type with a constraint for admissible values:
    //ᅟ
    //ᅟ    struct BitDepth final : define_constrained_integer<BitDepth, sequence<int, 1, 8, 24>> { using base::base; };
    //ᅟ    auto bitDepth = 8_c;
    //
template <typename DerivedT, typename ConstraintT, typename VerifierT = default_integer_constraint_verifier>
    struct define_constrained_integer : makeshift::detail::constrained_integer_base
{
    using base = define_constrained_integer;

    using value_type = typename ConstraintT::value_type;
    using constraint = ConstraintT;
    using verifier = VerifierT;

private:
    value_type value_;

    struct unchecked_constructor_tag { };

    static constexpr bool is_valid_(value_type value) noexcept
    {
        return VerifierT::is_valid(value, ConstraintT{ });
    }
    static void raise_error_(value_type value)
    {
        VerifierT::raise_error(value, tag_v<DerivedT>);
    }

public:
        //ᅟ
        // Constructs a constrained integer type from an integer literal.
        //
    template <typename T, T V>
        constexpr define_constrained_integer(std::integral_constant<T, V>) noexcept
            : value_(V)
    {
        static_assert(std::is_final<DerivedT>::value, "constrained integers must be declared final");
        static_assert(is_valid_(V), "given value does not fulfill constraint");
    }

        //ᅟ
        // Constructs a constrained integer type from an integer. Raises `std::runtime_error` if the integer type is not an admissible value.
        //
    /*explicit constexpr define_constrained_integer(value_type _value)
        : value_(_value)
    {
        if (!is_valid_(_value))
            raise_error_(_value);
    }*/

    explicit constexpr define_constrained_integer(unchecked_constructor_tag, value_type _value) : value_(_value) { }

        //ᅟ
        // Determines whether the given value is an admissible value.
        //
    static constexpr bool is_valid(value_type value) noexcept { return is_valid_(value); }


        //ᅟ
        // Checks whether the given value is an admissible value, and raises `std::runtime_error` if not.
        //
    static constexpr DerivedT check(value_type value)
    {
        if (!is_valid_(value))
            raise_error_(value);
        return DerivedT(unchecked_constructor_tag{ }, value);
    }


        //ᅟ
        // Checks whether the given value is an admissible value, and raises `std::invalid_argument` if not.
        //
    static constexpr DerivedT assume(value_type value)
    {
        Expects(is_valid_(value));
        return DerivedT(unchecked_constructor_tag{ }, value);
    }

    constexpr operator value_type(void) const noexcept { return value_; }
    constexpr value_type value(void) const noexcept { return value_; }

    friend constexpr bool operator ==(DerivedT lhs, DerivedT rhs) noexcept { return lhs.value_ == rhs.value_; }
    friend constexpr bool operator !=(DerivedT lhs, DerivedT rhs) noexcept { return lhs.value_ != rhs.value_; }
    friend constexpr bool operator < (DerivedT lhs, DerivedT rhs) noexcept { return lhs.value_ <  rhs.value_; }
    friend constexpr bool operator > (DerivedT lhs, DerivedT rhs) noexcept { return lhs.value_ >  rhs.value_; }
    friend constexpr bool operator <=(DerivedT lhs, DerivedT rhs) noexcept { return lhs.value_ <= rhs.value_; }
    friend constexpr bool operator >=(DerivedT lhs, DerivedT rhs) noexcept { return lhs.value_ >= rhs.value_; }
};


    // Specialize `hash<>` for anything that is a constrained integer.
template <typename ConstrainedIntT>
    struct hash<ConstrainedIntT, std::enable_if_t<is_constrained_integer_v<ConstrainedIntT>>>
{
    using argument_type = ConstrainedIntT;
    using result_type = std::size_t;
    constexpr result_type operator()(const argument_type& arg) const noexcept
    {
        return std::hash<typename argument_type::value_type>{ }(arg.value());
    }
};


    //ᅟ
    // Refers to an ad-hoc integer type with a constraint for admissible values.
    //
template <typename ConstraintT, typename VerifierT = default_integer_constraint_verifier>
    struct constrained_integer : define_constrained_integer<constrained_integer<ConstraintT, VerifierT>, ConstraintT, VerifierT>
{
    using base = define_constrained_integer<constrained_integer<ConstraintT, VerifierT>, ConstraintT, VerifierT>;
    using base::base;
};


    //ᅟ
    // Refers to an ad-hoc non-negative integer type.
    //
template <typename T> using non_negative_integer = constrained_integer<non_negative<T>>;

    //ᅟ
    // Refers to an ad-hoc non-positive integer type.
    //
template <typename T> using non_positive_integer = constrained_integer<non_positive<T>>;

    //ᅟ
    // Refers to an ad-hoc negative integer type.
    //
template <typename T> using negative_integer = constrained_integer<negative<T>>;

    //ᅟ
    // Refers to an ad-hoc positive integer type.
    //
template <typename T> using positive_integer = constrained_integer<positive<T>>;


    // Specialize `type_category_of<>` for anything that is a constrained integer.
template <typename ConstrainedIntT, typename MetadataTagT> constexpr type_category type_category_of<ConstrainedIntT, MetadataTagT, std::enable_if_t<is_constrained_integer_v<ConstrainedIntT>>> = type_category::value;


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `std::hash<>` for constrained_integer<>.
template <typename ConstraintT, typename VerifierT>
    struct hash<makeshift::constrained_integer<ConstraintT, VerifierT>>
{
    using argument_type = makeshift::constrained_integer<ConstraintT, VerifierT>;
    constexpr std::size_t operator()(const argument_type& arg) const noexcept
    {
        return hash<typename argument_type::value_type>{ }(arg.value());
    }
};


} // namespace std


#endif // INCLUDED_MAKESHIFT_CONSTRAINED_HPP_
