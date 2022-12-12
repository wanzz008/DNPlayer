//
// Created by bytedance on 2022/12/5.
//

#ifndef VESTUDY_EGLTHREAD_H
#define VESTUDY_EGLTHREAD_H


#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "pthread.h"
#include <unistd.h>
#include "macro.h"


#define RENDER_MODULE_AUTO 1
#define RENDER_MODULE_MANUAL 2

class EglThread {

public:
    pthread_t mEglThread = -1;

    int mRenderType = RENDER_MODULE_MANUAL;
    ANativeWindow *aNativeWindow = 0 ;

    typedef void (*OnCreate)();
    OnCreate onCreate;

    typedef void (*OnChange)(int width,int height);
    OnChange onChange;

    typedef void (*OnDraw)();
    OnDraw onDraw;

    int surfaceHeight = 0;
    int surfaceWidth = 0;

    pthread_mutex_t pthreadMutex;
    pthread_cond_t pthreadCond;

public:
    EglThread();
    ~EglThread();


    //设置模式
    void setRenderModule(int renderModule);

    void setCallBackOnCreate(OnCreate create);
    void setCallBackOnChange(OnChange onChange);
    void setCallBackOnDraw(OnDraw onDraw);

    void notifyRender();

    void onSurfaceCreate(ANativeWindow *pWindow);

    void onSurfaceChange(int width, int height);

    void useEGL();


    bool isCreate = false;
    bool isChange = false;
    bool isStart = false;
    bool isExit = false;
    float scale_v = 1.0; //给一个默认值1，初始缩放比
    uint8_t *data = 0;
    int width;
    int height;
};


#endif //VESTUDY_EGLTHREAD_H
