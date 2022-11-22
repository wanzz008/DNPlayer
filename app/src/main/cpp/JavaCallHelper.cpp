//
// Created by bytedance on 2022/11/18.
//

#include "JavaCallHelper.h"
#include "macro.h"


JavaCallHelper::JavaCallHelper(_JavaVM *pVm, _JNIEnv *pEnv, jobject object) {
    this->vm = pVm ;
    this->env = pEnv ;
    // 一旦涉及到jobject 跨方法 跨线程 就需要创建全局引用
    this->instance = env->NewGlobalRef(object) ;

    jclass clazz = env->GetObjectClass(object);
    onErrorId = env->GetMethodID(clazz,"onError","(I)V");
    onPrepareId = env->GetMethodID(clazz,"onPrepare","()V");

}
JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(instance);
}

//JavaCallHelper::JavaCallHelper(_JavaVM *pVm, _JNIEnv *pEnv, _jobject *pJobject) {
//    this->vm = pVm ;
//    this->env = pEnv ;
//    this->instance = env->NewGlobalRef(pJobject) ;
//}


void JavaCallHelper::onError(int thread,int errorCode) {
//主线程
    if (thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onErrorId,errorCode);
    } else{
        //子线程
        JNIEnv *env;
        //获得属于我这一个线程的jnienv
        vm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(instance,onErrorId,errorCode);
        vm->DetachCurrentThread();
    }
}

void JavaCallHelper::onPrepare(int thread) {
    if (thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onPrepareId);
    } else{
        //子线程
        JNIEnv *env;
        //获得属于我这一个线程的jnienv
        vm->AttachCurrentThread(&env,0);
        env->CallVoidMethod(instance,onPrepareId);
        vm->DetachCurrentThread();
    }
}
