#ifndef _A_LOOGER_H_
#define _A_LOOGER_H_

#include <android/log.h>

#ifndef ALOG
#   ifdef NDK_DEBUG
#       define ALOG(priority, tag, fmt...) __android_log_print(ANDROID_##priority, tag, fmt)
#   else
#       define ALOG(...)   ((void)0)
#   endif
#endif//ALOG


#ifndef ALOGV
#   define ALOGV(tag, fmt...) ((void)ALOG(LOG_VERBOSE, tag, fmt))
#endif

#ifndef ALOGD
#   define ALOGD(tag, fmt...) ((void)ALOG(LOG_DEBUG, tag, fmt))
#endif

#ifndef ALOGI
#   define ALOGI(tag, fmt...) ((void)ALOG(LOG_INFO, tag, fmt))
#endif

#ifndef ALOGW
#   define ALOGW(tag, fmt...) ((void)ALOG(LOG_WARN, tag, fmt))
#endif

#ifndef ALOGE
#   define ALOGE(tag, fmt...) ((void)ALOG(LOG_ERROR, tag, fmt))
#endif


#endif//_A_LOOGER_H_
