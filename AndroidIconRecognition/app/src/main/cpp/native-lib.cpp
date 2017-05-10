#include <jni.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>



using namespace cv;
using namespace std;




extern "C"{

string filename = "trainedSVM.xml";
vector<string> iconClass;
Ptr<ml::SVM> svm;



JNIEXPORT void JNICALL
Java_com_rosie_androidiconrecognition_MainActivity_loadSVM(JNIEnv *env, jobject, jstring filepath) {

    const char* nativefile = env -> GetStringUTFChars(filepath, JNI_FALSE);
    svm = ml::SVM::create();
    svm = ml::SVM::load(nativefile);

    static const string arr[] = {    "back",
                                      "close",
                                      "delete",
                                      "download",
                                      "edit",
                                      "home",
                                      "info",
                                      "love",
                                      "menu",
                                      "minus",
                                      "plus",
                                      "profile",
                                      "search",
                                      "settings",
                                      "share",
                                      "shoppingBag",
                                      "shoppingCart",
                                      "unknown" };

    iconClass = vector<string> (arr, arr + sizeof(arr)/sizeof(arr[0]));

}

JNIEXPORT jstring JNICALL
Java_com_rosie_androidiconrecognition_MainActivity_predictIcon(JNIEnv *env, jobject instance,
                                                               jlong inputaddr) {

    Mat &input = *(Mat *) inputaddr;
    // preprocessing
    resize(input, input, Size(24,24), 0, 0, CV_INTER_LANCZOS4);
    Mat gray_img;
    cvtColor(input, gray_img, CV_RGB2GRAY);

    HOGDescriptor d(Size(24, 24), Size(8, 8), Size(4, 4), Size(4, 4), 9);

    vector<float> descriptorsValues;
    vector<Point> locations;
    d.compute(gray_img, descriptorsValues, Size(0, 0), Size(0, 0), locations);

    int row = 1, col = descriptorsValues.size();

    Mat M(row, col, CV_32F);
    memcpy(&(M.data[0]), descriptorsValues.data(), col * sizeof(float));

    // prediction
    int result = (int) svm->predict(M);
    const char* cresult = iconClass[result].c_str();

    return env->NewStringUTF(cresult);
}


}
