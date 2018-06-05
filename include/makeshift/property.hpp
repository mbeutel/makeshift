
#ifndef MAKESHIFT_PROPERTY_HPP_
#define MAKESHIFT_PROPERTY_HPP_


#include <type_traits> // for declval<>(), decay<>, is_invocable<>

#include <makeshift/detail/meta.hpp> // for can_apply<>

#include <makeshift/types.hpp>    // for define_flags<>
#include <makeshift/metadata.hpp> // for property_metadata<>

#include <makeshift/experimental/tuple.hpp>


namespace makeshift
{

inline namespace metadata
{

struct PropertyFlag : define_flags<PropertyFlag>
{
    static constexpr flag readable { 1 };
    static constexpr flag writable { 2 };
};
using PropertyFlags = PropertyFlag::flags;


} // inline namespace metadata


namespace detail
{


struct InvalidSetter
{
    template <typename C, typename V>
        void operator ()(C& inst, V&& value)
    {
        static_assert(sizeof(V) == ~std::size_t(0), "property does not have a setter"); // TODO: should this be a runtime assertion?
    }
};
struct InvalidGetter
{
    template <typename C>
        UniversallyConvertibleType operator ()(const C& inst)
    {
        static_assert(sizeof(V) == ~std::size_t(0), "property does not have a getter"); // TODO: should this be a runtime assertion?
        return { };
    }
};

template <typename ObjectT, typename ValueT, typename GetterT, typename SetterT>
    class property_accessor
{
private:
    PropertyFlags flags_;
    GetterT getter_;
    SetterT setter_;

public:
    property_accessor(GetterT&& _getter, SetterT&& _setter) : getter_(std::move(_getter)), setter_(std::move(_setter)) { }

    using object_type = ObjectT;
    using element_type = ValueT;
    
    constexpr PropertyFlags flags(void) noexcept { return flags_; }
    constexpr ValueT get(const ObjectT& inst) { return getter_(inst); }
    constexpr void set(ObjectT& inst, ValueT value) { return setter_(inst, value); }
}

template <typename F> using is_functor_r = decltype(&T::operator ());
template <typename F> using is_functor_t = can_apply_t<is_functor_r, F>;
template <typename F> constexpr bool is_functor = is_functor_t<T>::value;

template <typename F> struct is_func_t : std::false_type { };
template <typename R, typename... ArgsT> struct is_func_t<R (*)(ArgsT...)> : std::true_type { };
template <typename F> constexpr bool is_func = is_func_t<T>::value;

template <typename F> struct functor_sig_0_;
template <typename R, typename... ArgsT> struct functor_sig_0_<R(ArgsT...)> { using type = R(ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct functor_sig_0_<R (C::*)(ArgsT...)> : functor_sig_0_<R(ArgsT...)> { };
template <typename R, typename C, typename... ArgsT> struct functor_sig_0_<R (C::*)(ArgsT...) const> : functor_sig_0_<R(ArgsT...)> { };
template <typename F> struct functor_sig : functor_sig_0_<decltype(&F::operator ())> { };
template <typename F> using functor_sig_t = typename functor_sig<F>::type;

template <typename F> struct func_sig;
template <typename R, typename... ArgsT> struct func_sig<R (*)(ArgsT...)> { using type = R(ArgsT...); };

template <typename F> struct is_callable_t : std::integral_constant<bool, is_functor<F> || is_func<F>> { };
template <typename F> constexpr bool is_callable = is_callable_t<T>::value;

template <typename F, bool IsFunctor, bool IsFunc> struct callable_sig_0_;
template <typename F> struct callable_sig_0_<F, true, false> : functor_sig<F> { };
template <typename F> struct callable_sig_0_<F, false, true> : func_sig<F> { };
template <typename F> struct callable_sig : callable_sig_0_<F, is_functor<F>, is_func<F>> { };
template <typename F> using callable_sig_t = typename callable_sig<F>::type;

struct none_t { };

} // namespace detail


inline namespace metadata
{

template <typename ObjT, typename ObjAttributesT, typename PropAccessorsC, typename PropAttributesT>
    auto get_property_accessor(const type_metadata<ObjT, ObjAttributesT>& typeMetadata, const property_metadata<PropAccessorsC, PropAttributesT>& propMetadata)
{
    auto single_or_default = tuple_freduce(makeshift::detail::none_t{ },
        overload(
            [](makeshift::detail::none_t, makeshift::detail::none_t) { return makeshift::detail::none_t{ }; },
            [](makeshift::detail::none_t, auto&& rhs) { return std::move(rhs); },
            [](auto&& lhs, makeshift::detail::none_t) { return std::move(lhs); }
        )
    );

    auto attr_getter = single_or_default(
        tuple_map(propMetadata.attributes,
            [](auto&& func)
            {
                using Func = std::decay_t<decltype(func)>;
                if constexpr (makeshift::detail::is_callable<Func>)
                {
                    using Sig = makeshift::detail::callable_sig_t<Func>;
                    if constexpr (std::is_invocable<Sig, const ObjT&>::value) // TODO
                    {
                        return std::forward<decltype(func)>(func);
                    }
                    else
                        return makeshift::detail::none_t{ };
                }
                else
                    return makeshift::detail::none_t{ };
            }
        )
    );
    auto attr_setter = single_or_default(
        tuple_map(propMetadata.attributes,
            [](auto&& func)
            {
                using Func = std::decay_t<decltype(func)>;
                if constexpr (makeshift::detail::is_callable<Func>)
                {
                    using Sig = makeshift::detail::callable_sig_t<Func>;
                    if constexpr (std::is_invocable<Sig, const ObjT&>::value)
                    {
                        return std::forward<decltype(func)>(func);
                    }
                    else
                        return makeshift::detail::none_t{ };
                }
                else
                    return makeshift::detail::none_t{ };
            }
        )
    );
    auto attr_setter = single_or_default(
        tuple_map(propMetadata.attributes,
            [](auto&& func)
            {
                if constexpr (std::is_invocable<std::decay_t<decltype(func)>, ObjT&, >::value)
                    return func;
                else
                    return makeshift::detail::none_t{ };
            }
        )
    );

    // first look for a lambda accessor, then for member accessors
}

} // inline namespace metadata

} // namespace makeshift

#endif // MAKESHIFT_PROPERTY_HPP_
