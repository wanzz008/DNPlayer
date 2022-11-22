//
// Created by bytedance on 2022/11/18.
//

#ifndef DNPLAYER_JAVACALLHELPER_H
#define DNPLAYER_JAVACALLHELPER_H


#include <jni.h>

class JavaCallHelper {
public:
    JavaCallHelper(_JavaVM *pVm, _JNIEnv *pEnv, _jobject *pJobject);
    ~JavaCallHelper();
    void onError(int thread,int errorCode);
    void onPrepare(int thread);

private:
    JavaVM *vm ;
    JNIEnv *env;
    jobject instance ;

    jmethodID onErrorId;
    jmethodID onPrepareId;
};


#endif //DNPLAYER_JAVACALLHELPER_H
