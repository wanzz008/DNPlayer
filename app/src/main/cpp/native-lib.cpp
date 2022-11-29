#include <jni.h>
#include <string>
#include "MyPlayer.h"
#include "android/native_window_jni.h"
#include "macro.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_dnplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

MyPlayer *player = 0 ;
JavaVM *javaVm = 0;
ANativeWindow *window = 0 ;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;

int JNI_OnLoad(JavaVM *vm, void *r) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

/**
 * 接收从videochannel中解码并且格式转换后的rgba数据，进行渲染
 * @param data
 * @param lineszie
 * @param w
 * @param h
 */
void render(uint8_t *data, int lineszie, int w, int h){
//    LOGE("用返回的图像数据进行render.....");
    pthread_mutex_lock(&mutex);
    if (!window){
        pthread_mutex_unlock(&mutex);
        return;
    }
    // 设置窗口属性
    ANativeWindow_setBuffersGeometry(window, w,
                                     h,
                                     WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //填充rgb数据给dst_data
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    // stride：一行多少个数据（RGBA） *4
    int dst_linesize = window_buffer.stride * 4;
    //一行一行的拷贝
    for (int i = 0; i < window_buffer.height; ++i) {
        //memcpy(dst_data , data, dst_linesize);
        memcpy(dst_data + i * dst_linesize, data + i * lineszie, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativePrepare(JNIEnv *env, jobject instance , jstring path_) {

    const char *path = env->GetStringUTFChars(path_,0);

    JavaCallHelper *helper = new JavaCallHelper(javaVm, env, instance);

    player = new MyPlayer(path,helper);
    // 设置渲染数据的回调
    player->setRenderFrameCallback(render);
    player->prepare();

    env->ReleaseStringUTFChars(path_,path);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativeStart(JNIEnv *env, jobject thiz) {

    player->start();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativeSetSurface(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);
    if (window){
        //把老的释放
        ANativeWindow_release(window);
        window = 0 ;
    }
    window = ANativeWindow_fromSurface(env,surface);
    pthread_mutex_unlock(&mutex);
}