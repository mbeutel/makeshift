
#ifndef INCLUDED_MAKESHIFT_DETAIL_STREAMABLE_STRING_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_STREAMABLE_STRING_HPP_


#include <iosfwd>
#include <string>
#include <string_view>

#include <makeshift/string.hpp>
#include <makeshift/streamable.hpp>

#include <makeshift/detail/cfg.hpp> // for MAKESHIFT_DLLFUNC


namespace makeshift
{

namespace detail
{


struct ostreamable_arg
{
    ostreamable_arg(const ostreamable_arg&) = delete;
    ostreamable_arg& operator =(const ostreamable_arg&) = delete;

private:
    struct concept_
    {
        virtual void to_stream(std::ostream& stream) const = 0;
        virtual ~concept_(void) { }
    };
    struct equivalent_layout_type { void* vmt; int& r1; int& r2; };
    template <typename T, typename SerializerT>
        struct model_ final : concept_
    {
    private:
        T& value_;
        SerializerT& serializer_;
    public:
        model_(T& _value, SerializerT& _serializer) noexcept : value_(_value), serializer_(_serializer) { }
        void to_stream(std::ostream& stream) const override { stream << streamable(value_, serializer_); }
    };

    concept_* c_;
    alignas(equivalent_layout_type) char buf_[sizeof(equivalent_layout_type)];

public:
    template <typename T, typename SerializerT>
        ostreamable_arg(const streamable_ref<T, SerializerT>& arg)
    {
        static_assert(sizeof(model_<T, SerializerT>) == sizeof(equivalent_layout_type)
            && alignof(model_<T, SerializerT>) == alignof(equivalent_layout_type));
        c_ = new (buf_) model_<T, SerializerT>{ arg.value(), arg.serializer() };
    }
    ~ostreamable_arg(void)
    {
        c_->~concept_();
    }
    friend std::ostream& operator <<(std::ostream& stream, const ostreamable_arg& self)
    {
        self.c_->to_stream(stream);
        return stream;
    }
};
struct istreamable_arg
{
    istreamable_arg(const istreamable_arg&) = delete;
    istreamable_arg& operator =(const istreamable_arg&) = delete;

private:
    struct concept_
    {
        virtual void from_stream(std::istream& stream) const = 0;
        virtual ~concept_(void) { }
    };
    struct equivalent_layout_type { void* vmt; int& r1; int& r2; };
    template <typename T, typename SerializerT>
        struct model_ final : concept_
    {
    private:
        T& value_;
        SerializerT& serializer_;
    public:
        model_(T& _value, SerializerT& _serializer) noexcept : value_(_value), serializer_(_serializer) { }
        void from_stream(std::istream& stream) const override { stream >> streamable(value_, serializer_); }
    };

    concept_* c_;
    alignas(equivalent_layout_type) char buf_[sizeof(equivalent_layout_type)];

public:
    template <typename T, typename SerializerT,
              typename = std::enable_if_t<!std::is_const<T>::value>>
        istreamable_arg(const streamable_ref<T, SerializerT>& arg)
    {
        static_assert(sizeof(model_<T, SerializerT>) == sizeof(equivalent_layout_type)
            && alignof(model_<T, SerializerT>) == alignof(equivalent_layout_type));
        c_ = new (buf_) model_<T, SerializerT>{ arg.value(), arg.serializer() };
    }
    ~istreamable_arg(void)
    {
        c_->~concept_();
    }
    friend std::istream& operator >>(std::istream& stream, const istreamable_arg& self)
    {
        self.c_->from_stream(stream);
        return stream;
    }
};

MAKESHIFT_DLLFUNC std::string streamable_to_string(ostreamable_arg arg);
MAKESHIFT_DLLFUNC void streamable_from_string(istreamable_arg arg, std::string_view string);


} // namespace detail


inline namespace serialize
{


template <typename T, typename SerializerT>
    std::string to_string(const streamable_ref<T, SerializerT>& arg)
{
    return makeshift::detail::streamable_to_string(arg);
}
template <typename T, typename SerializerT,
          typename = std::enable_if_t<!std::is_const<T>::value>>
    void from_string(const streamable_ref<T, SerializerT>& arg, std::string_view string)
{
    return makeshift::detail::streamable_from_string(arg, string);
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_STREAMABLE_STRING_HPP_
