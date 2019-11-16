#include "ALogger.h"
#include "JniConstants.h"
#include "ScopedLocalRef.h"


#include <cstdlib>
#include <atomic>
#include <mutex>


static const char *tag = "JniConstants";

static std::atomic<bool> g_constants_initialized(false);
static std::mutex g_constants_mutex;

jclass JniConstants::booleanClass;
jclass JniConstants::byteArrayClass;
jclass JniConstants::calendarClass;
jclass JniConstants::charsetICUClass;
jclass JniConstants::doubleClass;
jclass JniConstants::errnoExceptionClass;
jclass JniConstants::fileDescriptorClass;
jclass JniConstants::gaiExceptionClass;
jclass JniConstants::inet6AddressClass;
jclass JniConstants::inet6AddressHolderClass;
jclass JniConstants::inetAddressClass;
jclass JniConstants::inetAddressHolderClass;
jclass JniConstants::inetSocketAddressClass;
jclass JniConstants::inetSocketAddressHolderClass;
jclass JniConstants::integerClass;
jclass JniConstants::localeDataClass;
jclass JniConstants::longClass;
jclass JniConstants::netlinkSocketAddressClass;
jclass JniConstants::packetSocketAddressClass;
jclass JniConstants::patternSyntaxExceptionClass;
jclass JniConstants::referenceClass;
jclass JniConstants::socketTaggerClass;
jclass JniConstants::stringClass;
jclass JniConstants::structAddrinfoClass;
jclass JniConstants::structFlockClass;
jclass JniConstants::structGroupReqClass;
jclass JniConstants::structIfaddrs;
jclass JniConstants::structLingerClass;
jclass JniConstants::structPasswdClass;
jclass JniConstants::structPollfdClass;
jclass JniConstants::structStatClass;
jclass JniConstants::structStatVfsClass;
jclass JniConstants::structTimevalClass;
jclass JniConstants::structTimespecClass;
jclass JniConstants::structUcredClass;
jclass JniConstants::structUtsnameClass;
jclass JniConstants::unixSocketAddressClass;
jclass JniConstants::zipEntryClass;

static jclass findClass(JNIEnv *env, const char *name)
{
    ScopedLocalRef<jclass> localClass(env, env->FindClass(name));
    jclass result = reinterpret_cast<jclass>(env->NewGlobalRef(localClass.get()));
    if (result == NULL)
    {
        ALOGE(tag, "failed to find class '%s'", name);
        abort();
    }
    return result;
}

void JniConstants::init(JNIEnv *env)
{
    // Fast check
    if (g_constants_initialized)
    {
        // already initialized
        return;
    }

    // Slightly slower check
    std::lock_guard<std::mutex> guard(g_constants_mutex);
    if (g_constants_initialized)
    {
        // already initialized
        return;
    }

    booleanClass                 = findClass(env, "java/lang/Boolean");
    byteArrayClass               = findClass(env, "[B");
    calendarClass                = findClass(env, "java/util/Calendar");
    charsetICUClass              = findClass(env, "java/nio/charset/CharsetICU");
    doubleClass                  = findClass(env, "java/lang/Double");
    errnoExceptionClass          = findClass(env, "android/system/ErrnoException");
    fileDescriptorClass          = findClass(env, "java/io/FileDescriptor");
    gaiExceptionClass            = findClass(env, "android/system/GaiException");
    inet6AddressClass            = findClass(env, "java/net/Inet6Address");
    inet6AddressHolderClass      = findClass(env, "java/net/Inet6Address$Inet6AddressHolder");
    inetAddressClass             = findClass(env, "java/net/InetAddress");
    inetAddressHolderClass       = findClass(env, "java/net/InetAddress$InetAddressHolder");
    inetSocketAddressClass       = findClass(env, "java/net/InetSocketAddress");
    inetSocketAddressHolderClass = findClass(env, "java/net/InetSocketAddress$InetSocketAddressHolder");
    integerClass                 = findClass(env, "java/lang/Integer");
    localeDataClass              = findClass(env, "libcore/icu/LocaleData");
    longClass                    = findClass(env, "java/lang/Long");
    netlinkSocketAddressClass    = findClass(env, "android/system/NetlinkSocketAddress");
    packetSocketAddressClass     = findClass(env, "android/system/PacketSocketAddress");
    patternSyntaxExceptionClass  = findClass(env, "java/util/regex/PatternSyntaxException");
    referenceClass               = findClass(env, "java/lang/ref/Reference");
    socketTaggerClass            = findClass(env, "dalvik/system/SocketTagger");
    stringClass                  = findClass(env, "java/lang/String");
    structAddrinfoClass          = findClass(env, "android/system/StructAddrinfo");
    structFlockClass             = findClass(env, "android/system/StructFlock");
    structGroupReqClass          = findClass(env, "android/system/StructGroupReq");
    structIfaddrs                = findClass(env, "android/system/StructIfaddrs");
    structLingerClass            = findClass(env, "android/system/StructLinger");
    structPasswdClass            = findClass(env, "android/system/StructPasswd");
    structPollfdClass            = findClass(env, "android/system/StructPollfd");
    structStatClass              = findClass(env, "android/system/StructStat");
    structStatVfsClass           = findClass(env, "android/system/StructStatVfs");
    structTimevalClass           = findClass(env, "android/system/StructTimeval");
    structTimespecClass          = findClass(env, "android/system/StructTimespec");
    structUcredClass             = findClass(env, "android/system/StructUcred");
    structUtsnameClass           = findClass(env, "android/system/StructUtsname");
    unixSocketAddressClass       = findClass(env, "android/system/UnixSocketAddress");
    zipEntryClass                = findClass(env, "java/util/zip/ZipEntry");

    g_constants_initialized = true;
}
