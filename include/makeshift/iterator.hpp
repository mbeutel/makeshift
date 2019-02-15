
#ifndef INCLUDED_MAKESHIFT_ITERATOR_HPP_
#define INCLUDED_MAKESHIFT_ITERATOR_HPP_


#include <iterator>    // for output_iterator_tag
#include <type_traits> // for decay<>


namespace makeshift
{


template <typename FuncT>
    class callback_iterator
{
public:
    using difference_type = void;
    using value_type = void;
    using pointer = void;
    class reference
    {
        friend callback_iterator<FuncT>;

    private:
        callback_iterator& self_;

        constexpr reference(callback_iterator& _self)
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
    constexpr explicit callback_iterator(FuncT _func)
        : func_{ std::move(_func) }
    {
    }

    constexpr reference operator *(void)
    {
        return { *this };
    }

        // We don't bother to check if dereference--assign statements and increment statements are sequenced alternatingly as required.
    constexpr callback_iterator& operator ++(void)
    {
        return *this;
    }
    constexpr callback_iterator& operator ++(int)
    {
        return *this;
    }
};
template <typename FuncT>
    callback_iterator(FuncT&&) -> callback_iterator<std::decay_t<FuncT>>;


} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_ITERATOR_HPP_
