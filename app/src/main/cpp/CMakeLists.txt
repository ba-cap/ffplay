
cmake_minimum_required(VERSION 3.4.1)


## include directory
include_directories(
        include/ffmpeg
        nativehelper
        opensles
)

## shared library directory
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${ANDROID_ABI})

## message(WARNING "library directory ${LIBS_DIR} ")


####################################################################################################
# ffmpeg library
####################################################################################################

## libavcodec.so
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libavcodec.so)

# ## libavdevice.so
# add_library(avdevice SHARED IMPORTED)
# set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libavdevice.so)

## libavfilter.so
add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libavfilter.so)

## libavformat.so
add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libavformat.so)

## libavutil.so
add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libavutil.so)

# ## libpostproc.so
# add_library(postproc SHARED IMPORTED)
# set_target_properties(postproc PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libpostproc.so)

## libswresample.so
add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libswresample.so)

## libswscale.so
add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/libswscale.so)


####################################################################################################
# local JNI NDK nativehelper
####################################################################################################
add_library(
        nativehelper

        SHARED

        nativehelper/ALogger.h
        nativehelper/JniConstants.cpp
        nativehelper/JniConstants.h
        nativehelper/JNIHelp.cpp
        nativehelper/JNIHelp.h
        nativehelper/ScopedLocalRef.h
        nativehelper/ScopedStringChars.h
        nativehelper/toStringArray.h
        nativehelper/UniquePtr.h
)

target_link_libraries(
        nativehelper
        log
)


####################################################################################################
# local JNI NDK ffplay1.0
####################################################################################################

add_library(
        ffplay1.0

        SHARED

        opensles/OpenAudio.h
        opensles/OpenAudio.cpp
        jni_AudioPlayer.cpp
        jni_demo.cpp
)


target_link_libraries(
        ffplay1.0

        # ffmpeg
        avcodec avfilter avformat avutil swresample swscale

        # android
        log android OpenSLES

        #
        nativehelper
)



