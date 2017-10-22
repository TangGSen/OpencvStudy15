#include <jni.h>
#include "opencv2/opencv.hpp"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <android/bitmap.h>

#define LOG_TAG    "sensen"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)
using namespace cv;
using namespace std;
CascadeClassifier *classifier;
ANativeWindow *nativeWindow;


#define TEMPLE_WIDTH 0.24
#define TEMPLE_HEIGHT 0.07
#define DEFAULT_CARD_WIDTH 640
#define DEFAULT_CARD_HEIGHT 400
#define  FIX_IDCARD_SIZE Size(DEFAULT_CARD_WIDTH,DEFAULT_CARD_HEIGHT)
#define FIX_TEMPLATE_SIZE  Size(153, 28)

extern "C" {
extern JNIEXPORT void JNICALL Java_org_opencv_android_Utils_nBitmapToMat2
        (JNIEnv *env, jclass, jobject bitmap, jlong m_addr, jboolean needUnPremultiplyAlpha);
extern JNIEXPORT void JNICALL Java_org_opencv_android_Utils_nMatToBitmap
        (JNIEnv *env, jclass, jlong m_addr, jobject bitmap);

/**
 * 开始识别
 */
 jobject crateJavaBitmap(JNIEnv *env, Mat mat, jobject config) {
    int imageWith = mat.cols;
    int imageHeight = mat.rows;
    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMid = env->GetStaticMethodID(bitmapClass, "createBitmap",
                                                       "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jobject jbitmap = env->CallStaticObjectMethod(bitmapClass, createBitmapMid, imageWith,
                                                  imageHeight, config);
    Java_org_opencv_android_Utils_nMatToBitmap(env, bitmapClass, (jlong) &mat, jbitmap);
    return jbitmap;
}


JNIEXPORT jobject JNICALL
Java_sen_com_opencvstudy15_OpencvUtils_getIdCardNum(JNIEnv *env, jclass type, jobject srcBitmap,
                                                    jobject templeBitmap, jobject config) {
    //原图
    Mat src_image;
    //灰度图 需要拿去跟模板匹配
    Mat image_gray;
    //二值化，进行轮廓检测
    Mat image_threshold;
    //Gaussian 图片
    Mat image_gaussion;
    //Canny 图片
    Mat image_canny;
    //模板图片
    Mat image_tpl;
    //获取身份证图
    Mat image_idCard;
    //获取身份证号图
    Mat image_idNumber;

    //将java中的bitamap 转Mat
    Java_org_opencv_android_Utils_nBitmapToMat2
            (env, type, srcBitmap, (jlong) &src_image, 0);
    Java_org_opencv_android_Utils_nBitmapToMat2
            (env, type, templeBitmap, (jlong) &image_tpl, 0);

    cvtColor(src_image, image_gray, COLOR_BGRA2GRAY);
    //可以将中间产物，输出
    //imwrite("sdcard/gray.png",image_gray);
    //二值化,将大于195 都变成255
    threshold(image_gray, image_threshold, 195, 255, THRESH_TRUNC);

    //高斯模糊让图片变的更加平滑
    GaussianBlur(image_threshold, image_gaussion, Size(3, 3), 0);

    //canny 将边缘进行增强
    Canny(image_gaussion, image_canny, 180, 255);

    vector<vector<Point> > contours;
    vector<Vec4i> hierachy;
    //l轮廓检测：只检测外轮廓，并压缩水平方向，垂直方向，对角方向的元素
    //只保留改方向的终点坐标，比如矩形就是存储四个点
    //第一个是原图
    //第二个是获取轮廓的点的保存向量，是找到了各个轮廓
    //第三个是层次结构，存放了轮廓同一等级前后的索引，不同等级的父亲轮廓和孩子轮廓的索引
    // 第四个参数 mode :CV_RETR_EXTERNAL找到的轮廓里面没有小轮廓（洞），CV_RETR_LIST找到的轮廓中可以包括小轮廓
    // http://blog.csdn.net/corcplusplusorjava/article/details/20536251
    findContours(image_canny, contours, hierachy, RETR_LIST, CHAIN_APPROX_SIMPLE);
    int width = src_image.cols >> 1;
    int height = src_image.rows >> 1;

    vector<Rect> roiArea;
    Rect rectMin;
    for (int i = 0; i < contours.size(); ++i) {
        vector<Point> v = contours.at(i);
        Rect rect = boundingRect(v);
        rectangle(image_threshold, rect, Scalar(255, 255, 255));
        if (rect.width >= width && rect.height >= height) {
            roiArea.push_back(rect);
        }
    }

    if (roiArea.size() > 0) {
        rectMin = roiArea.at(0);
        for (int i = 0; i < roiArea.size(); ++i) {
            Rect temp = roiArea.at(0);
            if (temp.area() < rectMin.area()) {
                rectMin = temp;
            }
        }
    } else {
        rectMin = Rect(0, 0, image_gray.cols, image_gray.rows);
    }

    image_idCard = image_gray(rectMin);
//    imwrite("sdcard/image_idcard.png", image_idCard);
    resize(image_idCard, image_idCard, FIX_IDCARD_SIZE);
    resize(image_tpl, image_tpl, FIX_TEMPLATE_SIZE);

    int cols = image_idCard.cols - image_tpl.cols + 1;
    int rows = image_idCard.rows - image_tpl.rows + 1;
    cvtColor(image_tpl, image_tpl, COLOR_BGRA2GRAY);
    //创建输出图像，输出图像的宽度 = 被查找图像的宽度 - 模版图像的宽度 + 1
//    Mat match(rows, cols, CV_32F);
//        TM_SQDIFF 平方差匹配法
//        TM_CCORR 相关匹配法
//        TM_CCOEFF 相关系数匹配法
//        TM_SQDIFF_NORMED
//        TM_CCORR_NORMED
//        TM_CCOEFF_NORMED
    // 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
    Mat match;
    matchTemplate(image_idCard, image_tpl, match, TM_CCORR_NORMED);
    //归一化
    normalize(match, match, 0, 1, NORM_MINMAX, -1);

    Point maxLoc;
    minMaxLoc(match, 0, 0, 0, &maxLoc);
    ////计算 [身份证(模版):号码区域]
    //号码区域:
    //x: 身份证(模版)的X+宽
    //y: 身份证(模版)Y
    //w: 全图宽-(身份证(模版)X+身份证(模版)宽) - n(给个大概值)
    //h: 身份证(模版)高
    Rect rect(maxLoc.x + image_tpl.cols + 10,
              maxLoc.y - 15,
              image_idCard.cols - (maxLoc.x + image_tpl.cols) - 50,
              image_tpl.rows + 20);
    image_idNumber = image_idCard(rect);
//    imwrite("/sdcard/image_idNumber.png", image_idNumber);
    jobject result = crateJavaBitmap(env, image_idNumber, config);

    src_image.release();
    image_canny.release();
    image_gaussion.release();
    image_gray.release();
    image_idNumber.release();
    image_threshold.release();
    image_tpl.release();
    match.release();

    return result;
}
}
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
    LOGE("ANative:%s", path);
    env->ReleaseStringUTFChars(path_, path);
}

