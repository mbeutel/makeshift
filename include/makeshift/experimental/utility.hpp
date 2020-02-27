
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_HPP_


namespace makeshift {


    //
    // Class that can be constructed from any value via user-defined conversion sequence.
    // Useful for defining fallback overloads.
    //
    //     template <typename WidgetT>
    //     auto color(WidgetT widget) -> decltype(widget.color()) { ... }
    //     Color color(any_sink) { return Color::black }
    //
struct any_sink
{
    template <typename T>
    any_sink(T&&) noexcept
    {
    }
};


    //
    // Class from which any value can be constructed via user-defined conversion sequence.
    //
struct any_source
{
    template <typename T>
    operator T(void) const noexcept
    {
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_HPP_
