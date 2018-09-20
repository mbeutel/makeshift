
#ifndef INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
#define INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_


#include <string>
#include <type_traits> // for enable_if<>

#include <makeshift/type_traits.hpp>           // for tag<>, is_serializer<>
#include <makeshift/serialize.hpp>             // for define_serializer<>, metadata_tag_of_serializer<>

#include <makeshift/detail/serialize_enum.hpp> // for serialization_data<>


namespace makeshift
{

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
        friend std::string get_hint_impl(tag<T>, const hint_serializer& hintSerializer, SerializerT&&)
    {
        (void) hintSerializer;
        if constexpr (std::is_enum<T>::value && have_metadata_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>)
            return hint_impl(makeshift::detail::serialization_data<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>, hintSerializer.data);
        else if constexpr (std::is_same<T, bool>::value)
            return std::string("false") + hintSerializer.data.option_separator + std::string("true");
        else if constexpr (is_constrained_integer_v<T>)
            return T::verifier::get_hint(hintSerializer.data, typename T::constraint{ });
        else
            return { };
    }
};
hint_serializer(void) -> hint_serializer<>;
hint_serializer(const hint_options&) -> hint_serializer<>;
hint_serializer(hint_options&&) -> hint_serializer<>;


template <typename T, typename SerializerT,
          typename = std::enable_if_t<is_serializer_v<std::decay_t<SerializerT>>>>
    std::string hint(SerializerT&& serializer, tag<T> = { })
{
    return get_hint_impl(tag_v<T>, serializer, serializer);
}

template <typename T>
    std::string hint(tag<T> = { })
{
    return hint(hint_serializer(), tag_v<T>);
}


} // inline namespace serialize

} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
