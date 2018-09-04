
#ifndef INCLUDED_MAKESHIFT_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_TUPLE_HPP_


#include <cstddef>     // for size_t
#include <utility>     // for move(), forward<>(), tuple_element<>, tuple_size<>, get<>
#include <type_traits> // for decay<>, integral_constant<>, index_sequence<>

#include <makeshift/type_traits.hpp> // for can_apply<>, none


namespace makeshift
{

namespace detail
{


template <typename T> using is_tuple_like_r = std::integral_constant<std::size_t, std::tuple_size<T>::value>;


} // namespace detail


inline namespace types
{


    //ᅟ
    // Determines whether a type has a tuple-like interface (i.e. whether `std::tuple_size<T>::value` is well-formed).
    //
template <typename T> struct is_tuple_like : can_apply<makeshift::detail::is_tuple_like_r, T> { };

    //ᅟ
    // Determines whether a type has a tuple-like interface (i.e. whether `std::tuple_size<T>::value` is well-formed).
    //
template <typename T> constexpr bool is_tuple_like_v = is_tuple_like<T>::value;


    //ᅟ
    // Pass `tuple_index` to `tuple_foreach()` or `tuple_map()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    //ᅟ
    //ᅟ    auto print_tuple = tuple_foreach([](auto element, std::size_t idx) { std::cout << idx << ": " << element << '\n'; });
    //ᅟ    auto tuple = std::make_tuple(42, 1.41421);
    //ᅟ    print_tuple(tuple, tuple_index); // prints "0: 42\n1: 1.41421"
    //
struct tuple_index_t { };

    //ᅟ
    // Pass `tuple_index` to `tuple_foreach()` or `tuple_map()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    //ᅟ
    //ᅟ    auto print_tuple = tuple_foreach([](auto element, std::size_t idx) { std::cout << idx << ": " << element << '\n'; });
    //ᅟ    auto tuple = std::make_tuple(42, 1.41421);
    //ᅟ    print_tuple(tuple, tuple_index); // prints "0: 42\n1: 1.41421"
    //
static constexpr tuple_index_t tuple_index { };


} // inline namespace types


namespace detail
{


template <typename DerivedT>
    struct stream_base
{
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        friend constexpr auto operator |(TupleT&& tuple, const DerivedT& self)
    {
        return self(std::forward<TupleT>(tuple));
    }
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        friend constexpr auto operator |(TupleT&& tuple, DerivedT&& self)
    {
        return std::move(self)(std::forward<TupleT>(tuple));
    }
};


template <typename TupleT> struct args_sequence_of;
template <template <typename...> class TupleT, typename... ArgsT> struct args_sequence_of<TupleT<ArgsT...>> { using type = type_sequence<ArgsT...>; };
template <typename TupleT> using args_sequence_of_t = typename args_sequence_of<TupleT>::type;


enum class tuple_arg_kind_t
{
    scalar,
    tuple,
    index
};
template <bool IsTuple, typename T, std::size_t Default> struct tuple_size_or_default_;
template <typename T, std::size_t Default> struct tuple_size_or_default_<false, T, Default> : std::integral_constant<std::size_t, Default> { };
template <typename T, std::size_t Default> struct tuple_size_or_default_<true, T, Default> : std::tuple_size<T> { };
template <typename T, std::size_t Default = std::size_t(-1)> using tuple_size_or_default = tuple_size_or_default_<is_tuple_like_v<T>, T, Default>;
template <typename T, std::size_t Default = std::size_t(-1)> static constexpr std::size_t tuple_size_or_default_v = tuple_size_or_default<T, Default>::value;

template <bool IsTuple, typename T> struct tuple_arg_kind_of_;
template <typename T> struct tuple_arg_kind_of_<false, T> : std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::scalar> { };
template <> struct tuple_arg_kind_of_<false, tuple_index_t> : std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::index> { };
template <typename T> struct tuple_arg_kind_of_<true, T> : std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::tuple> { };
template <typename T> using tuple_arg_kind_of = tuple_arg_kind_of_<is_tuple_like_v<T>, T>;
template <typename T> static constexpr tuple_arg_kind_t tuple_arg_kind_of_v = tuple_arg_kind_of<T>::value;

template <std::size_t I, typename T>
    constexpr decltype(auto) get_entry(std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::scalar>, T&& arg) noexcept
{
    return std::forward<T>(arg);
}
template <std::size_t I, typename TupleT>
    constexpr decltype(auto) get_entry(std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::tuple>, TupleT&& arg) noexcept
{
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    return get<I>(std::forward<TupleT>(arg));
}
template <std::size_t I, typename T>
    constexpr std::integral_constant<std::size_t, I> get_entry(std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::index>, T&&) noexcept
{
    return { };
}

struct tuple_size_aggregator
{
    std::size_t value;
    bool all_equal;

