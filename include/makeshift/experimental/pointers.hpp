
#ifndef MAKESHIFT_POINTERS_HPP_
#define MAKESHIFT_POINTERS_HPP_


#include <memory>
#include <stdexcept>      // TODO
#include <typeinfo>       // for bad_cast
#include <type_traits>    // for declval<>()

#include <gsl/gsl-lite.hpp> // for Expects(), Ensures(), not_null<>

#include <makeshift/type_traits.hpp> // for can_instantiate<>



namespace makeshift
{

inline namespace pointers
{


    // GSL utilities
using gsl::not_null;


    // pointer aliases for standard pointers
template <typename T>
    using raw_ptr = T*;
template <typename T>
    using raw_ref = not_null<T*>;
template <typename T>
    using raw_arg = not_null<T*>;


    // casts for standard pointers
template <typename DstT, typename SrcT>
    constexpr DstT* const_pointer_cast(SrcT* object) noexcept
{
    return const_cast<DstT*>(object);
}
template <typename DstT, typename SrcT>
    constexpr DstT* static_pointer_cast(SrcT* object) noexcept
{
    return static_cast<DstT*>(object);
}
template <typename DstT, typename SrcT>
    constexpr DstT* dynamic_pointer_cast(SrcT* object) noexcept
{
    return dynamic_cast<DstT*>(object);
}
/*template <typename DstT, typename SrcT>
    constexpr DstT* reinterpret_pointer_cast(SrcT* object) noexcept
{
    return reinterpret_cast<DstT*>(object);
}*/
template <typename DstT, typename SrcT>
    constexpr raw_ref<DstT> const_ref_cast(raw_ref<SrcT> object) noexcept
{
    return raw_ref<DstT>(const_cast<DstT*>(object.get()));
}
template <typename DstT, typename SrcT>
    constexpr raw_ref<DstT> static_ref_cast(raw_ref<SrcT> object) noexcept
{
    return raw_ref<DstT>(static_cast<DstT*>(object.get()));
}
template <typename DstT, typename SrcT>
    DstT* dynamic_pointer_cast(raw_ref<SrcT> object) noexcept
{
    return dynamic_cast<DstT*>(object.get());
}
    // Like dynamic_cast<>() for references, dynamic_ref_cast<>() will throw std::bad_cast if the cast fails.
    // To avoid an exception, use dynamic_pointer_cast<>() instead.
template <typename DstT, typename SrcT>
    raw_ref<DstT> dynamic_ref_cast(raw_ref<SrcT> object)
{
    return raw_ref<DstT>(&dynamic_cast<DstT&>(*object));
}
/*template <typename DstT, typename SrcT>
    constexpr raw_ref<DstT> reinterpret_ref_cast(raw_ref<SrcT> object) noexcept
{
    return raw_ref<DstT>(reinterpret_cast<DstT*>(object.get()));
}*/


    // pointer aliases for smart pointers
using std::unique_ptr;
template <typename T>
    using unique_ref = not_null<unique_ptr<T>>;
template <typename T>
    using unique_arg = not_null<unique_ptr<T>>;
using std::shared_ptr;
template <typename T>
    using shared_ref = not_null<shared_ptr<T>>;
template <typename T>
    using shared_arg = not_null<shared_ptr<T>>;


    // casts for smart pointers
using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::const_pointer_cast;
//using std::reinterpret_pointer_cast;
template <typename DstT, typename SrcT>
    constexpr shared_ref<DstT> const_ref_cast(const shared_ref<SrcT>& object) noexcept
{
    return shared_ref<DstT>(const_pointer_cast<DstT>(object.get()));
}
template <typename DstT, typename SrcT>
    constexpr shared_ref<DstT> static_ref_cast(const shared_ref<SrcT>& object) noexcept
{
    return shared_ref<DstT>(static_pointer_cast<DstT>(object.get()));
}
template <typename DstT, typename SrcT>
    constexpr shared_ptr<DstT> dynamic_pointer_cast(const shared_ref<SrcT>& object) noexcept
{
    return dynamic_pointer_cast<DstT>(object.get());
}
    // Like dynamic_cast<>() for references, dynamic_ref_cast<>() will throw std::bad_cast if the cast fails.
    // To avoid an exception, use dynamic_pointer_cast<>() instead.
template <typename DstT, typename SrcT>
    constexpr shared_ref<DstT> dynamic_ref_cast(const shared_ref<SrcT>& object) noexcept
{
    auto intermediate = dynamic_pointer_cast<DstT>(object.get());
    if (intermediate == nullptr)
        throw std::bad_cast();
    return shared_ref<DstT>(std::move(intermediate));
}
/*template <typename DstT, typename SrcT>
    constexpr shared_ref<DstT> reinterpret_ref_cast(const shared_ref<SrcT>& object) noexcept
{
    return shared_ref<DstT>(reinterpret_pointer_cast<DstT>(object.get()));
}*/


    // traits type to extract information from arbitrary pointer-like types
template <typename T>
    struct pointer_traits
{
    using pointer = typename T::pointer;
    using element_type = typename T::element_type;
};
template <typename T>
    struct pointer_traits<not_null<T>>
{
    using pointer = typename T::pointer;
    using element_type = typename T::element_type;
};
template <typename T>
    struct pointer_traits<T*>
{
    using pointer = T*;
    using element_type = T;
};


    // extract raw pointer from smart pointer
template <typename PtrT>
    typename pointer_traits<std::decay_t<PtrT>>::pointer raw(PtrT&& ptr)
{
    return ptr.get();
}
template <typename PtrT>
    raw_ref<typename pointer_traits<std::decay_t<PtrT>>::element_type> raw(not_null<PtrT>&& ptr)
{
    return raw_ref<typename pointer_traits<std::decay_t<PtrT>>::element_type>(ptr.get().get());
}


} // inline namespace pointers

namespace detail
{

