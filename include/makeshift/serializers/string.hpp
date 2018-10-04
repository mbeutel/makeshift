
#ifndef INCLUDED_MAKESHIFT_SERIALIZERS_STRING_HPP_
#define INCLUDED_MAKESHIFT_SERIALIZERS_STRING_HPP_


#include <string>      // for string, to_string<>
#include <string_view>
#include <type_traits> // for decay<>, is_enum<>, enable_if<>
#include <utility>     // for move()
#include <algorithm>   // for equal()

#include <makeshift/type_traits.hpp> // for tag<>, is_serializer<>
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>   // for define_serializer<>, metadata_tag_of_serializer<>

#include <makeshift/detail/export.hpp>         // for MAKESHIFT_DLLFUNC
#include <makeshift/detail/serialize_enum.hpp>


namespace makeshift
{

namespace detail
{


inline std::string scalar_to_string(std::string s) { return std::move(s); }
inline std::string scalar_to_string(int val) { return std::to_string(val); }
inline std::string scalar_to_string(unsigned val) { return std::to_string(val); }
inline std::string scalar_to_string(long val) { return std::to_string(val); }
inline std::string scalar_to_string(unsigned long val) { return std::to_string(val); }
inline std::string scalar_to_string(long long val) { return std::to_string(val); }
inline std::string scalar_to_string(unsigned long long val) { return std::to_string(val); }
inline std::string scalar_to_string(float val) { return std::to_string(val); }
inline std::string scalar_to_string(double val) { return std::to_string(val); }
inline std::string scalar_to_string(long double val) { return std::to_string(val); }

inline std::string scalar_from_string(tag<std::string>, const std::string& s) { return s; }
inline int scalar_from_string(tag<int>, const std::string& string) { return std::stoi(string); }
MAKESHIFT_DLLFUNC unsigned scalar_from_string(tag<unsigned>, const std::string& string);
inline long scalar_from_string(tag<long>, const std::string& string) { return std::stol(string); }
inline unsigned long scalar_from_string(tag<unsigned long>, const std::string& string) { return std::stoul(string); }
inline long long scalar_from_string(tag<long long>, const std::string& string) { return std::stoll(string); }
inline unsigned long long scalar_from_string(tag<unsigned long long>, const std::string& string) { return std::stoull(string); }
inline float scalar_from_string(tag<float>, const std::string& string) { return std::stof(string); }
inline double scalar_from_string(tag<double>, const std::string& string) { return std::stod(string); }
inline long double scalar_from_string(tag<long double>, const std::string& string) { return std::stold(string); }


template <typename MetadataTagT, typename T> struct have_string_conversion : is_enum_with_metadata<MetadataTagT, T> { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, std::string> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, int> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, unsigned> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, unsigned long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, long long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, unsigned long long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, float> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, double> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, long double> : std::true_type { };
template <typename MetadataTagT, typename T> constexpr bool have_string_conversion_v = have_string_conversion<MetadataTagT, T>::value;


template <typename ConstrainedIntT, typename SerializerT>
    std::string constrained_integer_to_string(ConstrainedIntT value, SerializerT& serializer)
{
    return to_string(value.value(), serializer);
}

template <typename ConstrainedIntT, typename SerializerT>
    ConstrainedIntT constrained_integer_from_string(std::string_view string, SerializerT& serializer)
{
    using Int = typename ConstrainedIntT::value_type;
    Int result = from_string(string, serializer);
    return ConstrainedIntT::check(result);
}


} // namespace detail


inline namespace serialize
{


    // To customize string serialization for arbitrary types, define your own serializer type along with `to_string_impl()`, `from_string_impl()`
    // overloads discoverable by ADL.
    //
    // To override parts of the behavior of an existing serializer, define a new serializer that handles only the types you want to customize, and
    // chain it with the existing serializer using `chain()`.



    //ᅟ
    // Runtime arguments for string serializers.
    //
struct string_serializer_options
{
    enum_serialization_options enum_options;

    constexpr string_serializer_options(void) noexcept = default;
    constexpr string_serializer_options(enum_serialization_options _enum_options) noexcept : enum_options(_enum_options) { }
};


    //ᅟ
    // String serializer for common scalar types (built-in types, enums, constrained integers, and `std::string`).
    //
template <typename BaseT = void>
    struct string_serializer : define_serializer<string_serializer, BaseT, string_serializer_options>
{
    using base = define_serializer<makeshift::string_serializer, BaseT, string_serializer_options>; // TODO: report VC++ bug
    using base::base;
    
    template <typename T, typename SerializerT/*,
              typename = std::enable_if_t<makeshift::detail::have_string_conversion_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, std::decay_t<T>>>*/>
        friend std::enable_if_t<makeshift::detail::have_string_conversion_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, std::decay_t<T>>, std::string>
        to_string_impl(const T& value, const string_serializer& stringSerializer, SerializerT& serializer)
    {
        (void) stringSerializer;
        (void) serializer;
        using D = std::decay_t<T>;
        if constexpr (std::is_enum<D>::value && have_metadata_v<D, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>)
            return std::string(to_string(value, makeshift::detail::serialization_data<D, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>, data(stringSerializer).enum_options));
        else if constexpr (is_constrained_integer_v<D>)
            return makeshift::detail::constrained_integer_to_string(value, serializer);
        else
            return makeshift::detail::scalar_to_string(value);
    }
    template <typename T, typename SerializerT/*,
              typename = std::enable_if_t<makeshift::detail::have_string_conversion_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, T>>*/>
        friend std::enable_if_t<makeshift::detail::have_string_conversion_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, T>, T>
        from_string_impl(tag<T>, std::string_view string, const string_serializer& stringSerializer, SerializerT& serializer)
    {
        (void) stringSerializer;
        (void) serializer;
        if constexpr (std::is_enum<T>::value && have_metadata_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>)
            return from_string(tag_v<T>, string, makeshift::detail::serialization_data<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>, data(stringSerializer).enum_options);
        else if constexpr (is_constrained_integer_v<T>)
            return makeshift::detail::constrained_integer_from_string<T>(string, serializer);
        else
            return makeshift::detail::scalar_from_string(tag_v<T>, std::string(string));
    }
};
string_serializer(void) -> string_serializer<>;
string_serializer(const string_serializer_options&) -> string_serializer<>;
string_serializer(string_serializer_options&&) -> string_serializer<>;


} // inline namespace serialize


namespace detail
{


static constexpr string_serializer<> default_string_serializer{ };


} // namespace detail


inline namespace serialize
{


    //ᅟ
    // Serializes the given value as string using a default-initialized `string_serializer<>`.
    //ᅟ
    //ᅟ    std::string s = to_string(42); // returns "42"s
    //
template <typename T>
    std::string to_string(const T& value)
{
    return to_string(value, makeshift::detail::default_string_serializer);
}


    //ᅟ
    // Deserializes the given value from a string using a default-initialized `string_serializer<>`.
    //ᅟ
    //ᅟ    int i = from_string<int>("42"); // returns 42
    //
template <typename T>
    T from_string(std::string_view string, tag<T> = { })
{
    return from_string<T>(string, makeshift::detail::default_string_serializer);
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_SERIALIZERS_STRING_HPP_