    //constexpr tuple_size_aggregator(void) noexcept : value(std::size_t(-1)), all_equal(true) { }
    //constexpr tuple_size_aggregator(std::size_t _value, bool _all_equal) noexcept : value(_value), all_equal(_all_equal) { }

    friend constexpr tuple_size_aggregator operator +(tuple_size_aggregator lhs, std::size_t rhs_size_or_default) noexcept
    {
        if (!lhs.all_equal || rhs_size_or_default == std::size_t(-1))
            return std::move(lhs);
        else if (lhs.value == std::size_t(-1))
            return { rhs_size_or_default, true };
        return { lhs.value, lhs.value == rhs_size_or_default };
    }
};

template <bool IsMap, typename F>
    struct tuple_foreach_t : F, stream_base<tuple_foreach_t<IsMap, F>>
{
    constexpr tuple_foreach_t(F func) : F(std::move(func)) { }

private:
    template <std::size_t I, typename... Ts>
        constexpr auto invoke_tuple_element(Ts&&... args) const
    {
        return F::operator ()(get_entry<I>(tuple_arg_kind_of<std::decay_t<Ts>> { }, std::forward<Ts>(args))...);
    }
    template <std::size_t... Is, typename... Ts>
        constexpr auto invoke_tuple(std::true_type /*isMap*/, std::index_sequence<Is...>, Ts&&... args) const
    {
        return std::make_tuple(invoke_tuple_element<Is>(std::forward<Ts>(args)...)...);
    }
    template <std::size_t... Is, typename... Ts>
        constexpr void invoke_tuple(std::false_type /*isMap*/, std::index_sequence<Is...>, Ts&&... args) const
    {
        (invoke_tuple_element<Is>(std::forward<Ts>(args)...), ...);
    }
    template <typename... Ts>
        constexpr auto invoke(std::true_type /*scalar*/, Ts&&... args) const
    {
        return F::operator ()(get_entry<0>(tuple_arg_kind_of<std::decay_t<Ts>> { }, std::forward<Ts>(args))...);
    }
    template <typename... Ts>
        constexpr auto invoke(std::false_type /*scalar*/, Ts&&... args) const
    {
        //constexpr tuple_size_aggregator term;
        constexpr tuple_size_aggregator term{ std::size_t(-1), true };
        constexpr tuple_size_aggregator numElements = (term + ... + tuple_size_or_default_v<std::decay_t<Ts>>);
        static_assert(numElements.all_equal, "all tuple arguments must have the same size");
        return invoke_tuple(std::integral_constant<bool, IsMap>{ }, std::make_index_sequence<numElements.value>{ }, std::forward<Ts>(args)...);
    }
    
public:
    template <typename... Ts>
        constexpr auto operator ()(Ts&&... args) const
    {
        constexpr bool anyTuple = ((is_tuple_like_v<std::decay_t<Ts>>) || ...);
        return invoke(std::integral_constant<bool, !anyTuple>{ }, std::forward<Ts>(args)...);
    }
};

template <typename F, typename T>
    struct accumulator_wrapper
{
private:
    F& func_;
    T value_;
public:
    constexpr accumulator_wrapper(F& _func, T _value)
        : func_(_func), value_(std::move(_value))
    {
    }
    F& func(void) const noexcept { return func_; }
    constexpr T&& get(void) && noexcept { return std::move(value_); }
};
template <typename F, typename T>
    accumulator_wrapper<F, std::decay_t<T>> make_accumulator_wrapper(F& func, T&& value)
{
    return { func, std::forward<T>(value) };
}
template <typename F, typename T, typename U>
    constexpr auto operator +(accumulator_wrapper<F, T>&& lhs, U&& rhs)
{
    return make_accumulator_wrapper(lhs.func(), lhs.func()(std::move(lhs).get(), std::forward<U>(rhs)));
}

template <typename SelectedIs, typename Is, typename TupleT, template <typename> class PredT> struct select_indices_0_;
template <typename SelectedIs, std::size_t NextI, bool TakeNextI> struct select_indices_1_;
template <std::size_t... SelectedIs, std::size_t NextI> struct select_indices_1_<std::index_sequence<SelectedIs...>, NextI, true> { using type = std::index_sequence<SelectedIs..., NextI>; };
template <std::size_t... SelectedIs, std::size_t NextI> struct select_indices_1_<std::index_sequence<SelectedIs...>, NextI, false> { using type = std::index_sequence<SelectedIs...>; };
template <std::size_t... SelectedIs, std::size_t NextI, std::size_t... Is, typename TupleT, template <typename> class PredT>
    struct select_indices_0_<std::index_sequence<SelectedIs...>, std::index_sequence<NextI, Is...>, TupleT, PredT>
        : select_indices_0_<typename select_indices_1_<std::index_sequence<SelectedIs...>, NextI, PredT<std::tuple_element_t<NextI, TupleT>>::value>::type, std::index_sequence<Is...>, TupleT, PredT>
{
};
template <std::size_t... SelectedIs, typename TupleT, template <typename...> class PredT>
    struct select_indices_0_<std::index_sequence<SelectedIs...>, std::index_sequence<>, TupleT, PredT>
{
    using type = std::index_sequence<SelectedIs...>;
};
template <typename TupleT, template <typename...> class PredT>
    using select_indices = typename select_indices_0_<std::index_sequence<>, std::make_index_sequence<std::tuple_size<TupleT>::value>, TupleT, PredT>::type;


template <typename TupleT, std::size_t... Is>
    constexpr auto select_tuple_indices(TupleT&& tuple, std::index_sequence<Is...>)
{
    (void) tuple;
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    return std::make_tuple(get<Is>(std::forward<TupleT>(tuple))...);
}

template <template <typename> class PredT>
    struct tuple_filter_t : stream_base<tuple_filter_t<PredT>>
{
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        using SelectedIs = select_indices<std::decay_t<TupleT>, PredT>;
        return select_tuple_indices(std::forward<TupleT>(tuple), SelectedIs{ });
    }
};

template <typename F>
    struct tuple_reduce_t : F
{
    constexpr tuple_reduce_t(F func) : F(std::move(func)) { }

private:
    template <std::size_t... Is, typename ValT, typename TupleT>
        constexpr auto invoke(std::index_sequence<Is...>, ValT&& initialValue, TupleT&& tuple) const
    {
        (void) tuple;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        auto wrappedInitialValue = make_accumulator_wrapper(static_cast<const F&>(*this), std::forward<ValT>(initialValue));
        return (std::move(wrappedInitialValue) + ... + get<Is>(std::forward<TupleT>(tuple))).get();
    }
    
public:
    template <typename ValT, typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(ValT&& initialValue, TupleT&& tuple) const
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<ValT>(initialValue), std::forward<TupleT>(tuple));
    }
};

