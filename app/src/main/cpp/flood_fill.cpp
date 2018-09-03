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
Mat createAlphaFromMask(Mat &mask) {
    Mat alpha = Mat::zeros(mask.rows, mask.cols, CV_8UC1);
    //Mat gray = Mat::zeros(mask.rows, mask.cols, CV_8UC1);

    //cvtColor(mask, gray, COLOR_RGB2GRAY);

    for (int i = 0; i < mask.rows; i++) {
        for (int j = 0; j < mask.cols; j++) {
            alpha.at<uchar>(i, j) = static_cast<uchar>(255 - mask.at<uchar>(i, j));
        }
    }

    return alpha;
}

int addAlpha(Mat &src, Mat &dst, Mat &alpha) {
    if (src.channels() == 4) {
        return -1;
    } else if (src.channels() == 1) {
        cvtColor(src, src, COLOR_GRAY2RGB);
    }

    dst = Mat(src.rows, src.cols, CV_8UC4);

    vector<Mat> srcChannels;
    vector<Mat> dstChannels;
    //分离通道
    split(src, srcChannels);

    dstChannels.push_back(srcChannels[0]);
    dstChannels.push_back(srcChannels[1]);
    dstChannels.push_back(srcChannels[2]);
    //添加透明度通道
    dstChannels.push_back(alpha);
    //合并通道
    merge(dstChannels, dst);

    return 0;
}

Mat removeChannel(Mat &src, int which){
    vector<Mat> channels;
    split(src, channels);

    for (int i = 0; i < channels[which].rows; i++) {
        for (int j = 0; j < channels[which].cols; j++) {
            channels[which].at<uchar>(i, j) = 0;
        }
    }

    Mat dst;
    merge(channels, dst);
    return dst;
}


/**
 * Android Bitmap ARGB
 * OpenCV CV_8UC4 默认是RGBA
 * flood识别的是BGR
 * 所以要把RGBA-->BGR
 */

Mat srcBGR;
Mat maskGray;
static bool DEBUG = false;

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_tfkj_opencv3_FloodFillUtils_floodFillBitmapWithMask(JNIEnv *env, jclass type,
                                                             jobject bitmap, jobject maskBitmap,
                                                             jint x, jint y,
                                                             jint low, jint up) {

    LOGD("start()");

    // 把Bitmap转成Mat
    Mat srcRGBA;
    BitmapToMat(env, bitmap, srcRGBA, CV_8UC4);

    //转换成BGR
    cvtColor(srcRGBA, srcBGR, COLOR_RGBA2BGR);


    Rect fillRect;

    maskGray.create(srcBGR.rows + 2, srcBGR.cols + 2, CV_8UC1);
    maskGray = Scalar::all(0);

    int flags = 4 | FLOODFILL_MASK_ONLY | FLOODFILL_FIXED_RANGE | (255 << 8);

    //    InputOutputArray:输入和输出图像。
    //    mask:            输入的掩码图像。
    //    seedPoint：      算法开始处理的开始位置。
    //    newVal：         图像中所有被算法选中的点，都用这个数值来填充。
    //    rect:            最小包围矩阵。
    //    loDiff：         最大的低亮度之间的差异。
    //    upDiff：         最大的高亮度之间的差异。
    //    flag：           选择算法连接方式。
    int area = floodFill(srcBGR, maskGray, Point(x, y), Scalar(0, 0, 255), &fillRect,
                         Scalar(low, low, low),
                         Scalar(up, up, up), flags);


    LOGD("floodFill() fill pixel count: %d", area);

    Mat sizeCorrect = range(maskGray);
    Mat alpha = createAlphaFromMask(sizeCorrect);
    Mat resultMat;
    addAlpha(srcBGR, resultMat, alpha);

    if (DEBUG) {
        saveMat2File(resultMat, "mergedResultRGBA.png");
    }

    // 把mask转成Bitmap返回到Java层
    Mat maskRGBA;
    cvtColor(maskGray, maskRGBA, COLOR_GRAY2RGBA);

    sizeCorrect = range(maskRGBA);

    MatToBitmap2(env, sizeCorrect, maskBitmap, static_cast<jboolean>(false), CV_8UC4);

    // 返回int[]
    int size = resultMat.rows * resultMat.cols;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, (jint *)resultMat.data);
    return result;
}