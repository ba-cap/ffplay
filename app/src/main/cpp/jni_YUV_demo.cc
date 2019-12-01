
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

static const char *tag = "jni_yuv";

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

static enum {
    ATTRIBUTE_VERTEX,
    ATTRIBUTE_TEXTURE,
};


static void gl_error_check(const char *cmd)
{
    GLint error;
    for(error = glGetError(); error; error = glGetError()) {
        ALOGE(tag, "[%s] failed, error code= 0x%x\n", cmd, error);
    }
}

static void bind_texture(GLuint texture, GLuint width, GLuint height)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    // 缩小和放大过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,                // 细节基本 0默认
                 GL_LUMINANCE,     // gpu内部格式 亮度，灰度图
                 width, height,    // 拉升到全屏
                 0,                // 边框
                 GL_LUMINANCE,     // 数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, // 像素的数据类型
                 nullptr           // 纹理的数据
                );
}

static void active_texture(GLenum layout, GLuint texture, GLuint width, GLuint height, GLubyte *buffer)
{
    glActiveTexture(layout);
    gl_error_check("glActiveTexture");

    glBindTexture(GL_TEXTURE_2D, texture);
    gl_error_check("glBindTexture");

    // 替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
}


static GLuint create_compile_shader(const char *shader, GLenum type)
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
    GLint status = 0;
    glGetShaderiv(ishader, GL_COMPILE_STATUS, &status);
    if(!status) {
        GLint infoLen = 0;
        glGetShaderiv(ishader, GL_INFO_LOG_LENGTH, &infoLen);

        GLchar *logInfo = new GLchar[infoLen];
        GLint   logLen;
        glGetShaderInfoLog(ishader, infoLen, &logLen, logInfo );
        ALOGW(tag, "compile [%d] shader failed:\n %s.", type, logInfo);
        delete[] logInfo;
        glDeleteShader(ishader);
        ishader = 0;
    }

    return ishader;
}


static GLuint create_link_program(const char *vShaderSrc, const char *fShaderSrc)
{
    GLuint v_shader = create_compile_shader(vShaderSrc, GL_VERTEX_SHADER);
    if(0 == v_shader) {
        ALOGW(tag, "create vertex shader failed");
        return 0;
    }

    GLuint f_shader = create_compile_shader(fShaderSrc, GL_FRAGMENT_SHADER);
    if(0 == f_shader) {
        ALOGW(tag, "create fragment shader failed");
        return 0;
    }

    GLuint i_program = glCreateProgram();
    if(0 == i_program) {
        ALOGW(tag, "create program failed");
        return 0;
    }

    glAttachShader(i_program, v_shader);
    gl_error_check("glAttachShader");

    glAttachShader(i_program, f_shader);
    gl_error_check("glAttachShader");

    glLinkProgram(i_program);
    gl_error_check("glLinkProgram");

    GLint status = GL_FALSE;
    glGetProgramiv(i_program, GL_LINK_STATUS, &status);
    if(GL_TRUE != status) {
        GLint logLen;
        glGetProgramiv(i_program, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0) {
            GLchar *logData = new GLchar[logLen];
            glGetProgramInfoLog(i_program, logLen, nullptr, logData);
            ALOGW(tag, "link opengles program failed.\n%s\n", logData);
            delete[] logData;
        }
        glDeleteProgram(i_program);
        i_program = 0;
    }
    return i_program;
}


