
#ifndef ASC_CPTOOLS_TUPLE_FOREACH_HPP_
#define ASC_CPTOOLS_TUPLE_FOREACH_HPP_


#include <tuple>
#include <cstddef> // for size_t
#include <utility> // for move(), forward<>()
#include <type_traits> // for decay<>, integral_constant<>, index_sequence<>


namespace asc::cptools
{

namespace detail
{

template <typename... Fs>
    struct overload_t;
template <typename F0, typename... Fs>
    struct overload_t<F0, Fs...> : F0, overload_t<Fs...>
{
    constexpr overload_t(F0&& f0, Fs&&... rest) : F0(std::move(f0)), overload_t<Fs...>(std::move(rest)...) { }
    using F0::operator();
    using overload_t<Fs...>::operator();
};
template <typename F0>
    struct overload_t<F0> : F0
{
    constexpr overload_t(F0&& f0) : F0(std::move(f0)) { }
    using F0::operator();
};

} // namespace detail

    // Use this function to define a function object with multiple overloads given as individual function objects.
    // 
    //     auto f = overload(
    //         [](int i) { return "integer"; },
    //         [](float f) { return "float"; }
    //     );
    //     std::cout << f(42) << ", " << f(1.41421f) << std::endl;
    //
template <typename... Fs>
    constexpr auto overload(Fs&&... fs)
{
    return asc::cptools::detail::overload_t<typename std::decay<Fs>::type...>(std::forward<Fs>(fs)...);
}


struct tuple_index_t { };
static constexpr tuple_index_t tuple_index { };


namespace detail
{

enum class tuple_arg_kind_t
{
    scalar,
    tuple,
    index
};
template <typename T, std::size_t Default = std::size_t(-1)> struct tuple_size_or_default_t : std::integral_constant<std::size_t, Default> { };
template <typename... Ts, std::size_t Default> struct tuple_size_or_default_t<std::tuple<Ts...>, Default> : std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <typename T, std::size_t Default = std::size_t(-1)> static constexpr std::size_t tuple_size_or_default = tuple_size_or_default_t<T, Default>::value;

template <typename T> struct tuple_arg_kind_of_t : std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::scalar> { };
template <typename... Ts> struct tuple_arg_kind_of_t<std::tuple<Ts...>> : std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::tuple> { };
template <> struct tuple_arg_kind_of_t<tuple_index_t> : std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::index> { };
template <typename T> static constexpr tuple_arg_kind_t tuple_arg_kind_of = tuple_arg_kind_of_t<T>::value;

template <std::size_t I, typename T>
    constexpr decltype(auto) get_entry(std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::scalar>, T&& arg) noexcept
{
    return std::forward<T>(arg);
}
template <std::size_t I, typename TupleT>
    constexpr decltype(auto) get_entry(std::integral_constant<tuple_arg_kind_t, tuple_arg_kind_t::tuple>, TupleT&& arg) noexcept
{
    return std::get<I>(std::forward<TupleT>(arg));
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

    constexpr tuple_size_aggregator(void) noexcept : value(std::size_t(-1)), all_equal(true) { }
    constexpr tuple_size_aggregator(std::size_t _value, bool _all_equal) noexcept : value(_value), all_equal(_all_equal) { }

    friend constexpr tuple_size_aggregator operator +(tuple_size_aggregator&& lhs, std::size_t rhs_size_or_default) noexcept
    {
        if (!lhs.all_equal || rhs_size_or_default == std::size_t(-1))
            return std::move(lhs);
        else if (lhs.value == std::size_t(-1))
            return tuple_size_aggregator(rhs_size_or_default, true);
        return tuple_size_aggregator(lhs.value, lhs.value == rhs_size_or_default);
    }
};

template <bool IsMap, typename FuncT>
    struct tuple_foreach_t : FuncT
{
    constexpr tuple_foreach_t(FuncT func) : FuncT(std::move(func)) { }

private:
    template <std::size_t I, typename... Ts>
        constexpr auto invoke_tuple_element(Ts&&... args) const
    {
        return FuncT::operator ()(get_entry<I>(tuple_arg_kind_of_t<typename std::decay<Ts>::type> { }, std::forward<Ts>(args))...);
    }
    template <std::size_t... Is, typename... Ts>
        constexpr auto invoke_tuple(std::true_type /*isMap*/, std::index_sequence<Is...>, Ts&&... args) const
    {
        return std::make_tuple(invoke_tuple_element<Is>(std::forward<Ts>(args)...)...);
    }
    template <std::size_t... Is, typename... Ts>
        constexpr auto invoke_tuple(std::false_type /*isMap*/, std::index_sequence<Is...>, Ts&&... args) const
    {
        return (invoke_tuple_element<Is>(std::forward<Ts>(args)...), ...);
    }
    template <typename... Ts>
        constexpr auto invoke(std::true_type /*scalar*/, Ts&&... args) const
    {
        return FuncT::operator ()(get_entry<0>(tuple_arg_kind_of_t<typename std::decay<Ts>::type> { }, std::forward<Ts>(args))...);
    }
    template <typename... Ts>
        constexpr auto invoke(std::false_type /*scalar*/, Ts&&... args) const
    {
        constexpr tuple_size_aggregator numElements = (tuple_size_aggregator() + ... + tuple_size_or_default<typename std::decay<Ts>::type>);
        static_assert(numElements.all_equal, "all tuple arguments must have the same size");
        return invoke_tuple(std::integral_constant<bool, IsMap>{ }, std::make_index_sequence<numElements.value>{ }, std::forward<Ts>(args)...);
    }
    
public:
    template <typename... Ts>
        constexpr auto operator ()(Ts&&... args) const
    {
        constexpr bool anyTuple = ((tuple_arg_kind_of<typename std::decay<Ts>::type> == tuple_arg_kind_t::tuple) || ...);
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
    accumulator_wrapper<F, typename std::decay<T>::type> make_accumulator_wrapper(F& func, T&& value)
{
    return { func, std::forward<T>(value) };
}
template <typename F, typename T, typename U>
    constexpr auto operator +(accumulator_wrapper<F, T>&& lhs, U&& rhs)
{
    return make_accumulator_wrapper(lhs.func(), lhs.func()(std::move(lhs).get(), std::forward<U>(rhs)));
}

template <typename FuncT>
    struct tuple_reduce_t : FuncT
{
    constexpr tuple_reduce_t(FuncT func) : FuncT(std::move(func)) { }

private:
    template <std::size_t... Is, typename ValT, typename TupleT>
        constexpr auto invoke(std::index_sequence<Is...>, ValT&& initialValue, TupleT&& tuple) const
    {
        auto wrappedInitialValue = make_accumulator_wrapper(static_cast<const FuncT&>(*this), std::forward<ValT>(initialValue));
        return (std::move(wrappedInitialValue) + ... + std::get<Is>(std::forward<TupleT>(tuple))).get();
    }
    
public:
    template <typename ValT, typename TupleT,
              typename = typename std::enable_if<tuple_arg_kind_of<typename std::decay<TupleT>::type> == tuple_arg_kind_t::tuple>::type>
        constexpr auto operator ()(ValT&& initialValue, TupleT&& tuple) const
    {
        return invoke(std::make_index_sequence<std::tuple_size<typename std::decay<TupleT>::type>::value>{ }, std::forward<ValT>(initialValue), std::forward<TupleT>(tuple));
    }
};

} // namespace detail