    // TODO: explain
template <typename T, typename SC>
    struct substitute_class_;
template <typename T>
    struct substitute_class_<T, void> : std::integral_constant<bool, std::is_final<T>::value>
{
    using type = T;
};

template <typename T, typename SC>
    struct substitute_value_;
template <typename T>
    struct substitute_value_<T, void> : std::false_type
{
    using type = T;
};

} // namespace detail

inline namespace pointers
{


template <typename PtrT, typename ArgT = PtrT>
    struct default_pointer_
{
    using ptr = PtrT;
    using arg = ArgT;
};



template <typename T>
    constexpr default_pointer_<unique_ptr<T>> default_pointer(T*) noexcept
{
    return { };
}


template <typename T, typename SC = void>
    using ptr = typename decltype(default_pointer(std::declval<typename makeshift::detail::substitute_class_<T, SC>::type*>()))::ptr;
template <typename T, typename SC = void>
    using ref = not_null<typename decltype(default_pointer(std::declval<typename makeshift::detail::substitute_class_<T, SC>::type*>()))::ptr>;
template <typename T, typename SC = void>
    using arg = typename decltype(default_pointer(std::declval<typename makeshift::detail::substitute_class_<T, SC>::type*>()))::arg;


} // inline namespace pointers

namespace detail
{

    // SFINAE helpers for polymorphic_cast<>()
template <typename DstT, typename SrcT> using can_static_cast_r = decltype(static_cast<DstT>(std::declval<SrcT>()));
template <typename DstT, typename SrcT> using can_static_cast = can_instantiate<can_static_cast_r, DstT, SrcT>;
template <typename DstT, typename SrcT> constexpr bool can_static_cast_v = can_static_cast<DstT, SrcT>::value;

template <typename DstT, typename SrcT>
    constexpr decltype(auto) polymorphic_pointer_cast_impl(std::true_type /*canStaticCast*/, SrcT&& src) noexcept
{
    using namespace makeshift::pointers;
    return static_pointer_cast<DstT>(std::forward<SrcT>(src));
}
template <typename DstT, typename SrcT>
    decltype(auto) polymorphic_pointer_cast_impl(std::false_type /*canStaticCast*/, SrcT&& src)
{
    using namespace makeshift::pointers;
    return dynamic_pointer_cast<DstT>(std::forward<SrcT>(src));
}
template <typename DstT, typename SrcT>
    constexpr decltype(auto) polymorphic_ref_cast_impl(std::true_type /*canStaticCast*/, SrcT&& src) noexcept
{
    using namespace makeshift::pointers;
    return static_pointer_cast<DstT>(std::forward<SrcT>(src));
}
template <typename DstT, typename SrcT>
    decltype(auto) polymorphic_ref_cast_impl(std::false_type /*canStaticCast*/, SrcT&& src)
{
    using namespace makeshift::pointers;
    return dynamic_pointer_cast<DstT>(std::forward<SrcT>(src));
}


template <typename T>
    struct obj_wrapper
{
    obj_wrapper(obj_wrapper&&) = delete;
    obj_wrapper(const obj_wrapper&) = delete;
    obj_wrapper& operator =(obj_wrapper&&) = delete;
    obj_wrapper& operator =(const obj_wrapper&) = delete;
    
private:
    T instance_;

public:
    constexpr inline obj_wrapper(T&& rhs)
        : instance_(std::move(rhs))
    {
    }
    template <typename... Ts>
        constexpr inline obj_wrapper(Ts&&... args) // TODO: we must inject a factory here somehow
            : instance_(std::forward<Ts>(args)...)
    {   
    }

