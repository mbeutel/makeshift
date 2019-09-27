
#include <cfenv>    // for FE_*

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/fenv.hpp>

#if defined(_WIN32)
 #include <float.h>
#elif defined(__linux__)
 #include <fenv.h>
#else
 #error Unsupported operating system.
#endif


namespace makeshift
{


#if defined(_WIN32)
    // We assume that the flags for C++11 floating-point settings and for `_controlfp_s()` have identical values.
static_assert(FE_DIVBYZERO == _EM_ZERODIVIDE && FE_INEXACT == _EM_INEXACT && FE_INVALID == _EM_INVALID && FE_OVERFLOW == _EM_OVERFLOW && FE_UNDERFLOW == _EM_UNDERFLOW);
#endif // defined(_WIN32)


MAKESHIFT_NODISCARD bool try_set_trapping_fe_exceptions(int excepts) noexcept
{
    Expects((excepts & ~FE_ALL_EXCEPT) == 0);

#if defined(_WIN32)
    unsigned oldFlags;
    unsigned flags = static_cast<unsigned>(excepts); // convert flags
    int result = _controlfp_s(&oldFlags, flags, _MCW_EM);
    return result == 0;
#elif defined(__linux__)
    int flags = fegetexcept();
    Expects(flags != -1);
    int exceptsToEnable = excepts & ~(flags & FE_ALL_EXCEPT);
    int exceptsToDisable = ~excepts & (flags & FE_ALL_EXCEPT);
    if (exceptsToEnable != 0)
    {
        int result = feenableexcept(excepts);
        if (result != 0) return false;
    }
    if (exceptsToDisable != 0)
    {
        int result = fedisableexcept(excepts);
        if (result != 0) return false;
    }
    return true;
#else
 #error Unsupported operating system.
#endif
}

int get_trapping_fe_exceptions(void) noexcept
{
#if defined(_WIN32)
    unsigned flags;
    int result = _controlfp_s(&flags, 0, 0);
    Expects(result == 0);
    return static_cast<int>(flags) & FE_ALL_EXCEPT; // convert flags
#elif defined(__linux__)
    int flags = fegetexcept();
    Expects(flags != -1);
    return flags & FE_ALL_EXCEPT;
#else
 #error Unsupported operating system.
#endif
}


} // namespace makeshift
