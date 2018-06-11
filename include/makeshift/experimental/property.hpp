
#ifndef MAKESHIFT_PROPERTY_HPP_
#define MAKESHIFT_PROPERTY_HPP_


#include <functional>  // for invoke()
#include <type_traits> // for declval<>(), decay<>, is_member_object_pointer<>

#include <makeshift/type_traits.hpp> // for can_apply<>
#include <makeshift/functional.hpp>  // for is_callable<>
#include <makeshift/utility.hpp>     // for define_flags<>
#include <makeshift/tuple.hpp>
#include <makeshift/metadata.hpp>    // for property_metadata<>


namespace makeshift
{

inline namespace metadata
{

template <typename ObjectT, typename ValueT, typename GetterT, typename SetterT>
    class property_accessor
{
private:
    GetterT getter_;
    SetterT setter_;

public:
    property_accessor(GetterT&& _getter, SetterT&& _setter) : getter_(std::move(_getter)), setter_(std::move(_setter)) { }

    using object_type = ObjectT;
    using element_type = ValueT;
    
    constexpr ValueT get(const ObjectT& inst) const { return getter_(inst); }
    constexpr void set(ObjectT& inst, ValueT value) const { return setter_(inst, value); }
};

} // inline namespace metadata


namespace detail
{

struct invalid_setter
{
    template <typename C, typename V>
        void operator ()(C& inst, V&& value)
    {
        static_assert(sizeof(C) == ~std::size_t(0), "property does not have a setter"); // TODO: should this be a runtime assertion?
    }
};
struct invalid_getter
{
    template <typename C>
        universally_convertible operator ()(const C& inst)
    {
        static_assert(sizeof(C) == ~std::size_t(0), "property does not have a getter"); // TODO: should this be a runtime assertion?
        return { };
    }
};

template <typename... Cs>
    constexpr std::tuple<typename Cs::value_type...> tuple_from_integral_constants(type_sequence<Cs...>) noexcept
{
    return { Cs::value... };
}
template <typename Cs>
    constexpr auto tuple_from_integral_constants(void) noexcept
{
    return tuple_from_integral_constants(Cs{ });
}

template <typename F> struct is_getter_sig : std::false_type { };
template <typename V, typename C> struct is_getter_sig<V(const C&)> : std::true_type { };
template <typename F> using is_getter = is_getter_sig<callable_sig_t<F>>;

template <typename F> struct is_setter_sig : std::false_type { };
template <typename C, typename V> struct is_setter_sig<void(const C&, V)> : std::true_type { };
template <typename F> using is_setter = std::conjunction<is_setter_sig<callable_sig_t<F>>, std::is_member_object_pointer<F>>;

template <typename GetterT, typename SetterT> struct property_type : sig_return_type<callable_sig_t<GetterT>> { };
template <typename SetterT> struct property_type<invalid_getter, SetterT> { using type = std::decay_t<sig_arg_type_t<1, callable_sig_t<SetterT>>>; };
template <> struct property_type<invalid_getter, invalid_setter> { using type = none_t; };
template <typename GetterT, typename SetterT> using property_type_t = property_type<GetterT, SetterT>;

} // namespace detail

inline namespace metadata
{

template <typename AccessorT> struct has_getter;
template <typename ObjectT, typename ValueT, typename GetterT, typename SetterT> struct has_getter<property_accessor<ObjectT, ValueT, GetterT, SetterT>> : std::negation<std::is_same<GetterT, makeshift::detail::invalid_getter>> { };
template <typename AccessorT> constexpr bool has_getter_v = has_getter<AccessorT>::value;

template <typename AccessorT> struct has_setter;
template <typename ObjectT, typename ValueT, typename GetterT, typename SetterT> struct has_setter<property_accessor<ObjectT, ValueT, GetterT, SetterT>> : std::negation<std::is_same<SetterT, makeshift::detail::invalid_setter>> { };
template <typename AccessorT> constexpr bool has_setter_v = has_setter<AccessorT>::value;

template <typename ObjT, typename ObjAttributesT, typename PropAccessorsC, typename PropAttributesT>
    constexpr auto get_property_accessor(const type_metadata<ObjT, ObjAttributesT>& typeMetadata, const property_metadata<PropAccessorsC, PropAttributesT>& propMetadata)
{
    auto callablePropAttributes = propMetadata.attributes
        | tuple_filter<is_callable>();
    constexpr auto constAccessors = makeshift::detail::tuple_from_integral_constants<PropAccessorsC>()
        | tuple_filter<is_callable>();
    auto allAccessors = std::tuple_cat(callablePropAttributes, constAccessors);
    auto maybeGetter = allAccessors
        | tuple_filter<makeshift::detail::is_getter>()
        | single_or_default(makeshift::detail::invalid_getter{ });
    auto maybeSetter = allAccessors
        | tuple_filter<makeshift::detail::is_setter>()
        | single_or_default(makeshift::detail::invalid_setter{ });
    using Getter = decltype(maybeGetter);
    using Setter = decltype(maybeSetter);
    using PropType = makeshift::detail::property_type_t<Getter, Setter>;
    return property_accessor<ObjT, PropType, Getter, Setter>(flags_of_property<Getter, Setter>, std::move(maybeGetter), std::move(maybeSetter));
}

} // inline namespace metadata

} // namespace makeshift

#endif // MAKESHIFT_PROPERTY_HPP_