static bool makeEglDisplay(ANativeWindow *window, EGLDisplay *display, EGLSurface *surface)
{
    *display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(*display == EGL_NO_DISPLAY) {
        ALOGW(tag, "eglGetDisplay failed");
        return false;
    }

    if(EGL_TRUE != eglInitialize(*display, nullptr, nullptr)) {
        ALOGW(tag, "eglInitialize failed");
        *display = EGL_NO_DISPLAY;
        *surface = nullptr;
        return false;
    }

    // create surface
    EGLConfig  config;
    EGLint     config_num;
    EGLint     config_spec[] = {
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    if(EGL_TRUE != eglChooseConfig(*display, config_spec, &config, 1, &config_num)) {
        ALOGW(tag, "choose config failed");
        *display = EGL_NO_DISPLAY;
        *surface = nullptr;
        return false;
    }

    // create window surface
    *surface = eglCreateWindowSurface(*display, config, window, nullptr);
    if(*surface == nullptr) {
        ALOGW(tag, "create window surface failed");
        *display = EGL_NO_DISPLAY;
        *surface = nullptr;
        return false;
    }


    // create context
    const EGLint contextAttribute[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        2,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(*display, config, EGL_NO_CONTEXT, contextAttribute);
    if(context == EGL_NO_CONTEXT) {
        ALOGW(tag, "create egl context failed");
        *display = EGL_NO_DISPLAY;
        *surface = nullptr;
        return false;
    }

    if(EGL_TRUE != eglMakeCurrent(*display, *surface, *surface, context)) {
        ALOGW(tag, "eglMakeCurrent failed");
        *display = EGL_NO_DISPLAY;
        *surface = nullptr;
        return false;
    }

    ALOGI(tag, "EGL init success");
    return true;
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

    EGLDisplay display;
    EGLSurface eglSurface;
    if(!makeEglDisplay(native_win, &display, &eglSurface)) {
        ALOGE(tag, "make EGL display infor failed");

        // go to
        return;
    }

    GLuint program = create_link_program(str_vertex_shader, str_fragment_yuv420p_shader);
    if(0 == program) {
        ALOGE(tag, "create opengles program failed");
        // goto
        return;
    }

    GLuint width  = 424;
    GLuint height = 240;


    glBindAttribLocation(program, ATTRIBUTE_VERTEX, "aPosition");
    gl_error_check("glBindAttribLocation_aPosition");

    glBindAttribLocation(program, ATTRIBUTE_TEXTURE, "aTexCoord");
    gl_error_check("glBindAttribLocation_aTexCoord");


    glVertexAttribPointer(ATTRIBUTE_VERTEX, 3, GL_FLOAT, 0, 0, vertex_pointer);
    gl_error_check("glVertexAttribPointer_vertex");
    glEnableVertexAttribArray(ATTRIBUTE_VERTEX);
    gl_error_check("glEnableVertexAttribArray_vertex");


    glVertexAttribPointer(ATTRIBUTE_TEXTURE, 2, GL_FLOAT, 0, 0, texture_pointer);
    gl_error_check("glVertexAttribPointer_texture");
    glEnableVertexAttribArray(ATTRIBUTE_TEXTURE);
    gl_error_check("glEnableVertexAttribArray_texture");


    //GLuint aPosition =  (GLuint)glGetAttribLocation(program, "aPosition");
    //glEnableVertexAttribArray(aPosition);
    //glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), vertex_pointer);


    //GLuint aTexCoord = (GLuint) glGetAttribLocation(program, "aTexCoord");
    //glEnableVertexAttribArray(aTexCoord);
    //glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), texture_pointer);

    // 材质纹理初始化
    glUniform1i( glGetUniformLocation(program, "yTexture"), 0);  // 对应纹理第 1 层
    glUniform1i( glGetUniformLocation(program, "uTexture"), 1);  // 对应纹理第 2 层
    glUniform1i( glGetUniformLocation(program, "vTexture"), 2);  // 对应纹理第 3 层

    // 创建 opengl es 纹理
    GLuint texture_y = 0;
    GLuint texture_u = 0;
    GLuint texture_v = 0;

    glGenTextures(1, &texture_y);
    gl_error_check("glGenTextures_Y");

    glGenTextures(1, &texture_u);
    gl_error_check("glGenTextures_U");

    glGenTextures(1, &texture_v);
    gl_error_check("glGenTextures_V");

    bind_texture(texture_y, width, height);
    bind_texture(texture_u, width / 2, height / 2);
    bind_texture(texture_v, width / 2, height / 2);


    ///
    /// 纹理修改和显示
    ///
    const size_t BUF_LEN_Y   = width * height;
    const size_t BUF_LEN_U_V = BUF_LEN_Y / 4;
    GLubyte *buffer_y = new GLubyte[BUF_LEN_Y];
    GLubyte *buffer_u = new GLubyte[BUF_LEN_U_V];
    GLubyte *buffer_v = new GLubyte[BUF_LEN_U_V];


    for(int i = 0; i < 500; ++i) {
        memset(buffer_y, i, BUF_LEN_Y);
        memset(buffer_u, i, BUF_LEN_U_V);
        memset(buffer_v, i, BUF_LEN_U_V);

        // 420p  yyyyyyyy uu vv
        if(0 == feof(file)) {
            // yyyyyyyy
            //fread(buffer[0], 1, width * height, file);
            //fread(buffer[1], 1, width * height / 4, file);
            //fread(buffer[2], 1, width * height / 4, file);
        }

        // 激活第 1 层, 绑定到创建 opengl 纹理
        active_texture(GL_TEXTURE0, texture_y, width, height, buffer_y);

        // 激活第 2 层, 绑定到创建 opengl 纹理
        active_texture(GL_TEXTURE1, texture_u, width / 2, height / 2, buffer_u);

        // 激活第 3 层, 绑定到创建 opengl 纹理
        active_texture(GL_TEXTURE2, texture_v, width / 2, height / 2, buffer_v);


        //三维绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //窗口显示
        eglSwapBuffers(display, eglSurface);
    }

    delete[] buffer_y;
    delete[] buffer_u;
    delete[] buffer_v;



    // close the open file
    if(nullptr != file) {
        fclose(file);
        file = nullptr;
    }
}