#include <sys/types.h>
#include <jni.h>

extern void play_audio_pcm(const u_char *data, size_t size);

extern void play_video_yuv(JNIEnv *env, jobject surface, const char *path);

extern void play_video_yuv_book(JNIEnv *env, jobject surface, const char *path);