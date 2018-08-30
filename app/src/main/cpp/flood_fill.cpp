//
// Created by pc on 8/30/18.
//

#include <jni.h>
#include <string>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>

#include "mat_map_cvt.h"

using namespace cv;
using namespace std;


Mat range(Mat &img) {
    int m = img.rows;
    int n = img.cols;
    Mat temp = img(Range(1,m - 1), Range(1,n -1));
    return temp;
}

/**
 * Android Bitmap ARGB
 * OpenCV CV_8UC4 默认是RGBA
 * flood识别的是BGR
 * 所以要把RGBA-->BGR
 */

Mat srcBGR;
Mat maskBGR;
int FILLMODE = 1;
int g_nNewMaskVal = 255;

extern "C"
JNIEXPORT jint JNICALL
Java_com_tfkj_opencv3_FloodFillUtils_floodFillBitmap(JNIEnv *env, jclass type, jobject bitmap,
                                                     jobject maskBitmap, jint x, jint y, jint low,
                                                     jint up) {

    LOGD("start()");
    // 把Bitmap转成Mat
    Mat srcRGBA;
    BitmapToMat(env, bitmap, srcRGBA, CV_8UC4);
    Mat maskRGBA;
//    BitmapToMat(env, maskBitmap, maskRGBA, CV_8UC4);

    //转换成BGR
    cvtColor(srcRGBA,srcBGR,CV_RGBA2BGR);
//    cvtColor(maskRGBA,maskBGR,CV_RGBA2BGR);


    int lowDifference = FILLMODE == 0 ? 0 : low;
    int UpDifference = FILLMODE == 0 ? 0 : up;

    int b = (unsigned) 0;
    int g = (unsigned) 0;
    int r = (unsigned) 255;


    Rect ccomp;
    Scalar newVal = Scalar(b, g, r);
    Point mSeedPoint = Point(x, y);
//    int area = floodFill(srcBGR, mSeedPoint, newVal, &ccomp,
//                     Scalar(lowDifference, lowDifference, lowDifference),
//                     Scalar(UpDifference, UpDifference, UpDifference), flags);

    maskBGR.create(srcBGR.rows + 2, srcBGR.cols + 2, CV_8UC1);
    //threshold(maskBGR, maskBGR, 1, 128, THRESH_BINARY);
    int flags = 8 | FLOODFILL_MASK_ONLY | FLOODFILL_FIXED_RANGE | (g_nNewMaskVal << 8) ;
    //g_nConnectivity + (g_nNewMaskVal << 8) + (FILLMODE == 1 ? FLOODFILL_FIXED_RANGE : 0);
    int area = floodFill(srcBGR, maskBGR, mSeedPoint, newVal, &ccomp,
                         Scalar(lowDifference, lowDifference, lowDifference),
                         Scalar(UpDifference, UpDifference, UpDifference), flags);



    string path = "/storage/emulated/0/Download/maskBGR.jpg";
    imwrite(path,maskBGR);

    LOGD("有多少个点被重画-----------------%d", area);

    // 转换成RGBA
//    cvtColor(maskBGR,maskRGBA,CV_BGR2RGBA);
    cvtColor(maskBGR,maskRGBA, CV_GRAY2RGBA);
    LOGD("cvtColor()");

    // 转成Bitmap
    Mat adjust = range(maskRGBA);
    MatToBitmap(env, adjust, maskBitmap, CV_8UC4);

    return area;
}

Mat srcMat;
Mat dstMat;
Mat maskMat;

int g_nConnectivity = 4;
bool g_bUseMask = false;

extern "C"
JNIEXPORT void JNICALL
Java_com_tfkj_opencv3_FloodFillUtils_floodFill(JNIEnv *env, jobject instance, jobject bitmap,
                                               jint x, jint y, jint low, jint up) {

    BitmapToMat(env, bitmap, srcMat, CV_8UC4);

    Mat bgra;
    Mat bgr;

    //转换成BGRA
    cvtColor(srcMat,bgra,CV_RGBA2BGRA);
    //转换成BGR
    cvtColor(srcMat,bgr,CV_RGBA2BGR);

    srcMat = bgr;

    string path = "/storage/emulated/0/Download/src1.jpg";
    imwrite(path,srcMat);

//    MatToBitmap(env, srcMat, bitmap, CV_8UC3);
//
//    if (true){
//        return;
//    }

    srcMat.copyTo(dstMat);


    string dstPath = "/storage/emulated/0/Download/dst.jpg";
    imwrite(dstPath,dstMat);

    maskMat.create(srcMat.rows + 2, srcMat.cols + 2, CV_8UC1);

    Point mSeedPoint = Point(x, y);

    LOGD("start find-----------------%d, %d", x, y);

    int lowDifference = FILLMODE == 0 ? 0 : low;
    int UpDifference = FILLMODE == 0 ? 0 : up;
    int flags =
            g_nConnectivity + (g_nNewMaskVal << 8) + (FILLMODE == 1 ? FLOODFILL_FIXED_RANGE : 0);

//    int b = (unsigned) theRNG() & 255;
//    int g = (unsigned) theRNG() & 255;
//    int r = (unsigned) theRNG() & 255;

    int b = (unsigned) 0;
    int g = (unsigned) 0;
    int r = (unsigned) 255;


    Rect ccomp;
    Scalar newVal = Scalar(b, g, r);
    Mat dst = dstMat;//目标图的赋值
    int area;
    if (g_bUseMask) {


        threshold(maskMat, maskMat, 1, 128, THRESH_BINARY);
        area = floodFill(dst, maskMat, mSeedPoint, newVal, &ccomp,
                         Scalar(lowDifference, lowDifference, lowDifference),
                         Scalar(UpDifference, UpDifference, UpDifference), flags);

    } else {
        LOGD("start find-----------------floodFill flags: %d", flags);

        area = floodFill(dst, mSeedPoint, newVal, &ccomp,
                         Scalar(lowDifference, lowDifference, lowDifference),
                         Scalar(UpDifference, UpDifference, UpDifference), flags);

        string path = "/storage/emulated/0/Download/555.jpg";
        imwrite(path,dst);
    }

    LOGD("有多少个点被重画-----------------%d", area);

    Mat show;
    cvtColor(dst,dst,CV_BGR2RGBA);

    MatToBitmap(env, dst, bitmap, CV_8UC4);

//    Mat mat_image_src ;
//    BitmapToMat(env,bitmap,mat_image_src);//图片转化成mat
//    Mat mat_image_dst;
//    blur(mat_image_src, mat_image_dst, Size2i(10,10));
//    //第四步：转成java数组->更新
//    MatToBitmap(env,mat_image_dst,bitmap);//mat转成化图片

}