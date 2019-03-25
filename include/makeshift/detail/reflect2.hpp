
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <cstdint>     // for [u]int[8|16|32|64]_t
#include <type_traits> // for is_base_of<>, integral_constant<>, conjunction<>, disjunction<>

#include <makeshift/type_traits2.hpp> // for can_apply<>, type<>
#include <makeshift/metadata2.hpp>    // for named_t<>
#include <makeshift/tuple2.hpp>       // for array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_EMPTY_BASES


namespace makeshift
{

namespace detail
{


template <typename T> using declared_metadata_t = decltype(reflect(type_t<T>{ }));
template <typename T> struct declares_metadata : can_apply<makeshift::detail::declared_metadata_t, T> { };


template <typename NewMetadataT, typename OldMetadataT>
    struct merge_metadata_1_
{
    using new_categories = typename NewMetadataT::parameter_categories;
    using all_categories = typename unique_sequence_<type_sequence2_cat_t<typename NewMetadataT::parameter_categories, typename OldMetadataT::parameter_categories>>::type;

    template <typename CategoryT>
        constexpr static const NewMetadataT& select_impl(std::true_type /*fromNew*/, const NewMetadataT& newMetadata, const OldMetadataT&)
    {
        return newMetadata;
    }
    template <typename CategoryT>
        constexpr static const OldMetadataT& select_impl(std::false_type /*fromNew*/, const NewMetadataT&, const OldMetadataT& oldMetadata)
    {
        return oldMetadata;
    }
    template <typename CategoryT>
        constexpr static decltype(auto) select(const NewMetadataT& newMetadata, const OldMetadataT& oldMetadata)
    {
        return select_impl<CategoryT>(std::integral_constant<bool, try_index_of_type_in<CategoryT, new_categories>::value != -1>{ }, newMetadata, oldMetadata);
    }
};

template <typename NewMetadataT, typename OldMetadataT, typename CategoriesT>
    struct merge_metadata_2_;
template <typename NewMetadataT, typename OldMetadataT, typename... CategoriesT>
    struct merge_metadata_2_<NewMetadataT, OldMetadataT, type_sequence2<CategoriesT...>>
{
    using MM1 = merge_metadata_1_<NewMetadataT, OldMetadataT>;
    using all_parameters = type_sequence2<std::decay_t<decltype(select_parameter(MM1::template select<CategoriesT>(std::declval<const NewMetadataT&>(), std::declval<const OldMetadataT&>()), CategoriesT{ }))>...>;
};

template <typename NewMetadataT, typename OldMetadataT, typename CategoriesT, typename ParametersT>
    struct merge_metadata_0_;
template <typename NewMetadataT, typename OldMetadataT, typename... CategoriesT, typename... ParametersT>
    struct MAKESHIFT_EMPTY_BASES merge_metadata_0_<NewMetadataT, OldMetadataT, type_sequence2<CategoriesT...>, type_sequence2<ParametersT...>>
        : ParametersT...
{
    constexpr merge_metadata_0_(const NewMetadataT& newMetadata, const OldMetadataT& oldMetadata)
        : ParametersT(merge_metadata_1_<NewMetadataT, OldMetadataT>::template select<CategoriesT>(newMetadata, oldMetadata))...
    {
    }
};

template <typename NewMetadataT, typename OldMetadataT>
    struct merge_metadata
        : merge_metadata_0_<NewMetadataT, OldMetadataT,
                            typename merge_metadata_1_<NewMetadataT, OldMetadataT>::all_categories,
                            typename merge_metadata_2_<NewMetadataT, OldMetadataT, typename merge_metadata_1_<NewMetadataT, OldMetadataT>::all_categories>::all_parameters>
{
    using base = merge_metadata_0_<NewMetadataT, OldMetadataT,
                                   typename merge_metadata_1_<NewMetadataT, OldMetadataT>::all_categories,
                                   typename merge_metadata_2_<NewMetadataT, OldMetadataT, typename merge_metadata_1_<NewMetadataT, OldMetadataT>::all_categories>::all_parameters>;
    using base::base;
};


template <typename T, typename = void>
    struct default_metadata
{
    using parameter_categories = type_sequence2<>;
};
template <typename T>
    struct default_metadata<type_t<T>>
        : values_t<type_t<T>, 1>
{
    constexpr default_metadata(void)
        : values_t<type_t<T>, 1>({ { } })
    {
    }
};
template <typename TypeEnumT>
    struct default_metadata<TypeEnumT, std::enable_if_t<std::is_base_of<type_enum_base, TypeEnumT>::value>>
        : values_t<TypeEnumT, TypeEnumT::size>
{
private:
    template <std::size_t... Is>
        static constexpr std::array<TypeEnumT, TypeEnumT::size> _values_impl(std::index_sequence<Is...>) noexcept
    {
        return { TypeEnumT(int(Is))... };
    }

public:
    constexpr default_metadata(void)
        : values_t<TypeEnumT, TypeEnumT::size>(_values_impl(std::make_index_sequence<TypeEnumT::size>{ }))
    {
    }
};
template <> struct default_metadata<float> : name_t { constexpr default_metadata(void) : name_t("float") { } };
template <> struct default_metadata<double> : name_t { constexpr default_metadata(void) : name_t("double") { } };
template <> struct default_metadata<std::int8_t> : name_t { constexpr default_metadata(void) : name_t("int8") { } };
template <> struct default_metadata<std::int16_t> : name_t { constexpr default_metadata(void) : name_t("int16") { } };
template <> struct default_metadata<std::int32_t> : name_t { constexpr default_metadata(void) : name_t("int32") { } };
template <> struct default_metadata<std::int64_t> : name_t { constexpr default_metadata(void) : name_t("int64") { } };
template <>
    struct default_metadata<bool> : name_t, values_t<bool, 2>, value_names_t<2>
{
    using parameter_categories = type_sequence2<name_tag, values_tag, value_names_tag>;

    constexpr default_metadata(void)
        : name_t("bool"),
          values_t<bool, 2>({ false, true }),
          value_names_t<2>({ "false", "true" })
    {
    }
};

struct metadata_tag { };

template <typename T, bool RawMetadata>
    struct metadata_of_0;
template <typename T>
    struct MAKESHIFT_EMPTY_BASES metadata_of_0<T, true> : merge_metadata<declared_metadata_t<T>, default_metadata<T>>, metadata_tag
{
    constexpr metadata_of_0(void)
        : merge_metadata<declared_metadata_t<T>, default_metadata<T>>(reflect(type_t<T>{ }), { })
    {
    }
};
template <typename T>
    struct MAKESHIFT_EMPTY_BASES metadata_of_0<T, false> : default_metadata<T>, metadata_tag
{
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