template <typename ValT, typename F>
    struct tuple_bound_reduce_t : F, stream_base<tuple_bound_reduce_t<ValT, F>>
{
private:
    ValT initialValue_;

    template <std::size_t... Is, typename TupleT>
        constexpr auto invoke(std::index_sequence<Is...>, TupleT&& tuple) const &
    {
        (void) tuple;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        auto wrappedInitialValue = make_accumulator_wrapper(static_cast<const F&>(*this), initialValue_);
        return (std::move(wrappedInitialValue) + ... + get<Is>(std::forward<TupleT>(tuple))).get();
    }
    template <std::size_t... Is, typename TupleT>
        constexpr auto invoke(std::index_sequence<Is...>, TupleT&& tuple) &&
    {
        (void) tuple;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        auto wrappedInitialValue = make_accumulator_wrapper(static_cast<const F&>(*this), std::move(initialValue_));
        return (std::move(wrappedInitialValue) + ... + get<Is>(std::forward<TupleT>(tuple))).get();
    }
    
public:
    constexpr tuple_bound_reduce_t(ValT&& _initialValue, F func) : F(std::move(func)), initialValue_(std::move(_initialValue)) { }

    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const &
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple));
    }
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) &&
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple));
    }
};

template <typename T, typename DefaultT>
    struct get_or_default_t : stream_base<get_or_default_t<T, DefaultT>>
{
private:
    DefaultT defaultValue_;

