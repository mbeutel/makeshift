
#ifndef INCLUDED_MAKESHIFT_FENV_HPP_
#define INCLUDED_MAKESHIFT_FENV_HPP_


#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/macros.hpp>        // for MAKESHIFT_NODISCARD
#include <makeshift/detail/export.hpp> // for MAKESHIFT_PUBLIC


namespace makeshift
{


    //ᅟ
    // Sets hardware exception traps for the floating-point exceptions specified by the given mask value.
    // If an exception flag bit is on, the corresponding exception will be trapped; if the bit is clear, the exception will be masked.
    //
MAKESHIFT_PUBLIC MAKESHIFT_NODISCARD bool try_set_trapping_fe_exceptions(int excepts) noexcept;

    //ᅟ
    // Disables hardware exception traps for the floating-point exceptions specified by the given mask value.
    //
inline void set_trapping_fe_exceptions(int excepts)
{
    bool succeeded = try_set_trapping_fe_exceptions(excepts);
    Expects(succeeded);
}

    //ᅟ
    // Returns the bitmask of all floating-point exceptions for which trapping is currently enabled.
    //
MAKESHIFT_PUBLIC int get_trapping_fe_exceptions(void) noexcept;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FENV_HPP_
