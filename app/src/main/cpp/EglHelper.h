//
// Created by bytedance on 2022/12/6.
//

#ifndef VESTUDY_EGLHELPER_H
#define VESTUDY_EGLHELPER_H


#include "EGL/egl.h"

class EglHelper {

public:
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLSurface mEglSurface;

public:
    EglHelper();

    ~EglHelper();

    int initEgl(EGLNativeWindowType pWindow);

    void destroyEgl();

    int swapBuffers();
};


#endif //VESTUDY_EGLHELPER_H
