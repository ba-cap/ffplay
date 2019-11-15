#ifndef _SCOPED_LOCAL_REF_H_
#define _SCOPED_LOCAL_REF_H_

#include "JNIHelp.h"

#include <jni.h>

// A smart pointer that deletes a JNI local reference when it goes out of scope.
template<typename T>
class ScopedLocalRef
{
public:
    ScopedLocalRef(JNIEnv *env, T localRef) : mEnv(env), mLocalRef(localRef)
    {
    }

    ~ScopedLocalRef()
    {
        reset();
    }

    void reset(T ptr = nullptr)
    {
        if (ptr != mLocalRef)
        {
            if (mLocalRef != nullptr)
            {
                mEnv->DeleteLocalRef(mLocalRef);
            }
            mLocalRef = ptr;
        }
    }

    T release() __attribute__((warn_unused_result))
    {
        T localRef = mLocalRef;
        mLocalRef = nullptr;
        return localRef;
    }

    T get() const
    {
        return mLocalRef;
    }

    // Some better C++11 support.
#if __cplusplus >= 201103L
    // Move constructor.
    ScopedLocalRef(ScopedLocalRef&& s) : mEnv(s.mEnv), mLocalRef(s.release())
    {
    }

    explicit ScopedLocalRef(JNIEnv *env) : mEnv(env), mLocalRef(nullptr)
    {
    }

    // We do not expose an empty constructor as it can easily lead to errors
    // using common idioms, e.g.:
    //   ScopedLocalRef<...> ref;
    //   ref.reset(...);

    // Move assignment operator.
    ScopedLocalRef& operator=(ScopedLocalRef&& s)
    {
        reset(s.release());
        mEnv = s.mEnv;
        return *this;
    }

    // Allows "if (scoped_ref == nullptr)"
    bool operator==(std::nullptr_t) const
    {
        return mLocalRef == nullptr;
    }

    // Allows "if (scoped_ref != nullptr)"
    bool operator!=(std::nullptr_t) const
    {
        return mLocalRef != nullptr;
    }
#endif

private:
    JNIEnv *mEnv;
    T       mLocalRef;

    DISALLOW_COPY_AND_ASSIGN(ScopedLocalRef);
};


#endif//_SCOPED_LOCAL_REF_H_
