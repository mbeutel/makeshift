
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_


#include <makeshift/reflect.hpp> // for member_accessor()
#include <makeshift/tuple.hpp>   // for tuple_foreach()


namespace makeshift
{

namespace detail
{


template <typename T, typename SerializerT>
    void aggregate_to_stream(std::ostream& stream, const T& value, SerializerT&& serializer)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();

    stream << "{ ";
    bool first = true;
    tuple_foreach(members, [&](auto&& member)
    {
        if (first)
            first = false;
        else
            stream << ", ";

        auto accessor = member_accessor(member);
        auto theName = get_or_default<std::string_view>(member.attributes);
        if (!theName.empty())
        {
            string_to_stream(stream, theName);
            stream << ": ";
        }
        stream << streamable(accessor(value), serializer);
    });
    stream << " }";
}
template <typename T, typename SerializerT>
    void aggregate_from_stream(std::istream& stream, T& value, SerializerT&& serializer)
{
    static_assert(sizeof(T) == std::size_t(-1), "not implemented yet");
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_