    constexpr T* get(void) const noexcept { return &instance_; }
    constexpr T* operator ->(void) const noexcept { return &instance_; }
    constexpr T& operator *(void) const noexcept { return instance_; }
};

template <typename T>
    struct ptr_wrapper
{
    ptr_wrapper(ptr_wrapper&&) = delete;
    ptr_wrapper(const ptr_wrapper&) = delete;
    ptr_wrapper& operator =(ptr_wrapper&&) = delete;
    ptr_wrapper& operator =(const ptr_wrapper&) = delete;
    
private:
    ptr<T> instance_;

public:
    constexpr inline ptr_wrapper(ptr<T> rhs)
        : instance_(not_null<ptr<T>>(std::move(rhs)))
    {
    }
    template <typename... Ts>
        constexpr inline ptr_wrapper(Ts&&... args) // TODO: we must inject a factory here somehow
            : instance_(std::forward<Ts>(args)...)
    {   
    }

    constexpr T* get(void) const noexcept { return instance_.get(); }
    constexpr T* operator ->(void) const noexcept { return get(); }
    constexpr T& operator *(void) const noexcept { return *get(); }
};

template <typename T, typename SC>
    struct substitute_obj_
{
    using subst = substitute_class_<T, SC>;
    using value_type = typename subst::type;
    using type = typename std::conditional<subst::value, obj_wrapper<value_type>, ptr_wrapper<value_type>>::type;
};

} // namespace detail

inline namespace pointers
{

    // A polymorphic cast is a downcast that can be statically guaranteed to succeed.
    // polymorphic_pointer_cast<>() uses static_pointer_cast<>() if possible.
    // If the source and destination type are related by virtual inheritance, a static cast is not possible;
    // in this case the implementation falls back to dynamic_pointer_cast<>().
template <typename DstT, typename SrcPtrT>
    constexpr decltype(auto) polymorphic_pointer_cast(SrcPtrT&& src)
{
    return makeshift::detail::polymorphic_pointer_cast_impl<DstT>(
        makeshift::detail::can_static_cast<typename pointer_traits<DstT>::pointer, typename pointer_traits<std::decay_t<SrcPtrT>>::pointer>{},
        std::forward<SrcPtrT>(src));
}
    // A polymorphic cast is a downcast that can be statically guaranteed to succeed.
    // polymorphic_ref_cast<>() uses static_ref_cast<>() if possible.
    // If the source and destination type are related by virtual inheritance, a static cast is not possible;
    // in this case the implementation falls back to polymorphic_ref_cast<>().
template <typename DstT, typename SrcPtrT>
    constexpr decltype(auto) polymorphic_ref_cast(SrcPtrT&& src)
{
    return makeshift::detail::polymorphic_ref_cast_impl<DstT>(
        makeshift::detail::can_static_cast<typename pointer_traits<DstT>::pointer, typename pointer_traits<std::decay_t<SrcPtrT>>::pointer>{},
        std::forward<SrcPtrT>(src));
}


template <typename T, typename SC = void>
    using obj = typename makeshift::detail::substitute_obj_<T, SC>::type;
template <typename T, typename SC = void>
    using val = typename makeshift::detail::substitute_value_<T, SC>::type;


    /*  
     * What constitutes a concrete type? How does it make a difference?
     * 
     *       X  obj<X>  ref<X>  ptr<X>  arg<X>
     * =========================================
     *       I  ref<I>  ref<I>  ptr<I>  arg<I>
     *       C  ref<C>  ref<C>  ptr<C>  arg<C>
     * final C  C       ref<C>  ptr<C>  arg<C>
     *  C <- I  C       ref<C>  ptr<C>  arg<C>
     *  C <- C  C       ref<C>  ptr<C>  arg<C>
     * 
     *      X   val<X>
     * =================
     *      T   T
     * U <- T   U
     *  
     */

//template <typename T>
//    struct 



/// - typeof<>, obj<>, val<>, ref<>, ptr<>, arg<>


} // inline namespace pointers

} // namespace makeshift

#endif // MAKESHIFT_POINTERS_HPP_
