
#include <type_traits>


template <typename T>
    struct value_wrapper
{
    T value;

    template <typename U,
              typename = std::enable_if_t<std::is_same<T, std::decay_t<U>>::value>>
        value_wrapper(U&& _value)
        : value(std::forward<U>(_value))
    {
    }
    operator T(void) const { return value; }
};

void foo(value_wrapper<bool>)
{
}


void demo(void)
{
    foo(true);
    bool b = false;
    foo(b);
    foo(42);
    int i = 42;
    foo(i);
}


static_assert(false);
