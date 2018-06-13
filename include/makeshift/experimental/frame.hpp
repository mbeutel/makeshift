
#ifndef MAKESHIFT_FRAME_HPP_
#define MAKESHIFT_FRAME_HPP_


#include <vector>


namespace makeshift
{

namespace detail
{

// TODO: support allocators

template <typename T, template <typename...> class ContainerT>
    class frame_column
{
    // TODO: declare friendship with the code that handles move semantics, initialization etc.

private:
    ContainerT<T> data_;

public:
    using value_type = T;
    using size_type = typename ContainerT<T>::size_type;
    using difference_type = typename ContainerT<T>::difference_type;
    using reference = typename ContainerT<T>::reference;
    using const_reference = typename ContainerT<T>::const_reference;
    using pointer = typename ContainerT<T>::pointer;
    using const_pointer = typename ContainerT<T>::const_pointer;
    using iterator = typename ContainerT<T>::iterator;
    using const_iterator = typename ContainerT<T>::const_iterator;
    
    constexpr const T* data(void) const noexcept { return data_.data(); }
    constexpr T* data(void) noexcept { return data_.data(); }

    //const ContainerT<T>& container(void) const noexcept { return data_; }

    constexpr const T& at(size_type i) const { return data_.at(i); }
    constexpr T& at(size_type i) { return data_.at(i); }

    constexpr const T& front(void) const { return data_.front(); }
    constexpr T& front(void) { return data_.front(); }
    constexpr const T& back(void) const { return data_.back(); }
    constexpr T& back(void) { return data_.back(); }

    constexpr bool empty(void) const noexcept { return data_.empty(); }
    constexpr size_type size(void) const noexcept { return data_.size(); }
    constexpr size_type capacity(void) const noexcept { return data_.size(); }

    constexpr const T& operator [](size_type i) const noexcept { return data_[i]; }
    constexpr T& operator [](size_type i) noexcept { return data_[i]; }

    constexpr iterator begin(void) const noexcept { return data_.begin(); }
    constexpr iterator end(void) const noexcept { return data_.end(); }
    constexpr const_iterator cbegin(void) const noexcept { return data_.cbegin(); }
    constexpr const_iterator cend(void) const noexcept { return data_.cend(); }
};

} // namespace detail

namespace types
{

template <auto... Fields> struct fields_t { };
//template <auto... Fields> constexpr fields_t<Fields...> fields { };

template <template <typename...> class FieldsT, template <typename...> class ContainerT>
    struct define_frame
{
    using base = define_frame;
};

    // Useful types:
    //
    //     row<F>: tuple-like type with named fields
    //     row_ref<F>, const_row_ref<F>
    //     column<F, I>: frame_column<>
    //     F::row_iterator, F::const_row_iterator


template <typename ExtentT, typename BoundaryConditionT>
    struct DomainFields
{
    ExtentT extent;
    BoundaryConditionT boundaryCondition;

    friend auto fields_t<&DomainFields::extent, &DomainFields::boundaryCondition> fields(DomainFields*) { return { }; }
};


struct Domain : define_frame<std::vector, DomainFields>
{
    using base::base;
};

} // namespace types

} // namespace makeshift

#endif // MAKESHIFT_FRAME_HPP_
