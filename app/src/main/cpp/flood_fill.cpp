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
    Mat temp = img(Range(1, m - 1), Range(1, n - 1));
    return temp;
}


void saveMat2File(Mat &src, string file) {
    string path = "/storage/emulated/0/Download/" + file;
    imwrite(path, src);
}

/**
 * mask 需要是CV_8UC1
 * @param mask
 * @return
 */
cv::Mat createAlphaFromMask(cv::Mat &mask) {
    cv::Mat alpha = cv::Mat::zeros(mask.rows, mask.cols, CV_8UC1);
    //cv::Mat gray = cv::Mat::zeros(mask.rows, mask.cols, CV_8UC1);

    //cv::cvtColor(mask, gray, cv::COLOR_RGB2GRAY);

    for (int i = 0; i < mask.rows; i++) {
        for (int j = 0; j < mask.cols; j++) {
            alpha.at<uchar>(i, j) = static_cast<uchar>(255 - mask.at<uchar>(i, j));
        }
    }

    return alpha;
}

int addAlpha(cv::Mat &src, cv::Mat &dst, cv::Mat &alpha) {
    if (src.channels() == 4) {
        return -1;
    } else if (src.channels() == 1) {
        cv::cvtColor(src, src, cv::COLOR_GRAY2RGB);
    }

    dst = cv::Mat(src.rows, src.cols, CV_8UC4);

    std::vector<cv::Mat> srcChannels;
    std::vector<cv::Mat> dstChannels;
    //分离通道
    cv::split(src, srcChannels);

    dstChannels.push_back(srcChannels[0]);
    dstChannels.push_back(srcChannels[1]);
    dstChannels.push_back(srcChannels[2]);
    //添加透明度通道
    dstChannels.push_back(alpha);
    //合并通道
    cv::merge(dstChannels, dst);

    return 0;
}


/**
 * Android Bitmap ARGB
 * OpenCV CV_8UC4 默认是RGBA
 * flood识别的是BGR
 * 所以要把RGBA-->BGR
 */

Mat srcBGR;
Mat maskGray;
int FILLMODE = 1;
int g_nNewMaskVal = 255;

