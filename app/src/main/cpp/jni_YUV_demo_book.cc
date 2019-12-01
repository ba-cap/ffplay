
#include "demo.h"
#include "ALogger.h"

#include <jni.h>
#include <string>
#include <cstring>
#include <EGL/egl.h>

//#include <GLES3/gl3.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/log.h>
#include <android/native_window_jni.h>

static const char *tag = "jni_yuv_b";

static const char *str_vertex_shader =
    "attribute vec4 aPosition;\n"  // 顶点坐标
    "attribute vec2 aTexCoord;\n"  // 材质顶点坐标
    "varying   vec2 vTexCoord;\n"  // 输出材质坐标
    "void main() {\n"
    "    vTexCoord   = vec2( aTexCoord.x, 1.0 - aTexCoord.y );\n"
    "    gl_Position = aPosition;\n"
    "}\n";

static const char *str_fragment_yuv420p_shader =
    "precision mediump float;\n"     // float 精度
    "varying vec2      vTexCoord;\n" // 顶点着色器传递过来的坐标
    "uniform sampler2D yTexture;\n"  // 输入材质 (不透明灰度, 单像素)
    "uniform sampler2D uTexture;\n"
    "uniform sampler2D vTexture;\n"
    "void main() {\n"
    "    vec3  yuv;\n"
    "    vec3  rgb;\n"
    "    yuv.r = texture2D(yTexture, vTexCoord).r;\n"
    "    yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;\n"
    "    yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;\n"
    "    rgb   = mat3(1.0,          1.0, 1.0, "
    "                 0.0,     -0.39465, 2.03211, "
    "                 1.13983, -0.58060, 0.0) * yuv; "
    "    gl_FragColor = vec4(rgb, 1.0); "  // 输出像素颜色
    "}";

// 三维顶点数据
static float vertex_pointer[] = {
    1.0F, -1.0F, 0.0F,
    -1.0F, -1.0F, 0.0F,
    1.0F, 1.0F, 0.0F,
    -1.0F, 1.0F, 0.0F
};

// 材质坐标
static float texture_pointer[] = {
    1.0F, 0.0F,  // right bottom
    0.0F, 0.0F,
    1.0F, 1.0F,
    0.0F, 1.0F
};


GLint InitShader(const char *code, GLint type)
{
    //创建shader
    GLint sh = glCreateShader(type);
    if(sh == 0) {
        ALOGD(tag, "glCreateShader %d failed!", type);
        return 0;
    }
    //加载shader
    glShaderSource(sh,
                   1,    //shader数量
                   &code, //shader代码
                   0);   //代码长度
    //编译shader
    glCompileShader(sh);

    //获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if(status == 0) {
        ALOGD(tag, "glCompileShader failed!");
        return 0;
    }
    ALOGD(tag, "glCompileShader success!");
    return sh;
}




void play_video_yuv_book(JNIEnv *env, jobject surface, const char *path)
{
    ALOGD(tag, "play yuv file: %s.", path);

    FILE *fp = fopen(path, "rb");
    if(!fp) {
        ALOGD( tag, "open file %s failed!", path);
        return;
    }


    //1 获取原始窗口
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);

    ////////////////////
    ///EGL
    //1 EGL display创建和初始化
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY) {
        ALOGD( tag, "eglGetDisplay failed!");
        return;
    }
    if(EGL_TRUE != eglInitialize(display, 0, 0)) {
        ALOGD( tag, "eglInitialize failed!");
        return;
    }
    //2 surface
    //2-1 surface窗口配置
    //输出配置
    EGLConfig config;
    EGLint configNum;
    EGLint configSpec[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };
    if(EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum)) {
        ALOGD( tag, "eglChooseConfig failed!");
        return;
    }
    //创建surface
    EGLSurface winsurface = eglCreateWindowSurface(display, config, nwin, 0);
    if(winsurface == EGL_NO_SURFACE) {
        ALOGD( tag, "eglCreateWindowSurface failed!");
        return;
    }

    //3 context 创建关联的上下文
    const EGLint ctxAttr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
    if(context == EGL_NO_CONTEXT) {
        ALOGD( tag, "eglCreateContext failed!");
        return;
    }
    if(EGL_TRUE != eglMakeCurrent(display, winsurface, winsurface, context)) {
        ALOGD( tag, "eglMakeCurrent failed!");
        return;
    }

    ALOGD( tag, "EGL Init Success!");

    //顶点和片元shader初始化
    //顶点shader初始化
    GLint vsh = InitShader(str_vertex_shader, GL_VERTEX_SHADER);
    //片元yuv420 shader初始化
    GLint fsh = InitShader(str_fragment_yuv420p_shader, GL_FRAGMENT_SHADER);


    /////////////////////////////////////////////////////////////
    //创建渲染程序
    GLint program = glCreateProgram();
    if(program == 0) {
        ALOGD( tag, "glCreateProgram failed!");
        return;
    }
    //渲染程序中加入着色器代码
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    //链接程序
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        ALOGD( tag, "glLinkProgram failed!");
        return;
    }
    glUseProgram(program);
    ALOGD( tag, "glLinkProgram success!");
    /////////////////////////////////////////////////////////////


    //加入三维顶点数据 两个三角形组成正方形
    static float vers[] = {
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
    };
    GLuint apos = (GLuint)glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(apos);
    //传递顶点
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, vers);

    //加入材质坐标数据
    static float txts[] = {
        1.0f, 0.0f, //右下
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0, 1.0
    };
    GLuint atex = (GLuint)glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(atex);
    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txts);


    int width = 424;
    int height = 240;

    //材质纹理初始化
    //设置纹理层
    glUniform1i( glGetUniformLocation(program, "yTexture"), 0); //对于纹理第1层
    glUniform1i( glGetUniformLocation(program, "uTexture"), 1); //对于纹理第2层
    glUniform1i( glGetUniformLocation(program, "vTexture"), 2); //对于纹理第3层

    //创建opengl纹理
    GLuint texts[3] = {0};
    //创建三个纹理
    glGenTextures(3, texts);

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width, height, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
                );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
                );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
                );





    //////////////////////////////////////////////////////
    ////纹理的修改和显示
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];
    buf[1] = new unsigned char[width * height / 4];
    buf[2] = new unsigned char[width * height / 4];


    for(int i = 0; i < 10000; i++) {
        //memset(buf[0],i,width*height);
        // memset(buf[1],i,width*height/4);
        //memset(buf[2],i,width*height/4);

        //420p   yyyyyyyy uu vv
        if(feof(fp) == 0) {
            //yyyyyyyy
            fread(buf[0], 1, width * height, fp);
            fread(buf[1], 1, width * height / 4, fp);
            fread(buf[2], 1, width * height / 4, fp);
        }





        //激活第1层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texts[0]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[0]);



        //激活第2层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, texts[1]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[1]);


        //激活第2层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, texts[2]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[2]);

        //三维绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示
        eglSwapBuffers(display, winsurface);
    }

    fclose(fp);
}