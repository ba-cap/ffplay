//
//   author: patrick.dai
// datetime: 2019-12-02 12:23
//
#ifndef _XLOG_H_INCLUDE
#define _XLOG_H_INCLUDE

#include <android/log.h>


class XLog
{

};

#define XLOGD( ... ) __android_log_print(ANDROID_LOG_DEBUG, "xplay", __VA_ARGS__)

#define XLOGI( ... ) __android_log_print(ANDROID_LOG_INFO, "xplay", __VA_ARGS__)

#define XLOGW( ... ) __android_log_print(ANDROID_LOG_WARN, "xplay", __VA_ARGS__)

#define XLOGE( ... ) __android_log_print(ANDROID_LOG_ERROR, "xplay", __VA_ARGS__)


#endif//_XLOG_H_INCLUDE
