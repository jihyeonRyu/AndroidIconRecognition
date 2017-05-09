#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

extern "C"{

string filename = "trainedSVM.xml";
Ptr<ml::SVM> svm;

JNIEXPORT void JNICALL
Java_com_rosie_androidiconrecognition_MainActivity_loadSVM(JNIEnv *env, jobject obj, jstring filepath) {

    const char* nativefile = env -> GetStringUTFChars(filepath, JNI_FALSE);
    svm = ml::SVM::create();
    svm = ml::SVM::load(nativefile);
}

JNIEXPORT void JNICALL
java_com_rosie_androidiconrecognition_MainActivity_predictIcon(JNIEnv *env, jobject obj){

}

}
