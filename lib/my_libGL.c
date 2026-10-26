#include "wrapper.h"
#include <string.h>
#include <stdio.h>

WRAP_VOID_FUNC(glTranslated)
WRAP_VOID_FUNC(glNormal3f)
WRAP_VOID_FUNC(glBegin)
WRAP_VOID_FUNC(glEnd)
WRAP_VOID_FUNC(glEnable)
WRAP_VOID_FUNC(glMatrixMode)
WRAP_VOID_FUNC(glClear)
WRAP_FUNC(glGenLists)
WRAP_VOID_FUNC(glLoadIdentity)
WRAP_VOID_FUNC(glShadeModel)
WRAP_VOID_FUNC(glDrawBuffer)
WRAP_VOID_FUNC(glFrustum)
WRAP_VOID_FUNC(glTranslatef)
WRAP_VOID_FUNC(glNewList)
WRAP_VOID_FUNC(glEndList)
WRAP_VOID_FUNC(glMaterialfv)
WRAP_VOID_FUNC(glRotatef)
WRAP_VOID_FUNC(glLightfv)
WRAP_VOID_FUNC(glVertex3f)
WRAP_VOID_FUNC(glPushMatrix)
WRAP_FUNC(glGetString)
WRAP_VOID_FUNC(glCallList)
WRAP_VOID_FUNC(glPopMatrix)
WRAP_VOID_FUNC(glViewport)
WRAP_VOID_FUNC(glDeleteLists)

WRAP_FUNC(glXMakeCurrent)
WRAP_VOID_FUNC(glXDestroyContext)
WRAP_FUNC(glXCreateContext)
WRAP_VOID_FUNC(glXQueryDrawable)
WRAP_VOID_FUNC(glXSwapBuffers)
WRAP_FUNC(glXChooseVisual)
WRAP_FUNC(glXQueryExtensionsString)
WRAP_FUNC(glXGetFBConfigs)
WRAP_FUNC(glXQueryVersion)
WRAP_VOID_FUNC(glXCopyContext)
WRAP_FUNC(glXCreateGLXPixmap)
WRAP_VOID_FUNC(glXDestroyGLXPixmap)
WRAP_FUNC(glXGetConfig)
WRAP_FUNC(glXGetCurrentContext)
WRAP_FUNC(glXGetCurrentDrawable)
WRAP_FUNC(glXIsDirect)
WRAP_FUNC(glXQueryExtension)
WRAP_VOID_FUNC(glXUseXFont)
WRAP_VOID_FUNC(glXWaitGL)
WRAP_VOID_FUNC(glXWaitX)
WRAP_FUNC(glXGetClientString)
WRAP_FUNC(glXQueryServerString)
WRAP_FUNC(glXGetCurrentDisplay)
WRAP_FUNC(glXChooseFBConfig)
WRAP_FUNC(glXCreateNewContext)
WRAP_FUNC(glXCreatePbuffer)
WRAP_FUNC(glXCreatePixmap)
WRAP_FUNC(glXCreateWindow)
WRAP_VOID_FUNC(glXDestroyPbuffer)
WRAP_VOID_FUNC(glXDestroyPixmap)
WRAP_VOID_FUNC(glXDestroyWindow)
WRAP_FUNC(glXGetCurrentReadDrawable)
WRAP_FUNC(glXGetFBConfigAttrib)
WRAP_VOID_FUNC(glXGetSelectedEvent)
WRAP_FUNC(glXGetVisualFromFBConfig)
WRAP_FUNC(glXMakeContextCurrent)
WRAP_FUNC(glXQueryContext)
WRAP_VOID_FUNC(glXSelectEvent)
// TODO: glXGetProcAddress возвращает адрес функции,
// принимая её строку. Нужно чтобы он возвращал обёртки,
// в то время как лично он возвращает оригинал. дополнительно проверить glXGetProcAddressARB

typedef struct {
    const char* name;
    void* adderss;
} Function;

void* my_glXGetProcAddressARB(const char* name);
void my_glXGetProcAddress(const char* name);

Function GLX_functions[] = {
    "glXMakeCurrent", my_glXMakeCurrent,
    "glXDestroyContext", my_glXDestroyContext,
    "glXCreateContext", my_glXCreateContext,
    "glXQueryDrawable", my_glXQueryDrawable,
    "glXSwapBuffers", my_glXSwapBuffers,
    "glXChooseVisual", my_glXChooseVisual,
    "glXQueryExtensionsString", my_glXQueryExtensionsString,
    "glXGetFBConfigs", my_glXGetFBConfigs,
    "glXQueryVersion", my_glXQueryVersion,
    "glXCopyContext", my_glXCopyContext,
    "glXCreateGLXPixmap", my_glXCreateGLXPixmap,
    "glXDestroyGLXPixmap", my_glXDestroyGLXPixmap,
    "glXGetConfig", my_glXGetConfig,
    "glXGetCurrentContext", my_glXGetCurrentContext,
    "glXGetCurrentDrawable", my_glXGetCurrentDrawable,
    "glXIsDirect", my_glXIsDirect,
    "glXQueryExtension", my_glXQueryExtension,
    "glXUseXFont", my_glXUseXFont,
    "glXWaitGL", my_glXWaitGL,
    "glXWaitX", my_glXWaitX,
    "glXGetClientString", my_glXGetClientString,
    "glXQueryServerString", my_glXQueryServerString,
    "glXGetCurrentDisplay", my_glXGetCurrentDisplay,
    "glXChooseFBConfig", my_glXChooseFBConfig,
    "glXCreateNewContext", my_glXCreateNewContext,
    "glXCreatePbuffer", my_glXCreatePbuffer,
    "glXCreatePixmap", my_glXCreatePixmap,
    "glXCreateWindow", my_glXCreateWindow,
    "glXDestroyPbuffer", my_glXDestroyPbuffer,
    "glXDestroyPixmap", my_glXDestroyPixmap,
    "glXDestroyWindow", my_glXDestroyWindow,
    "glXGetCurrentReadDrawable", my_glXGetCurrentReadDrawable,
    "glXGetFBConfigAttrib", my_glXGetFBConfigAttrib,
    "glXGetSelectedEvent", my_glXGetSelectedEvent,
    "glXGetVisualFromFBConfig", my_glXGetVisualFromFBConfig,
    "glXMakeContextCurrent", my_glXMakeContextCurrent,
    "glXQueryContext", my_glXQueryContext,
    "glXSelectEvent", my_glXSelectEvent,
    "my_glXGetProcAddressARB", my_glXGetProcAddressARB,
    "my_glXGetProcAddress", my_glXGetProcAddress,
    NULL, NULL
};

void* my_glXGetProcAddressARB(const char* name) {
    void* ret = NULL;
    size_t addr = 0;
    Function func;
    do {
        func = GLX_functions[addr];
        if (strcmp(name, func.name) == 0) {
            ret = func.adderss;
            break;
        }
    } while(func.adderss);
    if (!ret) printf("No such function: %s\n", name);
    WRAPPED_RET(ret);
    return ret;
}
void my_glXGetProcAddress(const char* name) {
    void* ret = my_glXGetProcAddressARB(name);
    WRAPPED_RET(ret);
}