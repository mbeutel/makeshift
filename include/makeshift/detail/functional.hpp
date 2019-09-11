
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_HPP_


#include <utility> // for forward<>()


namespace makeshift
{

namespace detail
{


template <typename FRefT>
    class y_combinator_func_ref
{
private:
    FRefT func_;

public:
    constexpr explicit y_combinator_func_ref(FRefT _func) noexcept
        : func_(_func)
    {
    }
    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args) const
    {
        // Note that the lambda function must explicitly declare a return type. An error message about deduced result types will lead you here if you forgot.

        return func_(*this, std::forward<ArgsT>(args)...);
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_HPP_
