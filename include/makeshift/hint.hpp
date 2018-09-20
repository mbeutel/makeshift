
#ifndef INCLUDED_MAKESHIFT_HINT_HPP_
#define INCLUDED_MAKESHIFT_HINT_HPP_


#include <string>
#include <type_traits> // for enable_if<>

#include <makeshift/type_traits.hpp> // for tag<>, is_serializer<>


namespace makeshift
{

inline namespace serialize
{


template <typename T, typename SerializerT,
          typename = std::enable_if_t<is_serializer_v<std::decay_t<SerializerT>>>>
    std::string get_hint(SerializerT&& serializer, tag<T> = { })
{
    return get_hint_impl(tag_v<T>, serializer, serializer);
}


} // inline namespace serialize

} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_HINT_HPP_
