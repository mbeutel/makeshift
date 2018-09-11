
#ifndef INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
#define INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_


#include <string>

#include <makeshift/type_traits.hpp> // for tag<>
#include <makeshift/serialize.hpp>   // for define_serializer<>


namespace makeshift
{

inline namespace serialize
{


struct hint_serializer_args
{
    std::string_view option_separator = "|";
    std::string_view flags_separator = ",";
    std::string_view string_lquote = "";
    std::string_view string_rquote = "";

    constexpr hint_serializer_args(/*void*/) noexcept = default;
};

template <typename BaseT = void>
    struct hint_serializer_t : define_serializer<hint_serializer_t, BaseT, hint_serializer_args>
{
    using base = define_serializer<makeshift::serialize::hint_serializer_t, BaseT, hint_serializer_args>;
    using base::base;
    
    //get_hint_impl()
};

constexpr hint_serializer_t<> hint_serializer{ };


template <typename T, typename SerializerT>
    std::string hint(SerializerT& serializer, tag<T> = { })
{

}

template <typename T>
    std::string hint(tag<T> = { })
{
    return hint(hint_serializer, tag_v<T>);
}


} // inline namespace serialize

} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
