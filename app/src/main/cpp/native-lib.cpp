#include <jni.h>
#include <string>
#include <math.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>

#include <android/bitmap.h>
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "error", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))

using namespace cv;
using namespace std;

#define TAG "FIND_OBJ"
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)

bool isNormal(const vector<vector<Point>> &contours, int i);

jobject cpp2java(JNIEnv *env,std::vector<std::string> vector) {

    static jclass java_util_ArrayList;
    static jmethodID java_util_ArrayList_;
    jmethodID java_util_ArrayList_size;
    jmethodID java_util_ArrayList_get;
    jmethodID java_util_ArrayList_add;

    java_util_ArrayList      = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
    java_util_ArrayList_     = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
    java_util_ArrayList_size = env->GetMethodID (java_util_ArrayList, "size", "()I");
    java_util_ArrayList_get  = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
    java_util_ArrayList_add  = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");


    jobject result = env->NewObject(java_util_ArrayList, java_util_ArrayList_, vector.size());
    for (std::string s: vector) {
        jstring element = env->NewStringUTF(s.c_str());
        env->CallBooleanMethod(result, java_util_ArrayList_add, element);
        env->DeleteLocalRef(element);
    }
    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_tfkj_opencv3_MainActivity_find_1objects(JNIEnv *env, jclass type, jstring imagePath_) {
    const char *imagePath = env->GetStringUTFChars(imagePath_, 0);

    LOGD("start find-----------------%s", imagePath);

    Mat image = imread(imagePath);
    Mat dst, dst1;
    cvtColor(image, dst, COLOR_BGR2BGRA);


    vector<Mat> channels;
    split(dst, channels);

    for (int i = 0; i < channels[3].rows; i++){
        for (int j = 0; j < channels[3].cols; j++){
            channels[3].at<uchar>(i, j) = 0;
        }
    }

    merge(channels, dst1);

    //imwrite("/Users/apple/Desktop/dst1.png", dst1);

    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);

    Mat thresh;
    threshold(gray, thresh, 0, 255, THRESH_BINARY_INV|THRESH_OTSU);

    imwrite("/storage/emulated/0/Download/bin.png", thresh);

    morphologyEx(thresh, thresh, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(15, 15)), Point(-1,-1), 2);
    //morphologyEx(thresh, thresh, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(3, 3)), Point(-1,-1), 2);

    vector<vector<Point>> contours;
    findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    LOGD("contours.size() = %u\n", contours.size());

    int count = 0;
    vector<string> result;
    for (int i = 0; i< contours.size(); i++) {
        LOGD("contourArea = %f\n", contourArea(contours[i]));
        if (isNormal(contours, i)) {
            continue;
        }

        for (int k = 0; k < dst1.rows; k++){
            for (int j = 0; j < dst1.cols; j++){
                if (pointPolygonTest(contours[i], Point2f(j, k), false) > 0) {
                    dst1.at<Vec4b>(k, j)[3] = 255;
                }

            }
        }

        Rect rect = boundingRect(contours[i]);
        //rectangle(image, rect, Scalar(0, 0, 255), 1);

        ostringstream stream;
        stream << i;
        count ++;
        string path = "/storage/emulated/0/Download/" + stream.str() + ".png";
        result.push_back(path);

        imwrite(path, dst1(rect));
    }

    env->ReleaseStringUTFChars(imagePath_, imagePath);

    return cpp2java(env, result);
}

bool isNormal(const vector<vector<Point>> &contours, int i) { return contourArea(contours[i]) < 1000; }

//===================================================================================================================

void BitmapToMat2(JNIEnv *env, jobject& bitmap, Mat& mat, jboolean needUnPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    Mat &dst = mat;

    try {
        LOGD("nBitmapToMat");
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC4);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGD("nBitmapToMat: RGBA_8888 -> CV_8UC4");
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (needUnPremultiplyAlpha) cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
            else tmp.copyTo(dst);
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            LOGD("nBitmapToMat: RGB_565 -> CV_8UC4");
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

void BitmapToMat(JNIEnv *env, jobject& bitmap, Mat& mat) {
    BitmapToMat2(env, bitmap, mat, false);
}

void MatToBitmap2
        (JNIEnv *env, Mat& mat, jobject& bitmap, jboolean needPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    Mat &src = mat;

    try {
        LOGD("nMatToBitmap");
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
                LOGD("nMatToBitmap: CV_8UC1 -> RGBA_8888");
                cvtColor(src, tmp, COLOR_GRAY2RGBA);
            } else if (src.type() == CV_8UC3) {
                LOGD("nMatToBitmap: CV_8UC3 -> RGBA_8888");
                cvtColor(src, tmp, COLOR_RGB2RGBA);
            } else if (src.type() == CV_8UC4) {
                LOGD("nMatToBitmap: CV_8UC4 -> RGBA_8888");
                if (needPremultiplyAlpha)
                    cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                else
                    src.copyTo(tmp);
            }
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if (src.type() == CV_8UC1) {
                LOGD("nMatToBitmap: CV_8UC1 -> RGB_565");
                cvtColor(src, tmp, COLOR_GRAY2BGR565);
            } else if (src.type() == CV_8UC3) {
                LOGD("nMatToBitmap: CV_8UC3 -> RGB_565");
                cvtColor(src, tmp, COLOR_RGB2BGR565);
            } else if (src.type() == CV_8UC4) {
                LOGD("nMatToBitmap: CV_8UC4 -> RGB_565");
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

void MatToBitmap(JNIEnv *env, Mat& mat, jobject& bitmap) {
    MatToBitmap2(env, mat, bitmap, false);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tfkj_opencv3_FloodFillUtils_floodFill(JNIEnv *env, jobject instance, jobject bitmap,
                                               jint x, jint y, jint low, jint up) {

    Mat mat_image_src ;
    BitmapToMat(env,bitmap,mat_image_src);//图片转化成mat
    Mat mat_image_dst;
    blur(mat_image_src, mat_image_dst, Size2i(10,10));
    //第四步：转成java数组->更新
    MatToBitmap(env,mat_image_dst,bitmap);//mat转成化图片

}