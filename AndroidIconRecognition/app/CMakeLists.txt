
cmake_minimum_required(VERSION 3.4.1)

set(pathOpenCv C:/OpenCV-android-sdk)
set(pathProject D:/AndroidProject/AndroidIconRecognition/AndroidIconRecognition)
set(pathLibOpenCv_JAVA ${pathProject}/app/src/main/JniLibs/${ANDROID_ABI}/libopencv_java3.so)

include_directories(${pathOpenCv}/sdk/native/jni/include)

add_library( lib_opencv SHARED IMPORTED )
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${pathLibOpenCv_JAVA})

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

find_library( log-lib log)
find_library( android-lib android)

target_link_libraries( # Specifies the target library.
                       native-lib

                       lib_opencv

                      # Links the target library to the log library
                      # included in the NDK.
                      ${log-lib}
                      ${android-lib}

                      )