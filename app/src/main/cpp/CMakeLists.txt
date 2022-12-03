# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("dnplayer")

#使用CMakeLists.txt同样可以指定so库的输出路径,但一定要在add_library之前设置，否则不会生效:
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}/../libs/${ANDROID_ABI}) #指定路径
#如果只是在自己的项目中使用，CMake的方式在打包APK的时候会自动将cpp文件编译成so文件拷贝进去。如果要提供给外部使用时，Make Project，之后在libs目录下就可以看到生成的对应配置的相关CPU平台的.so文件。


file(GLOB source_file *.cpp)

add_library( # Sets the name of the library.
        dnplayer

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${source_file})

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

include_directories(include)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")


target_link_libraries( # Specifies the target library.
        dnplayer

        # Links the target library to the log library
        # included in the NDK.
        avformat avcodec avfilter    avutil swresample swscale
        z
        android OpenSLES
        ${log-lib})