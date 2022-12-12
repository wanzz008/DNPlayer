#include <jni.h>
#include <string>
#include "MyPlayer.h"
#include "android/native_window_jni.h"
#include "macro.h"
#include "EglThread.h"
#include <GLES3/gl3.h>


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

EglThread *eglThread = NULL;

/**
 * 用egl进行渲染
 * @param data
 * @param lineszie
 * @param width
 * @param height
 */
void eglRender(uint8_t *data, int lineszie, int width, int height){
    eglThread->width = width;
    eglThread->height = height;
    eglThread->data = data;
    eglThread->notifyRender();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativePrepare(JNIEnv *env, jobject instance , jstring path_,jboolean isEGL) {

    const char *path = env->GetStringUTFChars(path_,0);

    JavaCallHelper *helper = new JavaCallHelper(javaVm, env, instance);

    player = new MyPlayer(path,helper);
    // 设置渲染数据的回调
    if (!isEGL){
        player->setRenderFrameCallback(render);
    }else{
        player->setRenderFrameCallback(eglRender);
    }
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

const char *vShaderStr =
        "#version 300 es\n"
        "layout(location = 0) in vec4 a_position;\n"
        "layout(location = 1) in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";

const char *fShaderStr =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_TextureMap;//采样器\n"
        "void main()\n"
        "{\n"
        "    outColor = texture(s_TextureMap, v_texCoord);\n"
        "}";

GLfloat verticesCoords[] = {  //三维顶点坐标，两个三角形组成一个正方形
        -1.0f,  1.0f, 0.0f,  // Position 0
        -1.0f, -1.0f, 0.0f,  // Position 1
        1.0f,  -1.0f, 0.0f,  // Position 2
        1.0f,   1.0f, 0.0f,  // Position 3
};

//纹理坐标转换，以左上角为原点的纹理坐标转换成以左下角为原点的纹理坐标，
// 比如以左上角为原点的（0，0）对应以左下角为原点的纹理坐标的（0，1）
GLfloat textureCoords[] = {  //纹理坐标
        0.0f,  0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f         // TexCoord 3
};

GLushort indices[] = {0, 1, 2, 0, 2, 3};

unsigned int shaderProgram;

unsigned int m_TextureId, m_VboIds[3], m_VaoId;
void onCreate(){
    LOGE("callBackOnCreate");
    //初始化GLES相关
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //将着色器源码附加到着色器对象上
    glShaderSource(vertexShader, 1, &vShaderStr, NULL);
    //编译着色器
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
    }

    //创建片段着色器fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //将着色器源码附加到着色器对象上
    glShaderSource(fragmentShader, 1, &fShaderStr, NULL);
    //编译着色器
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
    }
    // link shaders
    //创建一个着色器程序对象
    shaderProgram = glCreateProgram();
    //将着色器附加到程序
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    //链接
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
    }
    //link完就可以删除了
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOGE("Compile Shader Success!");

    //初始化纹理
    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(3, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);

    glBindVertexArray(GL_NONE);

    LOGE("GLES Texture Success!");

}
void onChange(int width,int height){
    LOGE("Square::onChange........");
    // Set the viewport
    glViewport(0,0,width, height);
}
void onDraw(){
    if (!eglThread->data) {
        return;
    }
    LOGE("callBackOnDraw start");
    glClear(GL_COLOR_BUFFER_BIT);

    //upload RGBA image data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);

//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame->width, frame->height, 0, GL_RGBA,
//                 GL_UNSIGNED_BYTE, frame->data[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, eglThread->width, eglThread->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, eglThread->data);

    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // Use the program object
    glUseProgram(shaderProgram);

    glBindVertexArray(m_VaoId);

    // Bind the RGBA map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    unsigned int textureLoc = glGetUniformLocation(shaderProgram, "s_TextureMap");
    //设置每个采样器的方式告诉OpenGL每个着色器采样器属于哪个纹理单元
    glUniform1i(textureLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *) 0);
    LOGE("callBackOnDraw end");

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_eglInit(JNIEnv *env, jobject thiz, jobject surface) {
    eglThread = new EglThread;
    eglThread->setCallBackOnCreate(onCreate);
    eglThread->setCallBackOnChange(onChange);
    eglThread->setCallBackOnDraw(onDraw);

    eglThread->setRenderModule(RENDER_MODULE_MANUAL);


    ANativeWindow *window = ANativeWindow_fromSurface(env,surface);
    eglThread->onSurfaceCreate(window);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_dnplayer_MyPlayer_nativeSurfaceChanged(JNIEnv *env, jobject thiz, jobject surface,
                                                        jint width, jint height) {

    eglThread->onSurfaceChange(width,height);
}