
#ifndef INCLUDED_MAKESHIFT_ITERATOR_HPP_
#define INCLUDED_MAKESHIFT_ITERATOR_HPP_


#include <iterator>    // for input_iterator_tag, output_iterator_tag
#include <utility>     // for forward<>()
#include <type_traits> // for decay<>, declval<>()

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

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
	return { std::forward<FuncT>(func) };
}


} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_ITERATOR_HPP_
