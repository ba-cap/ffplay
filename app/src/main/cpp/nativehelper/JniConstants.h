#ifndef JNI_CONSTANTS_H_
#define JNI_CONSTANTS_H_

#include <jni.h>


struct JniConstants
{
    static void init(JNIEnv *env);

    static jclass booleanClass;
    static jclass byteArrayClass;
    static jclass calendarClass;
    static jclass charsetICUClass;
    static jclass doubleClass;
    static jclass errnoExceptionClass;
    static jclass fileDescriptorClass;
    static jclass gaiExceptionClass;
    static jclass inet6AddressClass;
    static jclass inet6AddressHolderClass;
    static jclass inetAddressClass;
    static jclass inetAddressHolderClass;
    static jclass inetSocketAddressClass;
    static jclass inetSocketAddressHolderClass;
    static jclass integerClass;
    static jclass localeDataClass;
    static jclass longClass;
    static jclass netlinkSocketAddressClass;
    static jclass packetSocketAddressClass;
    static jclass patternSyntaxExceptionClass;
    static jclass referenceClass;
    static jclass socketTaggerClass;
    static jclass stringClass;
    static jclass structAddrinfoClass;
    static jclass structFlockClass;
    static jclass structGroupReqClass;
    static jclass structIfaddrs;
    static jclass structLingerClass;
    static jclass structPasswdClass;
    static jclass structPollfdClass;
    static jclass structStatClass;
    static jclass structStatVfsClass;
    static jclass structTimevalClass;
    static jclass structTimespecClass;
    static jclass structUcredClass;
    static jclass structUtsnameClass;
    static jclass unixSocketAddressClass;
    static jclass zipEntryClass;
};


#endif//JNI_CONSTANTS_H_
