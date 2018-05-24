#include <type_traits>

struct Foo { };
struct Bar { };

template <typename A, typename B, typename C = Foo>
struct MyClass1
{
};
template <typename A, typename B, typename C = Bar>
struct MyClass2
{
};

template <typename... Ts>
struct TypeSeq;

template <typename Ls, typename Rs> struct RemoveLast0_;
template <typename Ls> struct RemoveLast0_<Ls, TypeSeq<>> : std::false_type { };
template <typename... Ls, typename N, typename R0, typename... Rs>
struct RemoveLast0_<TypeSeq<Ls...>, TypeSeq<N, R0, Rs...>> : RemoveLast0_<TypeSeq<Ls..., N>, TypeSeq<R0, Rs...>>
{
};
template <typename... Ls, typename R>
struct RemoveLast0_<TypeSeq<Ls...>, TypeSeq<R>> : std::true_type
{
using args = TypeSeq<Ls...>;
using last_arg = R;
};
template <typename Ts> struct RemoveLast_;
template <typename... Ts> struct RemoveLast_<TypeSeq<Ts...>> : RemoveLast0_<TypeSeq<>, TypeSeq<Ts...>> { };

template <template <typename...> class T, typename R, typename ArgsT>
struct RebindLast0_;
template <template <typename...> class T, typename R, typename... ArgsT>
struct RebindLast0_<T, R, TypeSeq<ArgsT...>> : std::true_type
{
using type = T<ArgsT..., R>;
};
template <typename C, typename R> struct RebindLast_ : std::false_type { };
template <template <typename...> class C, typename T0, typename... Ts, typename R>
struct RebindLast_<C<T0, Ts...>, R> : RebindLast0_<C, R, typename RemoveLast_<TypeSeq<T0, Ts...>>::args>
{
};

template <typename C> struct CanReconfigure_ : std::false_type { };template <template <typename...> class C, typename T0, typename... Ts> struct CanReconfigure_<C<T0, Ts...>>
: std::is_same<typename RemoveLast_<TypeSeq<T0, Ts...>>::last_arg, Foo>
{
};
template <typename C> constexpr bool canReconfigure = CanReconfigure_<C>::value;

template <typename C, typename R> using Reconfigure = typename RebindLast_<C, R>::type;


static constexpr bool canReconfigure1 = canReconfigure<MyClass1<int, float>>;
static constexpr bool canReconfigure2 = canReconfigure<MyClass2<int, float>>;

template <auto...> struct VD;
template <typename...> struct TD;

VD<canReconfigure1, canReconfigure2> vd;
TD<Reconfigure<MyClass1<int, float>, Bar>> td;