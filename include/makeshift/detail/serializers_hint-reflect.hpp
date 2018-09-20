
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_HINT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_HINT_REFLECT_HPP_


#include <string>
#include <sstream>
#include <iomanip>

#include <makeshift/serializers/hint.hpp>
#include <makeshift/type_traits.hpp>      // for tag<>
#include <makeshift/serialize.hpp>        // for metadata_tag_of_serializer<>
#include <makeshift/reflect.hpp>          // for member_accessor()
#include <makeshift/metadata.hpp>         // for have_metadata<>, metadata_of<>, caption_metadata
#include <makeshift/tuple.hpp>            // for tuple_foreach()


namespace makeshift
{

namespace detail
{


template <typename T, typename SerializerT>
    std::string get_compound_hint(SerializerT&&, tag<T>)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();

    std::ostringstream sstr;
    sstr << "{ ";
    bool first = true;
    tuple_foreach(members, [&](auto&& member)
    {
        using Accessor = decltype(member_accessor(member));
        using Member = typename Accessor::value_type;

        if (first)
            first = false;
        else
            sstr << ", ";

        std::string_view theName = get_or_default<std::string_view>(member.attributes);
        if (!theName.empty())
            sstr << theName;
        else
        {
            if constexpr (!have_metadata_v<Member, MetadataTag>)
                sstr << "val";
            else
            {
                auto slug = get_or_default<std::string_view>(metadata_of<Member, MetadataTag>.attributes);
                if (!slug.empty())
                    sstr << slug;
                else
                {
                    auto caption = get_or_default<caption_metadata>(metadata_of<Member, MetadataTag>.attributes);
                    if (!caption.value.empty())
                        sstr << std::quoted(caption.value);
                    else
                        sstr << "val";
                }
            }
        }
    });
    sstr << " }";
    return sstr.str();
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_HINT_REFLECT_HPP_
