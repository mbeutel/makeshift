
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <cstdint>     // for [u]int[8|16|32|64]_t
#include <type_traits> // for is_base_of<>, integral_constant<>, conjunction<>, disjunction<>

#include <makeshift/type_traits2.hpp> // for can_instantiate<>, type<>
#include <makeshift/metadata2.hpp>    // for named_t<>
#include <makeshift/tuple2.hpp>       // for array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_EMPTY_BASES


namespace makeshift
{

namespace detail
{


template <typename T> using declared_metadata_t = decltype(reflect(type_t<T>{ }));
template <typename T> struct declares_metadata : can_instantiate<makeshift::detail::declared_metadata_t, T> { };


template <typename NewParamsT, typename OldParamsT>
    struct override_params_1_
{
    using new_categories = typename NewParamsT::parameter_categories;
    using all_categories = typename unique_sequence_<type_sequence_cat_t<typename NewParamsT::parameter_categories, typename OldParamsT::parameter_categories>>::type;

    template <typename CategoryT>
        constexpr static const NewParamsT& select_impl(std::true_type /*fromNew*/, const NewParamsT& newMetadata, const OldParamsT&)
    {
        return newMetadata;
    }
    template <typename CategoryT>
        constexpr static const OldParamsT& select_impl(std::false_type /*fromNew*/, const NewParamsT&, const OldParamsT& oldMetadata)
    {
        return oldMetadata;
    }
    template <typename CategoryT>
        constexpr static decltype(auto) select(const NewParamsT& newMetadata, const OldParamsT& oldMetadata)
    {
        return select_impl<CategoryT>(std::integral_constant<bool, try_index_of_type_in<CategoryT, new_categories>::value != -1>{ }, newMetadata, oldMetadata);
    }
};

template <typename NewParamsT, typename OldParamsT, typename CategoriesT>
    struct override_params_2_;
template <typename NewParamsT, typename OldParamsT, typename... CategoriesT>
    struct override_params_2_<NewParamsT, OldParamsT, type_sequence<CategoriesT...>>
{
    using OP1 = override_params_1_<NewParamsT, OldParamsT>;
    using all_parameters = type_sequence<std::decay_t<decltype(select_parameter(OP1::template select<CategoriesT>(std::declval<const NewParamsT&>(), std::declval<const OldParamsT&>()), CategoriesT{ }))>...>;
};

template <typename NewParamsT, typename OldParamsT, typename CategoriesT, typename ParametersT>
    struct override_params_0_;
template <typename NewParamsT, typename OldParamsT, typename... CategoriesT, typename... ParametersT>
    struct MAKESHIFT_EMPTY_BASES override_params_0_<NewParamsT, OldParamsT, type_sequence<CategoriesT...>, type_sequence<ParametersT...>>
        : ParametersT...
{
    constexpr override_params_0_(const NewParamsT& newMetadata, const OldParamsT& oldMetadata)
        : ParametersT(override_params_1_<NewParamsT, OldParamsT>::template select<CategoriesT>(newMetadata, oldMetadata))...
    {
    }
};

template <typename NewParamsT, typename OldParamsT>
    struct override_params
        : override_params_0_<NewParamsT, OldParamsT,
                            typename override_params_1_<NewParamsT, OldParamsT>::all_categories,
                            typename override_params_2_<NewParamsT, OldParamsT, typename override_params_1_<NewParamsT, OldParamsT>::all_categories>::all_parameters>
{
    using base = override_params_0_<NewParamsT, OldParamsT,
                                   typename override_params_1_<NewParamsT, OldParamsT>::all_categories,
                                   typename override_params_2_<NewParamsT, OldParamsT, typename override_params_1_<NewParamsT, OldParamsT>::all_categories>::all_parameters>;
    using base::base;
};


template <typename T, typename = void>
    struct default_metadata
{
    using parameter_categories = type_sequence<>;
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
        : heterogeneous_values_t<typename apply_<type_sequence, typename TypeEnumT::types>::type>
{
private:
    using base = heterogeneous_values_t<typename apply_<type_sequence, typename TypeEnumT::types>::type>;

public:
    constexpr default_metadata(void)
        : base(typename TypeEnumT::types{ })
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
    struct default_metadata<bool> : name_t, values_t<bool, 2, name_t>
{
    using parameter_categories = type_sequence<name_tag, values_tag>;

    constexpr default_metadata(void)
        : name_t("bool"),
          values_t<bool, 2, name_t>({ makeshift::value(false, makeshift::name("false")), makeshift::value(true, makeshift::name("true")) })
    {
    }
};

struct metadata_tag { };

template <typename T, bool RawMetadata>
    struct metadata_of_0;
template <typename T>
    struct MAKESHIFT_EMPTY_BASES metadata_of_0<T, true> : override_params<declared_metadata_t<T>, default_metadata<T>>, metadata_tag
{
    constexpr metadata_of_0(void)
        : override_params<declared_metadata_t<T>, default_metadata<T>>(reflect(type_t<T>{ }), { })
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
