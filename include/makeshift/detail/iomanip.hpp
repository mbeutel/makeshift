
#ifndef INCLUDED_MAKESHIFT_DETAIL_IOMANIP_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_IOMANIP_HPP_


#include <ios>
#include <tuple>
#include <iosfwd>
#include <string>
#include <string_view>
#include <type_traits>  // for remove_reference<>, remove_cv<>

#include <makeshift/detail/serialize.hpp>


namespace makeshift {

namespace detail {


template <typename ToStreamFuncT, typename FromStreamFuncT = void>
struct manipulator : ToStreamFuncT, FromStreamFuncT
{
    template <typename... ArgsT>
    struct bound_manipulator
    {
        manipulator const& manip_;
        std::tuple<ArgsT...> args_;

        friend std::ostream&
        operator <<(std::ostream& stream, bound_manipulator&& m)
        {
            std::apply(
                [&manip = m.manip_, &stream]
                (auto&&... args)
                {
                    return static_cast<ToStreamFuncT const&>(manip)(stream, std::forward<decltype(args)>(args)...);
                }, m.args_);
            return stream;
        }
        friend std::istream&
        operator >>(std::istream& stream, bound_manipulator&& m)
        {
            std::apply(
                [&manip = m.manip_, &stream]
                (auto&&... args)
                {
                    return static_cast<FromStreamFuncT const&>(manip)(stream, std::forward<decltype(args)>(args)...);
                }, m.args_);
            return stream;
        }
    };

public:
    explicit constexpr manipulator(ToStreamFuncT toStreamFunc, FromStreamFuncT fromStreamFunc)
        : ToStreamFuncT(std::move(toStreamFunc)), FromStreamFuncT(std::move(fromStreamFunc))
    {
    }

    template <typename... ArgsT>
    constexpr bound_manipulator<ArgsT...>
    operator ()(ArgsT&&... args) const
    {
        return { *this, { std::forward<ArgsT>(args)... } };
    }
};
template <typename ToStreamFuncT>
struct manipulator<ToStreamFuncT, void> : ToStreamFuncT
{
    template <typename... ArgsT>
    friend struct bound_manipulator;
    template <typename... ArgsT>
    struct bound_manipulator
    {
        manipulator const& manip_;
        std::tuple<ArgsT...> args_;

        friend std::ostream&
        operator <<(std::ostream& stream, bound_manipulator&& m)
        {
            std::apply(
                [&manip = m.manip_, &stream]
                (auto&&... args)
                {
                    return static_cast<ToStreamFuncT const&>(manip)(stream, std::forward<decltype(args)>(args)...);
                }, m.args_);
            return stream;
        }
    };

public:
    explicit constexpr manipulator(ToStreamFuncT toStreamFunc)
        : ToStreamFuncT(std::move(toStreamFunc))
    {
    }

    template <typename... ArgsT>
    constexpr bound_manipulator<ArgsT...>
    operator ()(ArgsT&&... args) const
    {
        return { *this, { std::forward<ArgsT>(args)... } };
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_IOMANIP_HPP_
