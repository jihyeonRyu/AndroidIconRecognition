#include <jni.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <android/log.h>


using namespace cv;
using namespace std;

#define TAG "native-lib"

extern "C"{

class iconRecog {
private:
    int background = 0;
public:
    string filename = "trainedSVM.xml";
    static vector<string> iconClass;
    static Ptr<ml::SVM> svm;
    Mat square(Mat &originMat);
    Mat crop(Mat &originMat);
};

// initialize the static variable
vector<string> iconRecog::iconClass;
Ptr<ml::SVM> iconRecog::svm;

JNIEXPORT void JNICALL
Java_com_rosie_androidiconrecognition_MainActivity_loadSVM(JNIEnv *env, jobject, jstring filepath) {

    iconRecog mIconRecog;

    const char* nativefile = env -> GetStringUTFChars(filepath, JNI_FALSE);
    mIconRecog.svm = ml::SVM::create();
    mIconRecog.svm = ml::SVM::load(nativefile);

    static const string arr[] = { "back",
                                    "close",
                                    "(delete)|(remove)|(throwing out)",
                                    "download",
                                    "(edit)|(write)",
                                    "(home)|(shopping mall)",
                                    "information",
                                    "(prefer)|(like)",
                                    "(menu)|(list)",
                                    "minus",
                                    "(plus)|(grid)",
                                    "(my page)|(profile)" ,
                                    "search",
                                    "settings",
                                    "share",
                                    "shopping bag",
                                    "shopping cart",
                                    "unknown" }; // size 18

    mIconRecog.iconClass = vector<string> (arr, arr + sizeof(arr)/sizeof(arr[0]));
    __android_log_print(ANDROID_LOG_VERBOSE, TAG, "load SVM complete!");

}

JNIEXPORT jstring JNICALL
Java_com_rosie_androidiconrecognition_MainActivity_predictIcon(JNIEnv *env, jobject instance,
                                                               jlong inputaddr) {
    try{

        iconRecog mIconRecog;

        Mat &input = *(Mat *) inputaddr;
        Mat processed_img;

        vector<float> descriptorsValues;
        vector<Point> locations;

        ///////////////// image preprocessing ///////////////

        cvtColor(input, processed_img, CV_RGB2GRAY);

        processed_img = mIconRecog.crop(processed_img);

        processed_img = mIconRecog.square(processed_img);

        resize(processed_img, processed_img, Size(32, 32), 0, 0, CV_INTER_LANCZOS4);

        /////////////// prediction ////////////////

        HOGDescriptor d(Size(32, 32), Size(16, 16), Size(8, 8), Size(8, 8), 9);

        d.compute(processed_img, descriptorsValues, Size(0, 0), Size(0, 0), locations);

        int row = 1, col = descriptorsValues.size();

        Mat M(row, col, CV_32FC1); // channel 1
        memcpy(&(M.data[0]), descriptorsValues.data(), col * sizeof(float));

        int result = (int) mIconRecog.svm->predict(M);

        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "prediction complete!");

        if(result >= 0 && result < mIconRecog.iconClass.size()){
            const char* cresult;
            cresult = mIconRecog.iconClass[result].c_str();
            return env->NewStringUTF(cresult);
        } else
            return NULL;

    } catch(Exception e){

        env-> ExceptionDescribe();
        env-> ExceptionClear();
        return NULL;
    }
}


Mat iconRecog::square(Mat &originMat) {

    int col = originMat.cols;
    int row = originMat.rows;

    if (col == row) {
        return originMat;
    }

    int move1, move2;
    int margin = 10;
    Mat stride1, stride2, dst;

    if (col > row) {

        move1 = (int)((col - row + margin) / 2);
        move2 = (int)margin / 2;
        stride1 = (Mat_<float>(2, 3) << 1, 0, 0, 0, 1, move1); // move to down
        stride2 = (Mat_<float>(2, 3) << 1, 0, move2, 0, 1, 0); // move to right

        dst = Mat::zeros(Size(col+margin, col+margin), originMat.type());

    }else if (row > col) {

        move1 = (int)((row - col + margin) / 2);
        move2 = (int)margin / 2;

        stride1 = (Mat_<float>(2, 3) << 1, 0, move1, 0, 1, 0); // move to right
        stride2 = (Mat_<float>(2, 3) << 1, 0, 0, 0, 1, move2); // move to down

        dst = Mat::zeros(Size(row+margin, row+margin), originMat.type());

    }

    warpAffine(originMat, dst, stride1, dst.size(), INTER_LANCZOS4, BORDER_CONSTANT, cv::Scalar(background));
    warpAffine(dst, dst, stride2, dst.size(), INTER_LANCZOS4, BORDER_CONSTANT, cv::Scalar(background));

    __android_log_print(ANDROID_LOG_VERBOSE, TAG, "square complete");

    return dst;
}

