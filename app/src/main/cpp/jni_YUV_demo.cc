
#include "demo.h"
#include "ALogger.h"

#include <jni.h>
#include <string>
#include <cstring>
#include <EGL/egl.h>
//#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/native_window_jni.h>

static const char *tag = "jni_yuv";

static const char *str_vertex_shader =
    "attribute vec4 aPosition; "  // 顶点坐标
    "attribute vec2 aTexCoord; "  // 材质顶点坐标
    "varying   vec2 vTexCoord; "  // 输出材质坐标
    "void main() { "
    "    vTexCoord   = vec2( aTexCoord.x, 1.0 - aTexCoord.y ); "
    "    gl_Position = aPosition; "
    "}";

static const char *str_fragment_yuv420p_shader =
    "precision mediump float; "    // float 精度
    "varying vec2 vTexCoord; "     // 顶点着色器传递过来的坐标
    "uniform sampler2D yTexture; " // 输入材质 (不透明灰度, 单像素)
    "uniform sampler2D uTexture; "
    "uniform sampler2D vTexture; "
    "void main() { "
    "    vec3  yuv; "
    "    vec3  rgb; "
    "    yuv.r = texture2D(yTexture, vTexCoord).r; "
    "    yuv.g = texture2D(uTexture, vTexCoord).r - 0.5; "
    "    yuv.b = texture2D(vTexture, vTexCoord).r - 0.5; "
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


static GLuint init_shader(const char *shader, GLint type)
{
    // create a shader
    GLuint ishader = glCreateShader(type);
    if(0 == ishader) {
        ALOGE(tag, "create shader failed by type of [%d].", type);
        return ishader;
    }

    // load the shader
    glShaderSource(ishader, 1, &shader, nullptr);

    // compile shader
    glCompileShader(ishader);

    // get the compile information
    GLint status;
    glGetShaderiv(ishader, GL_COMPILE_STATUS, &status);
    if(0 == status) {
        GLint infoLen = 0;
        glGetShaderiv(ishader, GL_INFO_LOG_LENGTH, &infoLen);

        GLchar *logInfo = new GLchar[infoLen];
        GLint   logLen;
        glGetShaderInfoLog(ishader, infoLen, &logLen, logInfo );
        ALOGW(tag, "compile shader failed:\n %s.", logInfo);
        delete[] logInfo;
        glDeleteShader(ishader);
        ishader = 0;
    }

    return ishader;
}