    template <std::size_t... Is, typename TupleT, typename LDefaultT>
        static constexpr auto invoke(std::index_sequence<Is...>, TupleT&& tuple, LDefaultT&& defaultValue)
    {
        (void) tuple;
        (void) defaultValue;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        constexpr bool canGet = ((std::is_same<T, std::tuple_element_t<Is, std::decay_t<TupleT>>>::value) || ...);
        if constexpr (canGet)
            return get<T>(std::forward<TupleT>(tuple));
        else
            return std::forward<LDefaultT>(defaultValue);
    }
public:
    constexpr get_or_default_t(DefaultT&& _defaultValue) : defaultValue_(std::move(_defaultValue)) { }

    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const &
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple), defaultValue_);
    }
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) &&
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple), std::move(defaultValue_));
    }
};

template <typename T>
    struct get_t : stream_base<get_t<T>>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        return get<T>(std::forward<TupleT>(tuple));
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::forward<TupleT>(tuple));
    }
};
template <std::size_t I>
    struct get_by_index_t : stream_base<get_by_index_t<I>>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        return get<I>(std::forward<TupleT>(tuple));
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::forward<TupleT>(tuple));
    }
};

template <typename DefaultT>
    struct single_or_default_t : stream_base<single_or_default_t<DefaultT>>
{
private:
    DefaultT defaultValue_;

    template <typename TupleT, typename LDefaultT>
        static constexpr auto invoke(TupleT&& tuple, LDefaultT&& defaultValue)
    {
        (void) tuple;
        (void) defaultValue;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        constexpr std::size_t tupleSize = std::tuple_size<std::decay_t<TupleT>>::value;
        static_assert(tupleSize <= 1, "tuple may not have more than one entry");
        if constexpr (tupleSize == 1)
            return get<0>(std::forward<TupleT>(tuple));
        else
            return std::forward<LDefaultT>(defaultValue);
    }
public:
    constexpr single_or_default_t(DefaultT&& _defaultValue) : defaultValue_(_defaultValue) { }

    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const &
    {
        return invoke(std::forward<TupleT>(tuple), defaultValue_);
    }
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) &&
    {
        return invoke(std::forward<TupleT>(tuple), std::move(defaultValue_));
    }
};

struct single_t : stream_base<single_t>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        constexpr std::size_t tupleSize = std::tuple_size<std::decay_t<TupleT>>::value;
        static_assert(tupleSize == 1, "tuple must have exactly one entry");
        return get<0>(std::forward<TupleT>(tuple));
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::forward<TupleT>(tuple));
    }
};

struct tuple_cat_t : stream_base<tuple_cat_t>
{
private:
    template <std::size_t... Is, typename TupleT>
        static constexpr auto invoke(std::index_sequence<Is...>, TupleT&& tuple)
    {
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        return std::tuple_cat(get<Is>(std::forward<TupleT>(tuple))...);
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple));
    }
};

    /*
        which functions would be worth implementing?

        Sorting: ??
        
        Set operations:
        - Distinct: ?
        - Except: ?
        - Intersect: ?
        - Union: ?

        Quantifiers:
        - All/Any: ?
        - Contains: ?

        Projections:
        - SelectMany: ?

        Partitioning: ?

        Join: ?

        Group: ?

        Generation:
        - DefaultIfEmpty: ?
        - Range: ?
        - Repeat: ?

        Element:
        - ElementAtOrDefault: ?
        - First/Last: ?
        - FirstOrDefault/LastOrDefault: ?
        
        Concatenation: ?

    */


} // namespace detail


