# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)


#FFMpeg配置
#FFmpeg配置目录
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../../src/main/jniLibs)

# 编解码(最重要的库)
add_library(
            avcodec
            SHARED
            IMPORTED)
set_target_properties(
            avcodec
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libavcodec-56.so)

# 设备信息
add_library(
            avdevice
            SHARED
            IMPORTED)
set_target_properties(
            avdevice
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libavdevice-56.so)


# 滤镜特效处理库
add_library(
            avfilter
            SHARED
            IMPORTED)
set_target_properties(
            avfilter
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libavfilter-5.so)

# 封装格式处理库
add_library(
            avformat
            SHARED
            IMPORTED)
set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libavformat-56.so)

# 工具库(大部分库都需要这个库的支持)
add_library(
            avutil
            SHARED
            IMPORTED)
set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libavutil-54.so)


# 后期处理
add_library(
            postproc
            SHARED
            IMPORTED)
set_target_properties(
            postproc
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libpostproc-53.so)


# 音频采样数据格式转换库
add_library(
            swresample
            SHARED
            IMPORTED)
set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libswresample-1.so)

# 视频像素数据格式转换
add_library(
            swscale
            SHARED
            IMPORTED)
set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libswscale-3.so)


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)


#需要引入我们头文件
include_directories(src/main/jniLibs/include)
include_directories(src/main/jniLibs/other)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             MyLib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             #配置需要编译头文件(这个是在自己的动态库)
             #ffmpeg.h实现文件需要我们自己写（实现文件你可以按照我的要求区作）
             src/main/cpp/ffmpeg_jni.c
             # 配置头文件需要依赖的其他的头文件
             # 在other文件夹中的cmdutils_common_opts.h、config.h都是我们需要依赖
             # 如果少了，那么就编译错误，或者编译不通过
             src/main/cpp/ffmpeg_mod.c
             src/main/cpp/ffmpeg_opt.c
             src/main/cpp/ffmpeg_filter.c
             src/main/cpp/cmdutils.c)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       # 配置链接动态库
                       MyLib avcodec avdevice avfilter avformat avutil postproc swresample swscale

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
