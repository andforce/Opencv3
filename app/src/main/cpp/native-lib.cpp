#include <jni.h>
#include <string>
#include <math.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
using namespace cv;
using namespace std;

#define TAG "lm"
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)

extern "C" {
    JNIEXPORT void JNICALL Java_com_tfkj_opencv3_MainActivity_gray(JNIEnv *env, jclass type) {
        Mat image = imread("/sdcard/101/rock.png");
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

        morphologyEx(thresh, thresh, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(15, 15)), Point(-1,-1), 2);
        //morphologyEx(thresh, thresh, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(3, 3)), Point(-1,-1), 2);

        vector<vector<Point>> contours;
        findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        printf("contours.size() = %lu\n", contours.size());

        for (int i = 0; i< contours.size(); i++) {
            printf("contourArea = %f\n", contourArea(contours[i]));
            if (contourArea(contours[i]) < 10000) {
                continue;
            }

            for (int k = 0; k < dst1.rows; k++){
                for (int j = 0; j < dst1.cols; j++){
                    if (pointPolygonTest(contours[i], Point2f(j, k), false) > 0) {
                        dst1.at<Vec4b>(k, j)[3] = 255;
                    }

                }
            }
        }

        for (int i = 0; i< contours.size(); i++) {
            if (contourArea(contours[i]) < 10000) {
                continue;
            }

            Rect rect = boundingRect(contours[i]);
            //rectangle(image, rect, Scalar(0, 0, 255), 1);

            ostringstream stream;
            stream << i;
            imwrite("/sdcard/101/" + stream.str() + ".png", dst1(rect));
        }

        imwrite("/sdcard/101/dst1.png", dst1);
    }
}