JNIEXPORT void JNICALL
Java_sen_com_opencvstudy15_OpencvUtils_handleBitmap(JNIEnv *env, jobject instance, jobject bitmap) {
    Mat src;
    //将bitmap 装换成opencv 可以识别的格式
    bitmap2Mat(env, bitmap, src, true);
    //将图像转变为灰度图像
    if (classifier) {
        vector<Rect> faces;

        Mat grayMat;
        cvtColor(src, grayMat, CV_BGR2GRAY);
        //直线图均衡化，增强对比效果
        equalizeHist(grayMat, grayMat);
        //开始识别，并将识别到的脸部区域写入faces 向量中
        classifier->detectMultiScale(grayMat, faces);


        grayMat.release();
        int size = faces.size();
        for (int i = 0; i < size; i++) {
            Rect face = faces[i];
            rectangle(src, face.tl(), face.br(), Scalar(255, 0, 255));
        }
    }

    if (!nativeWindow) {
        LOGE("native windown null");
        goto end;
    }
    LOGE("ANativeWindow_Buffer");
    //绘制到android
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(nativeWindow, &window_buffer, 0)) {
        //加锁成功返回0，失败为1
        LOGE("native windown lock fail");
        goto end;
    };
    //转换成Android 有透明度的图片
    cvtColor(src, src, CV_BGR2BGRA);
    //jni 层 opencv 图像压缩
    resize(src, src, Size(window_buffer.width, window_buffer.height));
    //将数据复制到window_buffer
    memcpy(window_buffer.bits, src.data, window_buffer.width * window_buffer.height * 4);

    ANativeWindow_unlockAndPost(nativeWindow);

    end:
    src.release();


}

JNIEXPORT void JNICALL
Java_sen_com_opencvstudy15_OpencvUtils_setSurface(JNIEnv *env, jobject instance, jobject surface,
                                                  jint width, jint height) {
    if (surface) {
        if (nativeWindow) {
            ANativeWindow_release(nativeWindow);
            nativeWindow = 0;
        }
        nativeWindow = ANativeWindow_fromSurface(env, surface);
        if (nativeWindow) {
//            nativeWindow 设置分辨率和图像格式
            ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);

        }
    } else {
        //如果suface 为空，稀放旧的nativeWindow
        if (nativeWindow) {
            ANativeWindow_release(nativeWindow);
            nativeWindow = 0;
        }
    }


}
}