    // Higher-order function that takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns a procedure
    // which can be called with tuples in some or all arguments.
    //
    //     auto f = tuple_foreach([](auto elem) { std::cout << elem << '\n'; });
    //     f(std::make_tuple(1, 2.3f)); // prints "1\n2.3\n"
    //
template <typename FuncT>
    constexpr asc::cptools::detail::tuple_foreach_t<false, typename std::decay<FuncT>::type>
    tuple_foreach(FuncT&& func)
{
    return { std::forward<FuncT>(func) };
}

    // Higher-order function that takes a scalar function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function
    // which can be called with tuples in some or all arguments, and whose result will be a tuple of the results of the function of the tuple elements.
    //
    //     auto square = tuple_fmap([](auto x) { return x*x; });
    //     auto numbers = std::make_tuple(2, 3.0f);
    //     auto squaredNumbers = square(numbers); // returns (4, 9.0f)
    //
template <typename FuncT>
    constexpr asc::cptools::detail::tuple_foreach_t<true, typename std::decay<FuncT>::type>
    tuple_fmap(FuncT&& func)
{
    return { std::forward<FuncT>(func) };
}

    // Higher-order function that takes a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple return type) and returns
    // a function which reduces an initial value and a tuple to a scalar using the accumulator function.
    //
    //     auto sumTuple = ac::tuple_freduce(std::plus<int>{ });
    //     auto numbers = std::make_tuple(2, 3u);
    //     int sum = sumTuple(0, numbers); // returns 5
    //
template <typename FuncT>
    constexpr asc::cptools::detail::tuple_reduce_t<typename std::decay<FuncT>::type>
    tuple_freduce(FuncT&& func)
{
    return { std::forward<FuncT>(func) };
}

} // namespace asc::cptools

#endif // ASC_CPTOOLS_TUPLE_FOREACH_HPP_