extern "C"
JNIEXPORT jint JNICALL
Java_com_tfkj_opencv3_FloodFillUtils_floodFillBitmap(JNIEnv *env, jclass type, jobject bitmap,
                                                     jobject maskBitmap, jobject  resultBitmap, jint x, jint y, jint low,
                                                     jint up) {

    LOGD("start()");
    // 把Bitmap转成Mat
    Mat srcRGBA;
    BitmapToMat(env, bitmap, srcRGBA, CV_8UC4);
    Mat maskRGBA;
//    BitmapToMat(env, maskBitmap, maskRGBA, CV_8UC4);

    //转换成BGR
    cvtColor(srcRGBA, srcBGR, CV_RGBA2BGR);
//    cvtColor(maskRGBA,maskGray,CV_RGBA2BGR);


    int lowDifference = FILLMODE == 0 ? 0 : low;
    int UpDifference = FILLMODE == 0 ? 0 : up;

    int b = (unsigned) 0;
    int g = (unsigned) 0;
    int r = (unsigned) 255;


    Rect fillRect;
    Scalar newVal = Scalar(b, g, r);
    Point mSeedPoint = Point(x, y);
//    int area = floodFill(srcBGR, mSeedPoint, newVal, &fillRect,
//                     Scalar(lowDifference, lowDifference, lowDifference),
//                     Scalar(UpDifference, UpDifference, UpDifference), flags);

    maskGray.create(srcBGR.rows + 2, srcBGR.cols + 2, CV_8UC1);
    maskGray = Scalar::all(0);

    //threshold(maskGray, maskGray, 1, 128, THRESH_BINARY);
    int flags = 4 | FLOODFILL_MASK_ONLY | FLOODFILL_FIXED_RANGE | (g_nNewMaskVal << 8);
    //g_nConnectivity + (g_nNewMaskVal << 8) + (FILLMODE == 1 ? FLOODFILL_FIXED_RANGE : 0);


    //    InputOutputArray:输入和输出图像。
    //    mask:            输入的掩码图像。
    //    seedPoint：      算法开始处理的开始位置。
    //    newVal：         图像中所有被算法选中的点，都用这个数值来填充。
    //    rect:            最小包围矩阵。
    //    loDiff：         最大的低亮度之间的差异。
    //    upDiff：         最大的高亮度之间的差异。
    //    flag：           选择算法连接方式。
    int area = floodFill(srcBGR, maskGray, mSeedPoint, newVal, &fillRect,
                         Scalar(lowDifference, lowDifference, lowDifference),
                         Scalar(UpDifference, UpDifference, UpDifference), flags);

    if (true){

        Mat sizeCorrect;

        if (false){
            sizeCorrect = range(maskGray);

            Mat resultRGBA;
            srcRGBA.copyTo(resultRGBA);

            vector<Mat> channels;
            split(resultRGBA, channels);

            for (int i = 0; i < channels[3].rows; i++) {
                for (int j = 0; j < channels[3].cols; j++) {
                    if (i <= channels[3].rows / 2){
                        channels[3].at<uchar>(i, j) = 0;
                    } else{
                        channels[3].at<uchar>(i, j) = 255;
                    }
                }
            }

            Mat mergedResultRGBA;
            merge(channels, mergedResultRGBA);

            saveMat2File(mergedResultRGBA, "mergedResultRGBA.png");

            MatToBitmap2(env, mergedResultRGBA, resultBitmap, static_cast<jboolean>(false), CV_8UC4);
        } else {
            sizeCorrect = range(maskGray);
            Mat alpha = createAlphaFromMask(sizeCorrect);
            Mat resultMat;
            addAlpha(srcBGR, resultMat, alpha);
            saveMat2File(resultMat, "mergedResultRGBA.png");
            MatToBitmap2(env, resultMat, resultBitmap, static_cast<jboolean>(false), CV_8UC4);
        }




        // 把mask转成Bitmap返回到Java层
        cvtColor(maskGray, maskRGBA, CV_GRAY2RGBA);

        sizeCorrect = range(maskRGBA);

        MatToBitmap2(env, sizeCorrect, maskBitmap, static_cast<jboolean>(false), CV_8UC4);
    } else {
        // DEBUG
        saveMat2File(maskGray, "maskGray.jpg");

        LOGD("flood fill count is: %d", area);

        // 转换成RGBA
        // cvtColor(maskGray,maskRGBA,CV_BGR2RGBA);
        cvtColor(maskGray, maskRGBA, CV_GRAY2RGBA);
        LOGD("cvtColor()");

        // 转成Bitmap
        Mat adjust = range(maskRGBA);

//    // 反色操作
//    bitwise_not(adjust,adjust);
//
//    Mat inversedMat = 255 - adjust;
//
//    path = "/storage/emulated/0/Download/fanse.jpg";
//    imwrite(path,inversedMat);
//
////    bitwise_not(adjust, srcRGBA);
//
//    path = "/storage/emulated/0/Download/src1.jpg";
//    imwrite(path,inversedMat);
//
////    Mat result;
////    srcRGBA.copyTo(result, inversedMat);
////
////    MatToBitmap(env, result, maskBitmap, CV_8UC4);

        MatToBitmap2(env, adjust, maskBitmap, static_cast<jboolean>(true), CV_8UC4);
    }
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
    cvtColor(srcMat, bgra, CV_RGBA2BGRA);
    //转换成BGR
    cvtColor(srcMat, bgr, CV_RGBA2BGR);

    srcMat = bgr;

    string path = "/storage/emulated/0/Download/src1.jpg";
    imwrite(path, srcMat);

//    MatToBitmap(env, srcMat, bitmap, CV_8UC3);
//
//    if (true){
//        return;
//    }

    srcMat.copyTo(dstMat);


    string dstPath = "/storage/emulated/0/Download/dst.jpg";
    imwrite(dstPath, dstMat);

    maskMat.create(srcMat.rows + 2, srcMat.cols + 2, CV_8UC1);

    Point mSeedPoint = Point(x, y);

    LOGD("start find-----------------%d, %d", x, y);

    int lowDifference = FILLMODE == 0 ? 0 : low;
    int UpDifference = FILLMODE == 0 ? 0 : up;

//    int b = (unsigned) theRNG() & 255;
//    int g = (unsigned) theRNG() & 255;
//    int r = (unsigned) theRNG() & 255;

    int b = (unsigned) 0;
    int g = (unsigned) 0;
    int r = (unsigned) 255;


    Rect ccomp;
    Scalar newVal = Scalar(b, g, r);
    Mat dst = dstMat;//目标图的赋值

    //int flags = g_nConnectivity + (g_nNewMaskVal << 8) + (FILLMODE == 1 ? FLOODFILL_FIXED_RANGE : 0);

    int flags = 4 | FLOODFILL_FIXED_RANGE | (g_nNewMaskVal << 8);

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
        imwrite(path, dst);
    }

    LOGD("有多少个点被重画-----------------%d", area);

    Mat show;
    cvtColor(dst, dst, CV_BGR2RGBA);

    MatToBitmap2(env, dst, bitmap, static_cast<jboolean>(true), CV_8UC4);

//    Mat mat_image_src ;
//    BitmapToMat(env,bitmap,mat_image_src);//图片转化成mat
//    Mat mat_image_dst;
//    blur(mat_image_src, mat_image_dst, Size2i(10,10));
//    //第四步：转成java数组->更新
//    MatToBitmap(env,mat_image_dst,bitmap);//mat转成化图片

}