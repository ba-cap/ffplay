//
//   author: patrick.dai
// datetime: 2019-12-03 11:45
//

#include "XEGL.h"
#include "XLog.h"

#include <android/native_window.h>
#include <EGL/egl.h>

class CXEGL: public XEGL
{
private:
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mSurface = EGL_NO_SURFACE;
    EGLContext mContext = EGL_NO_CONTEXT;

public:
    virtual bool init(void *win)
    {
        ANativeWindow *n_win = (ANativeWindow *)win;

        // 初始化 EGL
        // 1. 获取 EGLDisplay 对象{显示设备}
        //    如果横竖屏时会重新创建的
        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if(mDisplay == EGL_NO_DISPLAY)
        {
            XLOGE("eglGetDisplay failed");
            return false;
        }
        XLOGI("eglGetDisplay success");

        // 2. 初始化 display
        if( EGL_TRUE != eglInitialize(mDisplay, nullptr, nullptr))
        {
            XLOGE("eglInitialize failed");
            return false;
        }
        XLOGI("eglInitialize success");

        // 3. 配置并创建 surface
        EGLint config_spec[] =
        {
            EGL_RED_SIZE,     8,
            EGL_GREEN_SIZE,   8,
            EGL_BLUE_SIZE,    8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
        };
        EGLConfig config     = 0;
        EGLint    config_num = 0;
        if(EGL_TRUE != eglChooseConfig(mDisplay, config_spec, &config, 1, &config_num))
        {
            XLOGE("eglChooseConfig failed");
            return false;
        }
        XLOGI("eglChooseConfig success");

        mSurface = eglCreateWindowSurface(mDisplay, config, n_win, nullptr);
        if(EGL_NO_SURFACE == mSurface)
        {
            XLOGE("eglCreateWindowSurface failed");
            return false;
        }
        XLOGI("eglCreateWindowSurface success");

        // 4. 创建并打开上下文
        const EGLint contextAttribute[] =
        {
            EGL_CONTEXT_CLIENT_VERSION,
            2,
            EGL_NONE
        };
        mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, contextAttribute);
        if(mContext == EGL_NO_CONTEXT)
        {
            XLOGE("eglCreateContext failed");
            return false;
        }
        XLOGI("eglCreateContext success");


        if( EGL_TRUE != eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) )
        {
            XLOGE("eglMakeCurrent failed");
            return false;
        }
        XLOGI("eglMakeCurrent success");

        return true;
    }

    virtual void draw()
    {
        if(EGL_NO_DISPLAY == mDisplay)
        {
            XLOGE("No EGLDisplay in CXEGL::draw()");
            return;
        }

        if(EGL_NO_SURFACE == mSurface)
        {
            XLOGE("No EGLSurface in CXEGL::draw()");
            return;
        }

        eglSwapBuffers(mDisplay, mSurface);
    }
};


XEGL *XEGL::get()
{
    static CXEGL *cxegl = new CXEGL;
    return cxegl;
}



