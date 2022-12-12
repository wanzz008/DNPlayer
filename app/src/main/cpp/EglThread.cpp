//
// Created by bytedance on 2022/12/5.
//

#include <EGL/eglplatform.h>
#include <GLES3/gl3.h>

#include "EglThread.h"
#include "EglHelper.h"

EglThread::EglThread() {
    pthread_mutex_init(&pthreadMutex,0);
    pthread_cond_init(&pthreadCond,0);
}

EglThread::~EglThread() {
    pthread_mutex_destroy(&pthreadMutex);
    pthread_cond_destroy(&pthreadCond);
}

void EglThread::setCallBackOnCreate(EglThread::OnCreate create) {
    this->onCreate = create;
}

void EglThread::setCallBackOnChange(EglThread::OnChange onChange) {
    this->onChange = onChange;
}

void EglThread::setCallBackOnDraw(EglThread::OnDraw onDraw) {
    this->onDraw = onDraw ;
}

void EglThread::setRenderModule(int renderModule) {
    this->mRenderType = renderModule ;
}

void EglThread::notifyRender() {
    LOGE("EglThread::notifyRender........");
    pthread_mutex_lock(&pthreadMutex);
    pthread_cond_signal(&pthreadCond);
    pthread_mutex_unlock(&pthreadMutex);
}

void *eglThreadImpl(void *args){
    EglThread *eglThread = static_cast<EglThread *>(args);
    LOGE("EglThread::eglThreadImpl........");
    eglThread->useEGL();
    return 0 ;
}

void EglThread::onSurfaceCreate(EGLNativeWindowType pWindow) { // ANativeWindow *pWindow
    if (mEglThread == -1){
//        LOGE("EglThread::onSurfaceCreate........");
        isCreate = true ;
        this->aNativeWindow = pWindow ;
        pthread_create(&mEglThread,0,eglThreadImpl, this);
    }
}


void EglThread::onSurfaceChange(int width, int height) {
    if (mEglThread != -1){
//        LOGE("EglThread::onSurfaceChange........");
        surfaceWidth = width ;
        surfaceHeight = height ;
        isChange = true ;

        notifyRender();
    }
}

void EglThread::useEGL() {

    EglHelper *helper = new EglHelper();
    if (helper->initEgl(aNativeWindow) != 0){
        LOGE("eglHelper initEgl error");
        return ;
    }

    this->isExit = false;
    isExit = false ;
    while (!isExit){
        if (isCreate){
            isCreate = false ;
            this->onCreate();
        }
        if (isChange){
            isChange = false ;
            isStart = true ;
            this->onChange(surfaceWidth , surfaceHeight);
        }

        if (isStart){
            LOGE("EglThread::onDraw........");
            this->onDraw();

            //切换缓冲区，显示
            helper->swapBuffers();
            if (mRenderType == RENDER_MODULE_AUTO){
                LOGE("EglThread::usleep........");
                usleep(1000000 / 60);
            } else{
                LOGE("EglThread::pthread_mutex_lock........");
                pthread_mutex_lock(&pthreadMutex);
                pthread_cond_wait(&pthreadCond,&pthreadMutex);
                pthread_mutex_unlock(&pthreadMutex);
            }

        }

    }
    helper->destroyEgl();
    delete helper;
    helper = NULL ;

}

