#include <jni.h>
#include <string>
#include "MyPlayer.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_dnplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JavaVM *javaVm = 0;

int JNI_OnLoad(JavaVM *vm, void *r) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativePrepare(JNIEnv *env, jobject instance , jstring path_) {

    const char *path = env->GetStringUTFChars(path_,0);

    JavaCallHelper *helper = new JavaCallHelper(javaVm, env, instance);

    MyPlayer *player = new MyPlayer(path,helper);
    player->prepare();

    env->ReleaseStringUTFChars(path_,path);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativeStart(JNIEnv *env, jobject thiz) {


}
