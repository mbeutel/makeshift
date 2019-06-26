
#ifndef INCLUDED_MAKESHIFT_ITERATOR_HPP_
#define INCLUDED_MAKESHIFT_ITERATOR_HPP_


#include <iterator>    // for input_iterator_tag, output_iterator_tag
#include <utility>     // for move(), forward<>()
#include <type_traits> // for decay<>, declval<>()

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD


namespace makeshift
{


template <typename F>
    class callback_input_iterator
{
public:
    using difference_type = void;
    using value_type = void;
    using pointer = void;
    using reference = decltype(std::declval<F>()());
    using iterator_category = std::input_iterator_tag;

private:
    F func_;

public:
    constexpr explicit callback_input_iterator(F _func)
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
template <typename F>
    callback_input_iterator(F&&) -> callback_input_iterator<std::decay_t<F>>;

template <typename F>
    MAKESHIFT_NODISCARD constexpr callback_input_iterator<std::decay_t<F>> make_callback_input_iterator(F&& func)
{
	return callback_input_iterator<std::decay_t<F>>{ std::forward<F>(func) };
}


template <typename F>
    class callback_output_iterator
{
public:
    using difference_type = void;
    using value_type = void;
    using pointer = void;
    class reference
    {
        friend callback_output_iterator<F>;

    private:
        callback_output_iterator& self_;

        constexpr reference(callback_output_iterator& _self) noexcept
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
    F func_;

public:
    constexpr explicit callback_output_iterator(F _func)
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
template <typename F>
    callback_output_iterator(F&&) -> callback_output_iterator<std::decay_t<F>>;

template <typename F>
    MAKESHIFT_NODISCARD constexpr callback_output_iterator<std::decay_t<F>> make_callback_output_iterator(F&& func)
{
	return callback_output_iterator<std::decay_t<F>>{ std::forward<F>(func) };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ITERATOR_HPP_
