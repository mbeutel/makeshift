
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_


#include <makeshift/utility2.hpp>


namespace makeshift
{

inline namespace metadata
{


    //ᅟ
    // Use `metadata_of<T, MetadataTagT>` to look up metadata for a type.
    //
template <typename T, typename MetadataTagT> static constexpr auto metadata_of = reflect(tag<T>{ }, MetadataTagT{ });

    //ᅟ
    // Use `metadata_of<T, MetadataTagT>` to look up metadata for a type.
    //
template <typename T, typename MetadataTagT> using metadata_of_t = decltype(reflect(tag<T>{ }, MetadataTagT{ }));


    //ᅟ
    // Determines whether there is metadata for the given type and the given tag.
    //
template <typename T, typename MetadataTagT> struct have_metadata : can_instantiate<metadata_of_t, T, MetadataTagT> { };

    //ᅟ
    // Determines whether there is metadata for the given type and the given tag.
    //
template <typename T, typename MetadataTagT> constexpr bool have_metadata_v = have_metadata<T, MetadataTagT>::value;


    //ᅟ
    // Determines qualities of a type.
    //
struct type_flag : define_flags<type_flag>
{
        //ᅟ
        // Indicates that the associated type is a value type (either a scalar type or a user-defined wrapper).
        //
    static constexpr auto value = flag(1);

        //ᅟ
        // Indicates that the associated type is a compound type, i.e. it has the semantics of a named tuple with regard to identity and comparability.
        // This type flag does not necessarily require aggregate-ness as defined in the C++ standard (`std::is_aggregate<>`), which imposes
        // unnecessary limitations (e.g. it may be reasonable for a compound type to have a user-defined constructor).
        //
    static constexpr auto compound = flag(2);

        //ᅟ
        // Indicates that the associated type is a composite type which itself forms a value, e.g. a geometrical point defined as `struct Point { int x, y; };`.
        //
    static constexpr auto compound_value = compound | value;
};
using type_flags = type_flag::flags;


} // inline namespace metadata


namespace detail
{


template <typename T>
    using default_type_flags
        = std::conditional_t<std::disjunction<std::is_scalar<T>, is_constrained_integer<T>>::value, constant<type_flag::value>,
          std::conditional_t<std::is_aggregate<T>::value, constant<type_flag::compound>,
          constant<type_flags::none>>>;
template <typename T> constexpr type_flags default_type_flags_v = default_type_flags<T>::value;
template <typename T, typename MetadataTagT> struct metadata_flags : metadata_of_t<T, MetadataTagT>::flags_type { };
template <typename T, typename MetadataTagT>
    using lookup_type_flags
        = typename std::conditional_t<have_metadata_v<T, MetadataTagT>, metadata_flags<T, MetadataTagT>, default_type_flags<T>>::type;
template <typename T, typename MetadataTagT>
    constexpr type_flags lookup_type_flags_v = lookup_type_flags<T, MetadataTagT>::value;


template <typename ChainableT> struct metadata_tag_r { using type = typename ChainableT::metadata_tag; };
template <typename ChainableT> using metadata_tag_rt = typename ChainableT::metadata_tag;


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // Determines qualities of a type.
    //
template <typename T, typename MetadataTagT, typename = void> constexpr type_flags type_flags_of_v = makeshift::detail::lookup_type_flags_v<T, MetadataTagT>;

    //ᅟ
    // Determines qualities of a type.
    //
template <typename T, typename MetadataTagT> using type_flags_of = constant<type_flags_of_v<T, MetadataTagT>>;


    //ᅟ
    // Determines whether the given type, according to the given metadata tag, is a compound type.
    //
template <typename T, typename MetadataTagT> struct is_compound : std::integral_constant<bool, (type_flags_of_v<T, MetadataTagT> & type_flag::compound) != type_flags::none>{ };

    //ᅟ
    // Determines whether the given type, according to the given metadata tag, is a compound type.
    //
template <typename T, typename MetadataTagT> constexpr bool is_compound_v = is_compound<T, MetadataTagT>::value;


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
