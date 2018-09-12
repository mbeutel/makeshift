
#ifndef INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
#define INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_


#include <string>

#include <makeshift/type_traits.hpp>           // for tag<>
#include <makeshift/serialize.hpp>             // for define_serializer<>

#include <makeshift/detail/serialize_enum.hpp> // for serialization_data<>


namespace makeshift
{

inline namespace serialize
{


struct hint_options_t
{
    std::string_view option_separator = "|";
    std::string_view flags_separator = ",";

    constexpr hint_options_t(/*void*/) noexcept = default;
};


} // inline namespace serialize


namespace detail
{


} // namespace detail


inline namespace serialize
{


struct hint_serializer_args
{
    hint_options_t hint_options;

    constexpr hint_serializer_args(/*void*/) noexcept = default;
    constexpr hint_serializer_args(hint_options_t _hintOptions) noexcept
        : hint_options(_hintOptions)
    {
    }
};

template <typename BaseT = void>
    struct hint_serializer_t : define_serializer<hint_serializer_t, BaseT, hint_serializer_args>
{
    using base = define_serializer<makeshift::hint_serializer_t, BaseT, hint_serializer_args>;
    using base::base;
    
    template <typename T, typename SerializerT>
        friend std::string get_hint_impl(tag<T>, const hint_serializer_t& hintSerializer, SerializerT&)
    {
        (void) hintSerializer;
        if constexpr (std::is_enum<T>::value && have_metadata_v<T, serializer_metadata_tag_t<std::decay_t<SerializerT>>>)
            return hint_impl(makeshift::detail::serialization_data<T, serializer_metadata_tag_t<std::decay_t<SerializerT>>>, hintSerializer.hint_options);
        else if constexpr (std::is_same<T, bool>::value)
            return std::string("false") + hintSerializer.hint_options.option_separator + std::string("true");
        else if constexpr (is_constrained_integer_v<T>)
            return T::verifier::get_hint(hintSerializer.hint_options, typename T::constraint{ });
        else
            return { };
    }
};

constexpr hint_serializer_t<> hint_serializer{ };


template <typename T, typename SerializerT>
    std::string hint(SerializerT& serializer, tag<T> = { })
{
    return get_hint_impl(tag_v<T>, serializer, serializer);
}

template <typename T>
    std::string hint(tag<T> = { })
{
    return hint(hint_serializer, tag_v<T>);
}


} // inline namespace serialize

} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