Mat iconRecog::crop(Mat &originMat){

    // originMat is grayScale
    uchar *data = originMat.data;
    vector<pair<int, int> > firstIndex;

    int row = originMat.rows;
    int col = originMat.cols;
    int threshold = 30;

    int current, before;

    //////////// left to right scanning ///////////
    for (int i = 0; i < row; i++) {

        for (int j = 0; j < col; j++) {
            if (j == 0) {
                before = (int)data[i*col + j];
            }
            current = (int) data[i*col + j];

            if (abs(current - before) > threshold) {
                background = before;
                int r = i;
                int c = j;
                pair<int, int> pil(r,c);
                firstIndex.push_back(pil);

                break;
            }

            before = current;

        }

    }

    // find min col index
    int minCol = col;
    int minRow;

    for (int i = 0; i < firstIndex.size(); i++) {

        int c = firstIndex.at(i).second;

        if (c < minCol) {
            minCol = c;
        }
    }

    int leftCol = minCol;

    firstIndex.clear();

    ///////////// top to bottom scanning ////////////

    for (int i = 0; i < col; i++) {

        for (int j = 0; j < row; j++) {

            if (j == 0) {
                before = (int)data[j*col + i];
            }

            current = (int) data[j*col + i];

            if (abs(current - before) > threshold) {

                int r = (int) (j*col + i) / col;
                int c = (int)(j*col + i) % col;
                pair<int, int> pil(r,c);
                firstIndex.push_back(pil);
                break;
            }

            before = current;
        }

    }

    // find min row index

    minRow = row;

    for (int i = 0; i < firstIndex.size(); i++) {

        int r = firstIndex.at(i).first;

        if (r < minRow) {
            minRow = r;
        }

    }

    int leftRow = minRow;

    firstIndex.clear();

    ////////////// right to left scanning /////////////

    for (int i = 0; i < row; i++) {

        for (int j = col-1; j >= 0 ; j--) {
            if (j == col-1) {
                before = (int)data[i*col + j];
            }
            current = (int) data[i*col + j];

            if (abs(current - before) > threshold) {

                int r = (int)(i*col + j) / col;
                int c = (int)(i*col + j) % col;
                pair<int, int> pil(r,c);
                firstIndex.push_back(pil);

                break;
            }

            before = current;
        }

    }

    // find max col index

    int maxCol = 0;
    int maxRow = 0;

    for (int i = 0; i < firstIndex.size(); i++) {

        int c = firstIndex.at(i).second;

        if (maxCol < c) {
            maxCol = c;
        }
    }

    int rightCol = maxCol;

    firstIndex.clear();

    ////////////// bottom to up scannig ////////////

    for (int i = 0; i < col; i++) {


        for (int j = row-1; j >= 0 ; j--) {
            if (j == row-1) {
                before = (int)data[col*j + i];
            }

            current = (int) data[col*j + i];

            if (abs(current - before) > threshold) {

                int r = (int)(col*j + i) / col;
                int c = (int)(col*j + i) % col;
                pair<int, int> pil(r,c);
                firstIndex.push_back(pil);
                break;
            }

            before = current;
        }

    }

    // find max row index

    maxRow = 0;

    for (int i = 0; i < firstIndex.size(); i++) {

        int r = firstIndex.at(i).first;

        if (maxRow < r) {
            maxRow = r;
        }
    }

    int rightRow = maxRow;

    // 관심영역 설정 (set ROI (X, Y, W, H)).

    if (leftCol == col || leftRow == row || rightCol - leftCol <= 0 || rightRow - leftRow <= 0 ) {
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, "cannot crop");
        return originMat;
    }

    Rect rect;
    rect = Rect(leftCol, leftRow, rightCol - leftCol, rightRow - leftRow);

    Mat cropMat = originMat(rect);
    __android_log_print(ANDROID_LOG_VERBOSE, TAG, "crop complete");
    return cropMat;

}


}
