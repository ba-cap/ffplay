#ifndef SCOPED_LOCAL_REF_H_included
#define SCOPED_LOCAL_REF_H_included

#include <jni.h>
#include <cstdlib>

#include "JNIHelp.h"

class ScopedStringChars
{
public:
    ScopedStringChars(JNIEnv *env, jstring s) : env_(env), string_(s), size_(0)
    {
        if (s == nullptr)
        {
            chars_ = nullptr;
            jniThrowNullPointerException(env, nullptr);
        }
        else
        {
            chars_ = env->GetStringChars(string_, nullptr);
            if (chars_ != nullptr)
            {
                size_ = env->GetStringLength(string_);
            }
        }
    }

    ~ScopedStringChars()
    {
        if (chars_ != nullptr)
        {
            env_->ReleaseStringChars(string_, chars_);
        }
    }

    const jchar *get() const
    {
        return chars_;
    }

    size_t size() const
    {
        return size_;
    }

    const jchar& operator[](size_t n) const
    {
        return chars_[n];
    }

private:
    JNIEnv *const env_;
    const jstring string_;
    const jchar  *chars_;
    size_t        size_;

    DISALLOW_COPY_AND_ASSIGN(ScopedStringChars);
};


#endif//SCOPED_LOCAL_REF_H_included
