
#ifndef MAKESHIFT_CONTRACT_HPP_
#define MAKESHIFT_CONTRACT_HPP_


#include <makeshift/version.hpp> // for MAKESHIFT_INTELLISENSE_PARSER


#include <makeshift/type_traits.hpp> // for ?


namespace makeshift
{



namespace detail
{


template <typename T, typename C = void> struct substitute;
template <typename T> struct substitute<T, void> { using type = T; };

#ifdef MAKESHIFT_INTELLISENSE_PARSER
template <typename T, typename C> struct substitute { using type = T; };
template <typename T, typename = void> using substitute_t = T;
#else // MAKESHIFT_INTELLISENSE_PARSER
template <typename T, typename C = void> using substitute_t = typename substitute<T, C>::type;
#endif // MAKESHIFT_INTELLISENSE_PARSER


} // namespace detail


inline namespace types
{

/*
    vocabulary:
    1 movable, copyable, disposable, equatable, hashable
        - movable: not used
        - copyable: copy constructor in value wrapper, virtual _clone() method in interface
        - clos[e?]able/disposable: new member function close()/dispose(), altered member function disposed(), virtual _dispose()/_disposed() methods in interface
        - equatable: operator overloads in value wrapper, virtual _equal() method in interface
        - hashable: template specialization for value wrapper with SFINAE, virtual _hash() method in interface
        => these concepts can be treated as modular! the user should be able to define others (e.g. comparable, invocable).
    | constructor(...)
    + base[s]<>, virtual_base[s]<>
    1 class args
    + config

*/

template <typename I, typename... AttributesT>
    struct define_interface
{
};

/*

    more vocabulary:
    - obj<>, unique<>, shared<>: obj wrappers
    - raw<>, ref<>, ptr<>: simple substituting aliases
    - unique_ptr<>, shared_ptr<>: substituting smart ptr aliases
    - make_unique<>(), make_shared<>(): item
    - raw_class<I> = I::metaclass; -> get<raw_class<I>>(cfg) = get<class_obj<I>>(cfg)
    - raw_class<>, class_ref<>, class_ptr<>: ...
    - class_of<C>, class_of<C>(classArgs...)
    - class_obj<> = obj<I::metaclass>: class obj wrapper
    - instance_of<>(classArgs..., args...)

*/



#ifdef MAKESHIFT_INTELLISENSE_PARSER
template <typename I, typename C = void> using ref = I&;
template <typename I, typename C = void> using ptr = I*;
#else // MAKESHIFT_INTELLISENSE_PARSER
template <typename I, typename C = void> using ref = typename makeshift::detail::substitute<I, C>::type&;
template <typename I, typename C = void> using ptr = typename makeshift::detail::substitute<I, C>::type*;
#endif // MAKESHIFT_INTELLISENSE_PARSER


struct Foo { void f(void); };

template <typename C>
    struct Bar
{
    ref<Foo, C> foo;
    void g(void)
    {
        
    }
};

    /*

        What do we need?
        - obj<I> { ptr }
        - obj<I> movable { ptr }
        - obj<I> copyable { ptr }
        - obj<I,C> { obj }
        - obj<I,C> movable { obj }
        - obj<I,C> copyable { obj }
        - unique<I>

    */

} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_CONTRACT_HPP_
