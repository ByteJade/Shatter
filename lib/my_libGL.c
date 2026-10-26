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
WRAP_VOID_FUNC(glAccum)
WRAP_VOID_FUNC(glAlphaFunc)
WRAP_MED_FUNC(glBitmap)
WRAP_VOID_FUNC(glBlendFunc)
WRAP_VOID_FUNC(glCallLists)
WRAP_VOID_FUNC(glClearAccum)
WRAP_VOID_FUNC(glClearColor)
WRAP_VOID_FUNC(glClearDepth)
WRAP_VOID_FUNC(glClearIndex)
WRAP_VOID_FUNC(glClearStencil)
WRAP_VOID_FUNC(glClipPlane)
WRAP_VOID_FUNC(glColor3b)
WRAP_VOID_FUNC(glColor3bv)
WRAP_VOID_FUNC(glColor3d)
WRAP_VOID_FUNC(glColor3dv)
WRAP_VOID_FUNC(glColor3f)
WRAP_VOID_FUNC(glColor3fv)
WRAP_VOID_FUNC(glColor3i)
WRAP_VOID_FUNC(glColor3iv)
WRAP_VOID_FUNC(glColor3s)
WRAP_VOID_FUNC(glColor3sv)
WRAP_VOID_FUNC(glColor3ub)
WRAP_VOID_FUNC(glColor3ubv)
WRAP_VOID_FUNC(glColor3ui)
WRAP_VOID_FUNC(glColor3uiv)
WRAP_VOID_FUNC(glColor3us)
WRAP_VOID_FUNC(glColor3usv)
WRAP_VOID_FUNC(glColor4b)
WRAP_VOID_FUNC(glColor4bv)
WRAP_VOID_FUNC(glColor4d)
WRAP_VOID_FUNC(glColor4dv)
WRAP_VOID_FUNC(glColor4f)
WRAP_VOID_FUNC(glColor4fv)
WRAP_VOID_FUNC(glColor4i)
WRAP_VOID_FUNC(glColor4iv)
WRAP_VOID_FUNC(glColor4s)
WRAP_VOID_FUNC(glColor4sv)
WRAP_VOID_FUNC(glColor4ub)
WRAP_VOID_FUNC(glColor4ubv)
WRAP_VOID_FUNC(glColor4ui)
WRAP_VOID_FUNC(glColor4uiv)
WRAP_VOID_FUNC(glColor4us)
WRAP_VOID_FUNC(glColor4usv)
WRAP_VOID_FUNC(glColorMask)
WRAP_VOID_FUNC(glColorMaterial)
WRAP_VOID_FUNC(glCopyPixels)
WRAP_VOID_FUNC(glCullFace)
WRAP_VOID_FUNC(glDepthFunc)
WRAP_VOID_FUNC(glDepthMask)
WRAP_VOID_FUNC(glDepthRange)
WRAP_VOID_FUNC(glDisable)
WRAP_VOID_FUNC(glDrawPixels)
WRAP_VOID_FUNC(glEdgeFlag)
WRAP_VOID_FUNC(glEdgeFlagv)
WRAP_VOID_FUNC(glEvalCoord1d)
WRAP_VOID_FUNC(glEvalCoord1dv)
WRAP_VOID_FUNC(glEvalCoord1f)
WRAP_VOID_FUNC(glEvalCoord1fv)
WRAP_VOID_FUNC(glEvalCoord2d)
WRAP_VOID_FUNC(glEvalCoord2dv)
WRAP_VOID_FUNC(glEvalCoord2f)
WRAP_VOID_FUNC(glEvalCoord2fv)
WRAP_VOID_FUNC(glEvalMesh1)
WRAP_VOID_FUNC(glEvalMesh2)
WRAP_VOID_FUNC(glEvalPoint1)
WRAP_VOID_FUNC(glEvalPoint2)
WRAP_VOID_FUNC(glFeedbackBuffer)
WRAP_VOID_FUNC(glFinish)
WRAP_VOID_FUNC(glFlush)
WRAP_VOID_FUNC(glFogf)
WRAP_VOID_FUNC(glFogfv)
WRAP_VOID_FUNC(glFogi)
WRAP_VOID_FUNC(glFogiv)
WRAP_VOID_FUNC(glFrontFace)
WRAP_VOID_FUNC(glGetBooleanv)
WRAP_VOID_FUNC(glGetClipPlane)
WRAP_VOID_FUNC(glGetDoublev)
WRAP_FUNC(glGetError)
WRAP_VOID_FUNC(glGetFloatv)
WRAP_VOID_FUNC(glGetIntegerv)
WRAP_FUNC(glCreateProgram)
WRAP_FUNC(glCreateShader)
WRAP_VOID_FUNC(glCompileShader)
WRAP_VOID_FUNC(glDeleteProgram)
WRAP_VOID_FUNC(glDeleteShader)
WRAP_VOID_FUNC(glDetachShader)
WRAP_VOID_FUNC(glBindFramebufferEXT)
WRAP_VOID_FUNC(glBindRenderbufferEXT)
WRAP_FUNC(glCheckFramebufferStatusEXT)
WRAP_VOID_FUNC(glDeleteFramebuffersEXT)
WRAP_VOID_FUNC(glFramebufferRenderbufferEXT)
WRAP_VOID_FUNC(glFramebufferTexture1DEXT)
WRAP_VOID_FUNC(glFramebufferTexture2DEXT)
WRAP_VOID_FUNC(glFramebufferTexture3DEXT)
WRAP_VOID_FUNC(glGenFramebuffersEXT)
WRAP_VOID_FUNC(glGenRenderbuffersEXT)
WRAP_VOID_FUNC(glGenerateMipmapEXT)
WRAP_VOID_FUNC(glGetFramebufferAttachmentParameterivEXT)
WRAP_VOID_FUNC(glGetRenderbufferParameterivEXT)
WRAP_FUNC(glIsFramebufferEXT)
WRAP_FUNC(glIsRenderbufferEXT)
WRAP_VOID_FUNC(glRenderbufferStorageEXT)

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
    "glXGetProcAddressARB", my_glXGetProcAddressARB,
    "glXGetProcAddress", my_glXGetProcAddress,
};

void* my_glXGetProcAddressARB(const char* name) {
    void* ret = NULL;
    const size_t end = sizeof(GLX_functions) / sizeof(Function);
    for (size_t addr = 0; addr < end; addr++) {
        Function func = GLX_functions[addr];
        if (strcmp(name, func.name) == 0) {
            ret = func.adderss;
            break;
        }
    }
    if (!ret) printf("No such function: %s\n", name);
    WRAPPED_RET(ret);
    return ret;
}
void my_glXGetProcAddress(const char* name) {
    void* ret = my_glXGetProcAddressARB(name);
    WRAPPED_RET(ret);
}