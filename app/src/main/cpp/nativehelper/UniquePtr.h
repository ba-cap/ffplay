#ifndef _UNIQUE_PTR_H_
#define _UNIQUE_PTR_H_

#include "JNIHelp.h"

#include <cstdlib>

// Default deleter for pointer types.
template<typename T>
struct DefaultDelete
{
    enum
    {
        type_must_be_complete = sizeof(T)
    };

    DefaultDelete() {}

    void operator()(T *p) const
    {
        delete p;
    }
};

// Default deleter for array types.
template<typename T>
struct DefaultDelete<T[]>
{
    enum
    {
        type_must_be_complete = sizeof(T)
    };

    void operator()(T *p) const
    {
        delete[] p;
    }
};


// A smart pointer that deletes the given pointer on destruction.
// Equivalent to C++0x's std::unique_ptr (a combination of boost::scoped_ptr
// and boost::scoped_array).
// Named to be in keeping with Android style but also to avoid
// collision with any other implementation, until we can switch over
// to unique_ptr.
// Use thus:
//   UniquePtr<C> c(new C);
template<typename T, typename D = DefaultDelete<T> >
class UniquePtr
{
public:
    // Construct a new UniquePtr, taking ownership of the given raw pointer.
    explicit UniquePtr(T *ptr = nullptr) : mPtr(ptr)
    {
    }

    ~UniquePtr()
    {
        reset();
    }

    // Accessors.
    T& operator*() const
    {
        return *mPtr;
    }

    T *operator->() const
    {
        return mPtr;
    }

    T *get() const
    {
        return mPtr;
    }

    // Returns the raw pointer and hands over ownership to the caller.
    // The pointer will not be deleted by UniquePtr.
    T *release() __attribute__((warn_unused_result))
    {
        T *result = mPtr;
        mPtr = nullptr;
        return result;
    }

    // Takes ownership of the given raw pointer.
    // If this smart pointer previously owned a different raw pointer, that
    // raw pointer will be freed.
    void reset(T *ptr = nullptr)
    {
        if (ptr != mPtr)
        {
            D()(mPtr);
            mPtr = ptr;
        }
    }

private:
    // The raw pointer.
    T *mPtr;

    // Comparing unique pointers is probably a mistake, since they're unique.
    template<typename T2>
    bool operator==(const UniquePtr<T2>& p) const;

    template<typename T2>
    bool operator!=(const UniquePtr<T2>& p) const;

    DISALLOW_COPY_AND_ASSIGN(UniquePtr);
};

// Partial specialization for array types. Like std::unique_ptr, this removes
// operator* and operator-> but adds operator[].
template<typename T, typename D>
class UniquePtr<T[], D>
{
public:
    explicit UniquePtr(T *ptr = nullptr) : mPtr(ptr)
    {
    }

    ~UniquePtr()
    {
        reset();
    }

    T& operator[](size_t i) const
    {
        return mPtr[i];
    }

    T *get() const
    {
        return mPtr;
    }

    T *release() __attribute__((warn_unused_result))
    {
        T *result = mPtr;
        mPtr = nullptr;
        return result;
    }

    void reset(T *ptr = nullptr)
    {
        if (ptr != mPtr)
        {
            D()(mPtr);
            mPtr = ptr;
        }
    }

private:
    T *mPtr;

    DISALLOW_COPY_AND_ASSIGN(UniquePtr);
};


#endif//_UNIQUE_PTR_H_
