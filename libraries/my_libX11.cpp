#include "wrapper.hpp"

extern "C" {
    WRAP_FUNC(XLookupKeysym)
    WRAP_FUNC(XLookupString)
    WRAP_BIG_FUNC(XCreateWindow)
    WRAP_FUNC(XCreateColormap)
    WRAP_FUNC(XDestroyWindow)
    WRAP_FUNC(XCloseDisplay)
    WRAP_FUNC(XNextEvent)
    WRAP_FUNC(XMapWindow)
    WRAP_FUNC_VOID(XSetNormalHints)
    WRAP_FUNC(XInternAtom)
    WRAP_BIG_FUNC(XSetStandardProperties)
    WRAP_BIG_FUNC(XChangeProperty)
    WRAP_FUNC(XPending)
    WRAP_FUNC(XParseGeometry)
    WRAP_FUNC(XFree)
    WRAP_FUNC(XOpenDisplay)
}