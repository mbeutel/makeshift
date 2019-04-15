
#ifndef INCLUDED_MAKESHIFT_ITERATOR_HPP_
#define INCLUDED_MAKESHIFT_ITERATOR_HPP_


#include <iterator>    // for input_iterator_tag, output_iterator_tag
#include <utility>     // for forward<>()
#include <type_traits> // for decay<>, declval<>()

#include <makeshift/constval.hpp> // for is_constval<>
#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/iterator.hpp>


namespace makeshift
{


template <typename FuncT>
    class callback_input_iterator
{
public:
    using difference_type = void;
    using value_type = void;
    using pointer = void;
    using reference = decltype(std::declval<FuncT>()());
    using iterator_category = std::input_iterator_tag;

private:
    FuncT func_;

public:
    constexpr explicit callback_input_iterator(FuncT _func)
        : func_{ std::move(_func) }
    {
    }

    constexpr reference operator *(void)
    {
        return func_();
    }

        // We don't bother to check if dereference--assign statements and increment statements are sequenced alternatingly as required.
    constexpr callback_input_iterator& operator ++(void)
    {
        return *this;
    }
    constexpr callback_input_iterator& operator ++(int)
    {
        return *this;
    }
};
template <typename FuncT>
    callback_input_iterator(FuncT&&) -> callback_input_iterator<std::decay_t<FuncT>>;

template <typename FuncT>
    MAKESHIFT_NODISCARD constexpr callback_input_iterator<std::decay_t<FuncT>> make_callback_input_iterator(FuncT&& func)
{
	return callback_input_iterator<std::decay_t<FuncT>>{ std::forward<FuncT>(func) };
}


template <typename FuncT>
    class callback_output_iterator
{
public:
    using difference_type = void;
    using value_type = void;
    using pointer = void;
    class reference
    {
        friend callback_output_iterator<FuncT>;

    private:
        callback_output_iterator& self_;

        constexpr reference(callback_output_iterator& _self)
            : self_{ _self }
        {
        }

    public:
        template <typename T>
            constexpr void operator =(T&& value) &&
        {
            self_.func_(std::forward<T>(value));
        }
    };
    using iterator_category = std::output_iterator_tag;

private:
    FuncT func_;

public:
    constexpr explicit callback_output_iterator(FuncT _func)
        : func_{ std::move(_func) }
    {
    }

    constexpr reference operator *(void)
    {
        return { *this };
    }

        // We don't bother to check if dereference--assign statements and increment statements are sequenced alternatingly as required.
    constexpr callback_output_iterator& operator ++(void)
    {
        return *this;
    }
    constexpr callback_output_iterator& operator ++(int)
    {
        return *this;
    }
};
template <typename FuncT>
    callback_output_iterator(FuncT&&) -> callback_output_iterator<std::decay_t<FuncT>>;

template <typename FuncT>
    MAKESHIFT_NODISCARD constexpr callback_output_iterator<std::decay_t<FuncT>> make_callback_output_iterator(FuncT&& func)
{
	return callback_output_iterator<std::decay_t<FuncT>>{ std::forward<FuncT>(func) };
}


    //ᅟ
    // Represents a pair of iterators.
    //
template <typename It, typename EndIt = It>
    struct range : makeshift::detail::range_base_<range<It, EndIt>>::type
{
    using iterator = It;
    using end_iterator = EndIt;

    It first;
    EndIt last;

    constexpr range(It _first, EndIt _last)
        : first(std::move(_first)), last(std::move(_last))
    {
    }
    template <std::size_t Size>
        constexpr range(makeshift::detail::fixed_random_access_range<It, Size> _rhs)
            : first(std::move(_rhs.first)), last(first + Size)
    {
    }

    MAKESHIFT_NODISCARD constexpr const It& begin(void) const noexcept { return first; }
    MAKESHIFT_NODISCARD constexpr const EndIt& end(void) const noexcept { return last; }
};

    //ᅟ
    // Construct a range from a pair of iterators.
    //
template <typename It, typename EndIt,
          typename = std::enable_if_t<!std::is_integral<EndIt>::value && !is_constval_v<EndIt>>>
    MAKESHIFT_NODISCARD range<It, EndIt> make_range(It first, EndIt last)
{
    return { std::move(first), std::move(last) };
}

    //ᅟ
    // Construct a range from an iterator and an extent.
    //
template <typename It, typename ExtentC>
    MAKESHIFT_NODISCARD auto make_range(It start, ExtentC extentC)
{
    return makeshift::detail::range_by_extent_<It, ExtentC, range>::type{ start, start + makeshift::constval_extract(extentC) };
}

    //ᅟ
    // Construct a range from another range (e.g. a container).
    // TODO: why would we want to do that? to take range args by value?
    //
template <typename ContainerT>
    MAKESHIFT_NODISCARD auto make_range(ContainerT&& container)
{
    return typename makeshift::detail::range_by_container_<std::remove_reference_t<ContainerT>, range>::type{ std::forward<ContainerT>(container).begin(), std::forward<ContainerT>(container).end() };
}


} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_ITERATOR_HPP_
