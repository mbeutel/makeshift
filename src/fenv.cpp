
#include <cfenv> // for FE_*

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/fenv.hpp>

#if defined(__APPLE__) && defined(__MACH__) && (defined(__i386__) || defined(__x86_64__))
 #define APPLE_INTEL
#endif // defined(__APPLE__) && defined(__MACH__) && (defined(__i386__) || defined(__x86_64__))

#if defined(_WIN32)
 #include <float.h>
#elif defined(__linux__) || defined(APPLE_INTEL)
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

#ifdef APPLE_INTEL
// borrowed from http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c
static int fegetexcept(void)
{
    fenv_t fenv;
    if (fegetenv(&fenv))
    {
        return -1;
    }
    return fenv.__control & FE_ALL_EXCEPT;
}
static int feenableexcept(int excepts)
{
    fenv_t fenv;
    int new_excepts = excepts & FE_ALL_EXCEPT;
    // previous masks
    int old_excepts;

    if (fegetenv(&fenv))
    {
        return -1;
    }
    old_excepts = fenv.__control & FE_ALL_EXCEPT;

    // unmask
    fenv.__control &= ~new_excepts;
    fenv.__mxcsr   &= ~(new_excepts << 7);

    return fesetenv(&fenv) ? -1 : old_excepts;
}
static int fedisableexcept(int excepts)
{
    fenv_t fenv;
    int new_excepts = excepts & FE_ALL_EXCEPT;
    // all previous masks
    int old_excepts;

    if (fegetenv(&fenv))
    {
        return -1;
    }
    old_excepts = fenv.__control & FE_ALL_EXCEPT;

    // mask
    fenv.__control |= new_excepts;
    fenv.__mxcsr   |= new_excepts << 7;

    return fesetenv(&fenv) ? -1 : old_excepts;
}
#endif

gsl_NODISCARD bool try_set_trapping_fe_exceptions(int excepts) noexcept
{
    Expects((excepts & ~FE_ALL_EXCEPT) == 0);

#if defined(_WIN32)
    unsigned oldFlags;
    unsigned flags = static_cast<unsigned>(excepts); // convert flags
    int result = _controlfp_s(&oldFlags, flags, _MCW_EM);
    return result == 0;
#elif defined(__linux__) || defined(APPLE_INTEL)
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
#elif defined(__linux__) || defined(APPLE_INTEL)
    int flags = fegetexcept();
    Expects(flags != -1);
    return flags & FE_ALL_EXCEPT;
#else
 #error Unsupported operating system.
#endif
}


} // namespace makeshift