void play_video_yuv(JNIEnv *env, jobject surface, const char *path)
{
    ALOGD(tag, "play yuv file: %s.", path);

    bool canNextStep = true;

    FILE *file = fopen(path, "rb");
    if (nullptr == file) {
        ALOGE(tag, "open file: %s failed.", path);
        return;
    }

    // get the native surface
    ANativeWindow *native_win = ANativeWindow_fromSurface(env, surface);

    ///
    /// EGL
    // EDL display create and init
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        ALOGE(tag, "eglGetDisplay failed");
        canNextStep = false;
    }
    if (canNextStep && EGL_TRUE != eglInitialize(display, nullptr, nullptr)) {
        ALOGE(tag, "egl init failed");
        canNextStep = false;
    }

    // surface window config
    EGLConfig config;
    EGLint config_num;
    EGLint config_spec[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_NONE
    };
    if (canNextStep && EGL_TRUE != eglChooseConfig(display, config_spec, &config, 1, &config_num)) {
        ALOGE(tag, "eglChooseConfig failed");
        canNextStep = false;
    }

    // create surface
    EGLSurface win_surface = nullptr;
    if (canNextStep) {
        win_surface = eglCreateWindowSurface(display, config, native_win, 0);
        if (EGL_NO_SURFACE == win_surface) {
            ALOGE(tag, "eglCreateWindowSurface failed");
            canNextStep = false;
        }
    }

    // create context
    EGLContext context = nullptr;
    if (canNextStep) {
        const EGLint ctxAttribute[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            2,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttribute);
        if (EGL_NO_CONTEXT == context) {
            ALOGE(tag, "create egl context failed");
            canNextStep = false;
        }
    }
    if (canNextStep && EGL_TRUE != eglMakeCurrent(display, win_surface, win_surface, context)) {
        ALOGE(tag, "eglMakeCurrent failed");
        canNextStep = false;
    }

    ALOGI(tag, "egl init success");


    ///
    /// create shader
    ///
    // create vertex and fragment shader
    GLuint vertex_shader = 0;
    if (canNextStep && 0 == (vertex_shader = init_shader(str_vertex_shader, GL_VERTEX_SHADER))) {
        ALOGE(tag, "create vertex shader failed");
        canNextStep = false;
    }
    GLuint fragment_yuv420p_shader = 0;
    if (canNextStep &&
        (0 == (fragment_yuv420p_shader =
                   init_shader(str_fragment_yuv420p_shader, GL_FRAGMENT_SHADER)))) {
        ALOGE(tag, "create fragment shader failed");
        canNextStep = false;
    }

    // create shader program
    GLuint program = 0;
    if (canNextStep && 0 == (program = glCreateProgram())) {
        ALOGE(tag, "create shader program failed");
        canNextStep = false;
    }

    if (canNextStep) {
        // attach the shader
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_yuv420p_shader);

        // link the program
        glLinkProgram(program);

        // get the link status
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(GL_TRUE != status) {
            GLint infoLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            GLchar *logData = new GLchar[infoLen];
            GLsizei realLogLen;
            glGetProgramInfoLog(program, infoLen, &realLogLen, logData);
            ALOGE(tag, "link program failed.\n%s", logData);
            delete[] logData;
            glDeleteProgram(program);

            canNextStep = false;
        }
    }

    GLuint aPosition = 0;
    GLuint aTexCoord = 0;

    int width  = 424;
    int height = 240;

    if(canNextStep) {
        ALOGI(tag, "link program success");

        aPosition = (GLuint)glGetAttribLocation(program, "aPosition");
        glEnableVertexAttribArray(aPosition);
        glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), vertex_pointer);

        aTexCoord = (GLuint) glGetAttribLocation(program, "aTexCoord");
        glEnableVertexAttribArray(aTexCoord);
        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), texture_pointer);


        // 材质纹理初始化
        glUniform1i( glGetUniformLocation(program, "yTexture"), 0);  // 对应纹理第 1 层
        glUniform1i( glGetUniformLocation(program, "uTexture"), 1);  // 对应纹理第 2 层
        glUniform1i( glGetUniformLocation(program, "vTexture"), 2);  // 对应纹理第 3 层


        // 创建 opengl es 纹理
        GLuint texts[3] = {0x00};
        // 创建 3 个纹理
        glGenTextures(3, texts);

        // 设置纹理属性
        glBindTexture(GL_TEXTURE_2D, texts[0]);
        // 缩小过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 设置纹理格式和大小
        glTexImage2D(
            GL_TEXTURE_2D,
            0,                 // 基本细节 默认0
            GL_LUMINANCE,      // GPU 内部格式, 亮度,灰度图
            width, height,      // 拉升到全屏
            0,                 // 边框
            GL_LUMINANCE,      // 数据像素格式, 与上面一致
            GL_UNSIGNED_BYTE,  // 像素数据类型
            nullptr );


        // 设置纹理属性
        glBindTexture(GL_TEXTURE_2D, texts[1]);
        // 缩小过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 设置纹理格式和大小
        glTexImage2D(
            GL_TEXTURE_2D,
            0,                    // 基本细节 默认0
            GL_LUMINANCE,         // GPU 内部格式, 亮度,灰度图
            width / 2, height / 2, // 拉升到全屏
            0,                    // 边框
            GL_LUMINANCE,         // 数据像素格式, 与上面一致
            GL_UNSIGNED_BYTE,     // 像素数据类型
            nullptr );


        // 设置纹理属性
        glBindTexture(GL_TEXTURE_2D, texts[2]);
        // 缩小过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 设置纹理格式和大小
        glTexImage2D(
            GL_TEXTURE_2D,
            0,                    // 基本细节 默认0
            GL_LUMINANCE,         // GPU 内部格式, 亮度,灰度图
            width / 2, height / 2, // 拉升到全屏
            0,                    // 边框
            GL_LUMINANCE,         // 数据像素格式, 与上面一致
            GL_UNSIGNED_BYTE,     // 像素数据类型
            nullptr );



        ///
        /// 纹理修改和显示
        ///
        uint8_t *buffer[3] = {nullptr};
        buffer[0] = new uint8_t[width * height];
        buffer[1] = new uint8_t[width * height / 4];
        buffer[2] = new uint8_t[width * height / 4];

        for(int i = 0; i < 5000; ++i) {
            memset(buffer[0], i, width * height);
            memset(buffer[1], i, width * height / 4);
            memset(buffer[2], i, width * height / 4);

            // 420p  yyyyyyyy uu vv
            if(0 == feof(file)) {
                // yyyyyyyy
                //fread(buffer[0], 1, width * height, file);
                //fread(buffer[1], 1, width * height / 4, file);
                //fread(buffer[2], 1, width * height / 4, file);
            }

            // 激活第 1 层, 绑定到创建 opengl 纹理
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texts[0]);
            //替换纹理内容
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer[0]);

            // 激活第 2 层, 绑定到创建 opengl 纹理
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texts[1]);
            //替换纹理内容
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer[1]);

            // 激活第 3 层, 绑定到创建 opengl 纹理
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texts[2]);
            //替换纹理内容
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer[2]);


            //三维绘制
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            //窗口显示
            eglSwapBuffers(display, win_surface);
        }

        delete[] buffer[0];
        delete[] buffer[1];
        delete[] buffer[2];
    }


    // close the open file
    if(nullptr != file) {
        fclose(file);
        file = nullptr;
    }
}