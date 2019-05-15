
#ifndef INCLUDED_MAKESHIFT_PROTOTYPES_HPP_
#define INCLUDED_MAKESHIFT_PROTOTYPES_HPP_


#include <type_traits> // for integral_constant<>
#include <string_view> // TODO: preliminary

#include <makeshift/version.hpp> // for MAKESHIFT_EMPTY_BASES, MAKESHIFT_NODISCARD

#include <makeshift/detail/prototypes.hpp>


namespace makeshift
{


template <typename MemberT, typename T>
    class property : makeshift::detail::define_member<MemberT, property<MemberT, T>>
{
private:
    T value_;
public:
    constexpr property(T _value) noexcept
        : value_(std::move(_value))
    {
    }
    constexpr const T& operator ()(void) const & noexcept { return value_; }
    constexpr T&& operator ()(void) && noexcept { return std::move(value_); }
};

template <typename MemberT, typename F>
    class MAKESHIFT_EMPTY_BASES method : makeshift::detail::define_member<MemberT, method<MemberT, F>>, private F
{
public:
    constexpr method(F _func) noexcept
        : F(std::move(_func))
    {
    }
    
    // TODO: here we need an additional mechanism for discarding the "self" parameter if not needed
    
    constexpr const F& operator ()(void) const & noexcept { return *this; }
    constexpr F&& operator ()(void) && noexcept { return std::move(*this); }
};

template <typename MemberT, typename T>
    struct define_property
{
    constexpr property<MemberT, T> operator =(T value) const
    {
        return { value };
    }
    constexpr const T& operator ()(const property<MemberT, T>& _property) const noexcept
    {
        return _property();
    }
    constexpr T&& operator ()(property<MemberT, T>&& _property) const noexcept
    {
        return std::move(_property)();
    }
};

template <typename MemberT>
    struct define_method
{
    template <typename F>
        constexpr method<MemberT, F> operator =(F func) const
    {
        return { func };
    }
    template <typename F>
        constexpr const F& operator ()(const method<MemberT, F>& _method) const noexcept
    {
        return _method();
    }
    constexpr F&& operator ()(method<MemberT, F>&& _method) const noexcept
    {
        return std::move(_method)();
    }
};

struct name_t : define_property<name_t, std::string_view>
{
    template <typename AggregateT>
        struct interface
    {
        constexpr std::string_view name(void) const noexcept
        {
            return makeshift::select_member(static_cast<const AggregateT&>(*this), name_t{ })();
        }
    };
};
constexpr inline name_t name{ };

struct model_property_t
{
    template <typename T>
        struct _mixin
    {
        
    };
    
    // foo.enabled() <-- concept ModelProperty<bool>, apply to model instance to obtain a bool
    // model, viewmodel, view; do we make a viewmodel? it's actually a metamodel, but we can use define_model() and make_model()
};


struct to_stream_t : define_method<to_stream_t>
{
    template <typename AggregateT>
        struct interface
    {
        template <typename T>
            std::ostream& to_stream(std::ostream& stream, const T& obj) const
        {
            return makeshift::select_member(static_cast<const AggregateT&>(*this), to_stream_t{ })()(static_cast<const AggregateT&>(*this), stream, obj);
        }
    };

    template <typename F>
        class mixin : makeshift::detail::define_member<to_stream_t, mixin<F>>, F
    {
    public:
        constexpr mixin(F _func)
            : F(std::move(_func))
        {
        }
        template <typename T,
                  typename = std::enable_if_t<std::is_invocable<F, const DerivedT&, std::ostream&, const T&>>> // TODO: with result?
            std::ostream& operator ()(type<to_stream_op>, std::ostream& stream, const T& obj) const
        {
            return func_(static_cast<const DerivedT&>(*this), stream, obj);
        }

    };

    /*
    
        member:
            - func
            
        mixin:
            - to_stream()
            - extraction
    
     */

    template <typename F>
        class mixin : 
    {
    private:
        F func_;
        
    public:
        constexpr member(F _func) noexcept
            : func_(std::move(_func))
        {
        }
        constexpr const F& operator ()(void) const & noexcept { return func_; }
        constexpr F&& operator ()(void) && noexcept { return std::move(func_); }
        template <typename... ArgsT,
                  typename = std::enable_if_t<std::is_invocable_v<const F&, ArgsT&&...>>>
            decltype(auto) operator ()(to_stream_t, ArgsT&&... args) const
        {
            return func_(static_cast<const DerivedT&>(*this), stream, obj);
        }
>
            
    };
    
    template <typename AggregateT>
        struct mixin
    {
    private:
        F func_;
        
    public:
        constexpr member(F _func) noexcept
            : func_(std::move(_func))
        {
        }
        constexpr const F& operator ()(void) const & noexcept { return func_; }
        constexpr F operator ()(void) && noexcept { return std::move(func_); }
    };
    
    template <typename F>
        constexpr member<std::decay_t<F>> operator =(F&& func) const
    {
        return { std::forward<F>(func); }
    }
    template <typename F>
        constexpr const F& operator ()(const member<F>& member) const noexcept
    {
        return member();
    }
    template <typename F>
        constexpr F operator ()(member<F>&& member) const
    {
        return std::move(member)();
    }
};



} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_PROTOTYPES_HPP_