inline namespace types
{


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns a procedure which can be called
    // with tuples in some or all arguments.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(1, 2.3f);
    //ᅟ    numbers | tuple_foreach([](auto elem) { std::cout << elem << '\n'; }); // prints "1\n2.3\n"
    //
template <typename F>
    constexpr makeshift::detail::tuple_foreach_t<false, std::decay_t<F>>
    tuple_foreach(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a tuple and a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and calls the procedure for every
    // element in the tuple.
    //ᅟ
    //ᅟ    tuple_foreach(
    //ᅟ        std::make_tuple(1, 2.3f),
    //ᅟ        [](auto elem) { std::cout << elem << '\n'; }); }
    //ᅟ    ); // prints "1\n2.3\n"
    //
template <typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr void
    tuple_foreach(TupleT&& tuple, F&& func)
{
    tuple_foreach(std::forward<F>(func))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor that maps a tuple to a new tuple which contains only the values for which the given type predicate is true.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(1, 2, 3u);
    //ᅟ    auto signedNumbers = numbers
    //ᅟ        | tuple_filter<std::is_signed>(); // returns (1, 2)
    //
template <template <typename> class PredT>
    constexpr makeshift::detail::tuple_filter_t<PredT>
    tuple_filter(void) noexcept
{
    return { };
}


    //ᅟ
    // Maps a tuple to a new tuple which contains only the values for which the given type predicate is true.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(1, 2, 3u);
    //ᅟ    auto signedNumbers = tuple_filter<std::is_signed>(numbers); // returns (1, 2)
    //
template <template <typename> class PredT, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_filter(TupleT&& tuple) noexcept
{
    return tuple_filter<PredT>()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a scalar function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function which can be called 
    // with tuples in some or all arguments, and whose result will be a tuple of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3.0f);
    //ᅟ    auto squaredNumbers = numbers
    //ᅟ        | tuple_map([](auto x) { return x*x; }); // returns (4, 9.0f)
    //
template <typename F>
    constexpr makeshift::detail::tuple_foreach_t<true, std::decay_t<F>>
    tuple_map(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a tuple and a scalar function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a tuple of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3.0f);
    //ᅟ    auto squaredNumbers = tuple_map(numbers, [](auto x) { return x*x; }); // returns (4, 9.0f)
    //
template <typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_map(TupleT&& tuple, F&& func)
{
    return tuple_map(std::forward<F>(func))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function which reduces
    // an initial value and a tuple to a scalar using the accumulator function.
    //ᅟ
    //ᅟ    auto sumTuple = tuple_reduce(std::plus<int>{ });
    //ᅟ    auto numbers = std::make_tuple(2, 3u);
    //ᅟ    int sum = sumTuple(0, numbers); // returns 5
    //
template <typename F>
    constexpr makeshift::detail::tuple_reduce_t<std::decay_t<F>>
    tuple_reduce(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns a function
    // which reduces a tuple to a scalar using the accumulator function.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3u);
    //ᅟ    int sum = numbers
    //ᅟ        | tuple_reduce(0, std::plus<int>{ }); // returns 5
    //
template <typename ValT, typename F>
    constexpr makeshift::detail::tuple_bound_reduce_t<std::decay_t<ValT>, std::decay_t<F>>
    tuple_reduce(ValT&& initialValue, F&& func)
{
    return { std::forward<ValT>(initialValue), std::forward<F>(func) };
}


    //ᅟ
    // Takes a tuple, an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns the
    // reduction of the tuple (i.e. the left fold).
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3u);
    //ᅟ    int sum = tuple_reduce(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_reduce(TupleT&& tuple, T&& initialValue, F&& func)
{
    return tuple_reduce(std::forward<F>(func))(std::forward<T>(initialValue), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type, or which returns the provided default value if the tuple does not contain
    // an element of the given type. The type of the default value does not need to match the desired element type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto str = tuple
    //ᅟ        | get_or_default<std::string>("bar"sv); // returns "bar"sv
    //
template <typename T, typename DefaultT,
          typename = std::enable_if_t<!is_tuple_like_v<std::decay_t<DefaultT>>>> // TODO: this is not optimal because we might have nested tuples...
    constexpr makeshift::detail::get_or_default_t<T, std::decay_t<DefaultT>>
    get_or_default(DefaultT&& defaultValue)
{
    return { std::forward<DefaultT>(defaultValue) };
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type, or which returns a default-constructed element if the tuple does not
    // contain an element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto str = tuple
    //ᅟ        | get_or_default<std::string>(); // returns ""s
    //
template <typename T>
    constexpr makeshift::detail::get_or_default_t<T, std::decay_t<T>>
    get_or_default(void)
{
    return {{ }};
}


    //ᅟ
    // Returns the tuple element of the given type, or the provided default value if the tuple does not contain an element of the given type.
    // The type of the default value does not need to match the desired element type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto str = get_or_default<std::string>(tuple, "bar"sv); // returns "bar"sv
    //
template <typename T, typename TupleT, typename DefaultT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    get_or_default(TupleT&& tuple, DefaultT&& defaultValue)
{
    return get_or_default<T>(std::forward<DefaultT>(defaultValue))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns the tuple element of the given type, or the provided default value if the tuple does not contain an element of the given type.
    // The type of the default value does not need to match the desired element type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto str = get_or_default<std::string>(tuple); // returns ""s
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr T
    get_or_default(TupleT&& tuple)
{
    return get_or_default<T>()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type, or `none` if the tuple does not contain an element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto str = tuple
    //ᅟ        | get_or_none<std::string>(); // returns none
    //
template <typename T>
    constexpr makeshift::detail::get_or_default_t<T, none_t>
    get_or_none(void)
{
    return {{ }};
}


    //ᅟ
    // Returns the tuple element of the given type, or `none` if the tuple does not contain an element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto str = get_or_none<std::string>(tuple); // returns none
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    get_or_none(TupleT&& tuple)
{
    return get_or_none<T>()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto elem = tuple
    //ᅟ        | get<int>(); // returns 42
    //
template <typename T>
    constexpr makeshift::detail::get_t<T>
    get(void)
{
    return { };
}


    //ᅟ
    // Returns a functor which retrieves the tuple element with the given index. Negative indices count from the end.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto elem = tuple
    //ᅟ        | get<0>(); // returns 42
    //
template <int I>
    constexpr makeshift::detail::get_by_index_t<I>
    get(void)
{
    return { };
}


    //ᅟ
    // Returns a functor which retrieves the single element in a tuple, or which returns the provided default value if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(12, 42);
    //ᅟ    auto elem = tuple
    //ᅟ        | single_or_default(0); // returns 0
    //
template <typename DefaultT>
    constexpr makeshift::detail::single_or_default_t<std::decay_t<DefaultT>>
    single_or_default(DefaultT&& defaultValue)
{
    return { std::forward<DefaultT>(defaultValue) };
}


    //ᅟ
    // Returns the single element in a tuple, or the provided default value if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(12, 42);
    //ᅟ    auto elem = single_or_default(tuple, 0); // returns 0
    //
template <typename TupleT, typename DefaultT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    single_or_default(TupleT&& tuple, DefaultT&& defaultValue)
{
    return single_or_default(std::forward<DefaultT>(defaultValue))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the single element in a tuple, or which returns `none` if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(12, 42);
    //ᅟ    auto elem = tuple
    //ᅟ        | single_or_none(); // returns none
    //
constexpr inline makeshift::detail::single_or_default_t<none_t>
single_or_none(void)
{
    return {{ }};
}


    //ᅟ
    // Returns the single element in a tuple, or `none` if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(12, 42);
    //ᅟ    auto elem = single_or_none(tuple); // returns 0
    //
template <typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    single_or_none(TupleT&& tuple)
{
    return single_or_none()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the single element in a tuple.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto elem = tuple
    //ᅟ        | single(); // returns 42
    //
constexpr inline makeshift::detail::single_t
single(void)
{
    return { };
}


    //ᅟ
    // Returns the single element in a tuple.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(42);
    //ᅟ    auto elem = single(tuple); // returns 42
    //
template <typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    single(TupleT&& tuple)
{
    return single()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Concatenates the tuples in a tuple of tuples.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(std::make_tuple(1), std::make_tuple(2));
    //ᅟ    auto flat_tuple = tuple
    //ᅟ        | tuple_cat(); // returns (1, 2)
    //
constexpr inline makeshift::detail::tuple_cat_t
tuple_cat(void)
{
    return { };
}


} // inline namespace types

} // namespace makeshift


#ifdef MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_
 #include <makeshift/detail/utility_keyword_tuple.hpp>
#endif // MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_


#endif // INCLUDED_MAKESHIFT_TUPLE_HPP_
