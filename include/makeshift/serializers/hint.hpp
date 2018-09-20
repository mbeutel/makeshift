
#ifndef INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
#define INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_


#include <string>

#include <makeshift/type_traits.hpp>           // for tag<>
#include <makeshift/serialize.hpp>             // for define_serializer<>, metadata_tag_of_serializer<>
#include <makeshift/hint.hpp>                  // for get_hint()

#include <makeshift/detail/serialize_enum.hpp> // for serialization_data<>


namespace makeshift
{

namespace detail
{


    // defined in serializers_hint-reflect.hpp
template <typename T, typename SerializerT>
    std::string get_compound_hint(SerializerT&& serializer, tag<T> = { });


} // namespace detail



inline namespace serialize
{


    // TODO: document


struct hint_options
{
    std::string_view option_separator = "|";
    std::string_view flags_separator = ",";

    constexpr hint_options(void) noexcept = default;
};


template <typename BaseT = void>
    struct hint_serializer : define_serializer<hint_serializer, BaseT, struct hint_options>
{
    using base = define_serializer<makeshift::hint_serializer, BaseT, hint_options>;
    using base::base;
    
    template <typename T, typename SerializerT>
        friend std::string get_hint_impl(tag<T>, const hint_serializer& hintSerializer, SerializerT&& serializer)
    {
        (void) serializer;
        (void) hintSerializer;
        using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
        if constexpr (std::is_enum<T>::value && have_metadata_v<T, MetadataTag>)
            return get_hint(makeshift::detail::serialization_data<T, MetadataTag>, hintSerializer.data);
        else if constexpr (std::is_same<T, bool>::value)
            return std::string("false") + hintSerializer.data.option_separator + std::string("true");
        else if constexpr (is_constrained_integer_v<T>)
            return T::verifier::get_hint(hintSerializer.data, typename T::constraint{ });
        else if constexpr (makeshift::detail::is_any_compound<T, MetadataTag>)
            return makeshift::detail::get_compound_hint<T>(serializer);
        else
            return { };
    }
};
hint_serializer(void) -> hint_serializer<>;
hint_serializer(const hint_options&) -> hint_serializer<>;
hint_serializer(hint_options&&) -> hint_serializer<>;


template <typename T>
    std::string get_hint(tag<T> = { })
{
    return get_hint(hint_serializer(), tag_v<T>);
}


} // inline namespace serialize

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_REFLECT_HPP_
 #include <makeshift/detail/serializers_hint-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_


#endif // INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
