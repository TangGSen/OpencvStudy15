#include <jni.h>
#include "opencv2/opencv.hpp"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <string>
#define LOG_TAG    "sensen"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)
using namespace cv;
using namespace std;
CascadeClassifier *classifier;
ANativeWindow *nativeWindow;



extern "C" {

void bitmap2Mat(JNIEnv *env, jobject bitmap, Mat &dst, jboolean needUnPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    try {
        LOGE("nBitmapToMat");
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC4);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGE("nBitmapToMat: RGBA_8888 -> CV_8UC4");
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (needUnPremultiplyAlpha) cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
            else tmp.copyTo(dst);
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            LOGE("nBitmapToMat: RGB_565 -> CV_8UC4");
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, COLOR_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nBitmapToMat catched cv::Exception: %s", e.what());
        jclass je = env->FindClass("org/opencv/core/CvException");
        if (!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nBitmapToMat catched unknown exception (...)");
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
}


void mat2Bitmap(JNIEnv *env, Mat &src, jobject bitmap, jboolean needPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;

    try {
        LOGE("nMatToBitmap");
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);

        CV_Assert(src.dims == 2 && info.height == (uint32_t) src.rows &&
                  info.width == (uint32_t) src.cols);
        CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (src.type() == CV_8UC1) {
                LOGE("nMatToBitmap: CV_8UC1 -> RGBA_8888");
                cvtColor(src, tmp, COLOR_GRAY2RGBA);
            } else if (src.type() == CV_8UC3) {
                LOGE("nMatToBitmap: CV_8UC3 -> RGBA_8888");
                cvtColor(src, tmp, COLOR_RGB2RGBA);
            } else if (src.type() == CV_8UC4) {
                LOGE("nMatToBitmap: CV_8UC4 -> RGBA_8888");
                if (needPremultiplyAlpha) cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                else src.copyTo(tmp);
            }
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if (src.type() == CV_8UC1) {
                LOGE("nMatToBitmap: CV_8UC1 -> RGB_565");
                cvtColor(src, tmp, COLOR_GRAY2BGR565);
            } else if (src.type() == CV_8UC3) {
                LOGE("nMatToBitmap: CV_8UC3 -> RGB_565");
                cvtColor(src, tmp, COLOR_RGB2BGR565);
            } else if (src.type() == CV_8UC4) {
                LOGE("nMatToBitmap: CV_8UC4 -> RGB_565");
                cvtColor(src, tmp, COLOR_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nMatToBitmap catched cv::Exception: %s", e.what());
        jclass je = env->FindClass("org/opencv/core/CvException");
        if (!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nMatToBitmap catched unknown exception (...)");
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return;
    }
}
JNIEXPORT void JNICALL
Java_sen_com_opencvstudy15_OpencvUtils_loadFaceXml(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    //加载分类器
    classifier = new CascadeClassifier(path);
    LOGE("ANative:%s",path);
    env->ReleaseStringUTFChars(path_, path);
}

JNIEXPORT void JNICALL
Java_sen_com_opencvstudy15_OpencvUtils_handleBitmap(JNIEnv *env, jobject instance, jobject bitmap) {
    Mat src;
    //将bitmap 装换成opencv 可以识别的格式
    bitmap2Mat(env,bitmap,src, true);
    //将图像转变为灰度图像
    if(classifier){
        vector<Rect> faces;

        Mat grayMat;
        cvtColor(src,grayMat,CV_BGR2GRAY);
        //直线图均衡化，增强对比效果
        equalizeHist(grayMat,grayMat);
        //开始识别，并将识别到的脸部区域写入faces 向量中
        classifier->detectMultiScale(grayMat,faces);


        grayMat.release();
        int size = faces.size();
        for(int i = 0;i<size;i++){
            Rect face = faces[i];
            rectangle(src,face.tl(),face.br(),Scalar(255,0,255));
        }
    }

    if (!nativeWindow){
        LOGE("native windown null");
        goto end;
    }
    LOGE("ANativeWindow_Buffer");
    //绘制到android
    ANativeWindow_Buffer window_buffer;
    if(ANativeWindow_lock(nativeWindow,&window_buffer,0)){
        //加锁成功返回0，失败为1
       LOGE("native windown lock fail");
        goto end;
    };
    //转换成Android 有透明度的图片
    cvtColor(src,src,CV_BGR2BGRA);
    //jni 层 opencv 图像压缩
    resize(src,src,Size(window_buffer.width,window_buffer.height));
    //将数据复制到window_buffer
    memcpy(window_buffer.bits,src.data,window_buffer.width*window_buffer.height*4);

    ANativeWindow_unlockAndPost(nativeWindow);

    end:
    src.release();




}

JNIEXPORT void JNICALL
Java_sen_com_opencvstudy15_OpencvUtils_setSurface(JNIEnv *env, jobject instance, jobject surface,
                                                  jint width, jint height) {
    if (surface){
        if (nativeWindow){
            ANativeWindow_release(nativeWindow);
            nativeWindow =0;
        }
        nativeWindow = ANativeWindow_fromSurface(env,surface);
        if (nativeWindow){
//            nativeWindow 设置分辨率和图像格式
            ANativeWindow_setBuffersGeometry(nativeWindow,width,height,WINDOW_FORMAT_RGBA_8888);

        }
    }else{
        //如果suface 为空，稀放旧的nativeWindow
        if (nativeWindow){
            ANativeWindow_release(nativeWindow);
            nativeWindow =0;
        }
    }


}
}
