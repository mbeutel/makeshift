// We could define mk:: function object pendants for most standard function objects, and ...

// Could it be I'm reinventing traits here?

// How can we permit recursion on the combined thing?


auto streamer = mk::make_object(
    mk::to_stream{ },
    mk::from_stream{ }
);
streamer.to_stream(stream, obj);

    // if to_stream is a function object, this should invoke it rather than constructing a new one;
    // if it is a type, this uses CTAD
auto ctxWriter = mk::to_stream(
    [](auto&& self, std::ostream& stream, auto obj)
    {
        stream << "obj: ";
        self.next().to_stream(stream, obj); // next() would simply decrement the priority tag value
    });

auto boolWriter = mk::make_operator<mk::to_stream_tag>(
    [](std::ostream& stream, value_wrapper<bool> obj) // using `value_wrapper<>` to suppress value conversions
    {
        stream << (obj ? "yes" : "no");
    });
auto fullWriter = mk::take_first(ctxWriter, streamer);

    // we use ..._op because this is profoundly different from default function objects such as `equal_to<>`
struct to_stream_op
{
    template <typename DerivedT>
        struct interface
    {
        template <typename T>
            std::ostream& to_stream(std::ostream& stream, const T& obj) const
        {
            return invoke_operator(static_cast<const DerivedT&>(*this), type_c<to_stream_op>, stream, obj);
        }
    };
    
        // this can be fully generic as far as I can tell
    template <typename DerivedT, typename F>
        struct implementation
    {
    private:
        F func_;
    
    public:
        implementation(F _func)
            : func_(std::move(_func))
        {
        }
    
        template <typename T,
                  typename = std::enable_if_t<std::is_invocable<F, const DerivedT&, std::ostream&, const T&>>> // TODO: with result?
            std::ostream& operator ()(type<to_stream_op>, std::ostream& stream, const T& obj) const
        {
            return func_(static_cast<const DerivedT&>(*this), stream, obj);
        }
    }
};

template <typename ContextT = void>
    struct to_stream_op : define_operator<to_stream_op<ContextT>>
{
    template <typename T>
        std::ostream& to_stream(std::ostream& stream, const T& obj)
    {
        // how to obtain a context here?
    }
};

constexpr inline auto reflect(type<Color>)
{
    return define_metadata(
        name = "foo",
        values = std::array{ Color::red, Color::blue }
        values = std::array{
            define_value(Color::red, name="red"),
            define_value(Color::blue, name="blue")
    );
}

? auto comparer = equal_to([](auto lhs, auto rhs) { return lhs == rhs; });
+ auto equalFunc = equal_to(comparer);
+ auto comparer = make_class(
+     equal_to = equalFunc
+ );

