#ifndef JNIHELP_H_
#define JNIHELP_H_

#if !defined(DISALLOW_COPY_AND_ASSIGN)
#   if __cplusplus >= 201103L
#       define DISALLOW_COPY_AND_ASSIGN(TypeName) \
            TypeName(const TypeName&) = delete;  \
            void operator=(const TypeName&) = delete
#   else
#       define DISALLOW_COPY_AND_ASSIGN(TypeName) \
            TypeName(const TypeName&);  \
            void operator=(const TypeName&)
#   endif  //__has_feature(cxx_deleted_functions)
#endif  // !defined(DISALLOW_COPY_AND_ASSIGN)


#include <jni.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Register one or more native methods with a particular class.
 * "className" looks like "java/lang/String". Aborts on failure.
 * TODO: fix all callers and change the return type to void.
 */
int jniRegisterNativeMethods(C_JNIEnv *env, const char *className, const JNINativeMethod *gMethods, int numMethods);

/*
 * Throw an exception with the specified class and an optional message.
 *
 * The "className" argument will be passed directly to FindClass, which
 * takes strings with slashes (e.g. "java/lang/Object").
 *
 * If an exception is currently pending, we log a warning message and
 * clear it.
 *
 * Returns 0 on success, nonzero if something failed (e.g. the exception
 * class couldn't be found, so *an* exception will still be pending).
 *
 * Currently aborts the VM if it can't throw the exception.
 */
int jniThrowException(C_JNIEnv *env, const char *className, const char *msg);

/*
 * Throw a java.lang.NullPointerException, with an optional message.
 */
int jniThrowNullPointerException(C_JNIEnv *env, const char *msg);

/*
 * Throw a java.lang.RuntimeException, with an optional message.
 */
int jniThrowRuntimeException(C_JNIEnv *env, const char *msg);

/*
 * Throw a java.io.IOException, generating the message from errno.
 */
int jniThrowIOException(C_JNIEnv *env, int errnum);

/*
 * Return a pointer to a locale-dependent error string explaining errno
 * value 'errnum'. The returned pointer may or may not be equal to 'buf'.
 * This function is thread-safe (unlike strerror) and portable (unlike
 * strerror_r).
 */
const char *jniStrError(int errnum, char *buf, size_t buflen);

/*
 * Returns a new java.io.FileDescriptor for the given int fd.
 */
jobject jniCreateFileDescriptor(C_JNIEnv *env, int fd);

/*
 * Returns the int fd from a java.io.FileDescriptor.
 */
int jniGetFDFromFileDescriptor(C_JNIEnv *env, jobject fileDescriptor);

/*
 * Sets the int fd in a java.io.FileDescriptor.
 */
void jniSetFileDescriptorOfFD(C_JNIEnv *env, jobject fileDescriptor, int value);

/*
 * Returns the reference from a java.lang.ref.Reference.
 */
jobject jniGetReferent(C_JNIEnv *env, jobject ref);

/*
 * Returns a Java String object created from UTF-16 data either from jchar or,
 * if called from C++11, char16_t (a bitwise identical distinct type).
 */
jstring jniCreateString(C_JNIEnv *env, const jchar *unicodeChars, jsize len);

/*
 * Log a message and an exception.
 * If exception is NULL, logs the current exception in the JNI environment.
 */
void jniLogException(C_JNIEnv *env, int priority, const char *tag, jthrowable exception);

#ifdef __cplusplus
}
#endif



/*
 * For C++ code, we provide inlines that map to the C functions.  g++ always
 * inlines these, even on non-optimized builds.
 */
#if defined(__cplusplus)
inline int jniRegisterNativeMethods(JNIEnv *env, const char *className, const JNINativeMethod *gMethods, int numMethods)
{
    return jniRegisterNativeMethods(&env->functions, className, gMethods, numMethods);
}

inline int jniThrowException(JNIEnv *env, const char *className, const char *msg)
{
    return jniThrowException(&env->functions, className, msg);
}

extern "C" int jniThrowExceptionFmt(C_JNIEnv *env, const char *className, const char *fmt, va_list args);

/*
 * Equivalent to jniThrowException but with a printf-like format string and
 * variable-length argument list. This is only available in C++.
 */
inline int jniThrowExceptionFmt(JNIEnv *env, const char *className, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return jniThrowExceptionFmt(&env->functions, className, fmt, args);
    va_end(args);
}

inline int jniThrowNullPointerException(JNIEnv *env, const char *msg)
{
    return jniThrowNullPointerException(&env->functions, msg);
}

inline int jniThrowRuntimeException(JNIEnv *env, const char *msg)
{
    return jniThrowRuntimeException(&env->functions, msg);
}

inline int jniThrowIOException(JNIEnv *env, int errnum)
{
    return jniThrowIOException(&env->functions, errnum);
}

inline jobject jniCreateFileDescriptor(JNIEnv *env, int fd)
{
    return jniCreateFileDescriptor(&env->functions, fd);
}

inline int jniGetFDFromFileDescriptor(JNIEnv *env, jobject fileDescriptor)
{
    return jniGetFDFromFileDescriptor(&env->functions, fileDescriptor);
}

inline void jniSetFileDescriptorOfFD(JNIEnv *env, jobject fileDescriptor, int value)
{
    jniSetFileDescriptorOfFD(&env->functions, fileDescriptor, value);
}

inline jobject jniGetReferent(JNIEnv *env, jobject ref)
{
    return jniGetReferent(&env->functions, ref);
}

inline jstring jniCreateString(JNIEnv *env, const jchar *unicodeChars, jsize len)
{
    return jniCreateString(&env->functions, unicodeChars, len);
}
#endif

#if __cplusplus >= 201103L
inline jstring jniCreateString(JNIEnv *env, const char16_t *unicodeChars, jsize len)
{
    return jniCreateString(&env->functions, reinterpret_cast<const jchar *>(unicodeChars), len);
}
#endif  // __cplusplus >= 201103L

inline void jniLogException(JNIEnv *env, int priority, const char *tag, jthrowable exception = NULL)
{
    jniLogException(&env->functions, priority, tag, exception);
}


#endif//JNIHELP_H_
