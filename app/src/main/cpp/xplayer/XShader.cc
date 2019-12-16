//
//   author: patrick.dai
// datetime: 2019-12-03 11:40
//

#include "XShader.h"
#include "XLog.h"

#include <GLES2/gl2.h>


#define STR_MAKER(str) #str

static const char *str_shader_vertex =
    STR_MAKER(
        attribute vec4 aPosition; // 顶点坐标
        attribute vec2 aTexCoord; // 材质顶点坐标
        varyint   vec2 vTexCoord; // 输出材质坐标
        void main()
{
    vTexCoord   = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
    gl_Position = aPosition;
});

static const char *str_shader_fragment_YUV420P =
    STR_MAKER(
        precision mediump float;     // 精度
        varying vec2      vTexCoord; // 顶点着色器传递入参坐标
        uniform sampler2D yTexture;  // 输入材质(不透明灰度, 单像素)
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        void main()
{
    vec3 yuv;
    vec3 rgb;
    yuv.r = texture2D(yTexture, vTexCoord).r;
    yun.g = texture2D(uTexture, vTexCoord).r - 0.5;
    yun.b = texture2D(vTexture, vTexCoord).r - 0.5;
    rgb   = mat3( 1.0,      1.0,     1.0,
                  0.0,     -0.39465, 2.03211,
                  1.13983, -0.58060, 0.0 );
    // 输出像素颜色
    gl_FragColor = vec4(rgb, 1.0);
});

// 三维顶点坐标数据
// 两个三角形组成正方形
static const float vertex_position[] =
{
    1.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f
};

// 材质坐标数据
static const float texture_position[] =
{
    1.0f, 0.0f,  // right  bottom
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};


static GLuint create_compile_shader(const char *src, GLenum type)
{
    GLuint shader = glCreateShader(type);
    if(shader == 0)
    {
        XLOGE("create shader type of %d failed.", type);
        return 0;
    }

    // load the shader source
    glShaderSource(shader,
                   1,       // shader 数量
                   &src,    // shader 代码
                   nullptr  // 代码长度
                  );

    // Compile the shader
    glCompileShader ( shader );

    // check compile status
    GLint compiled;
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );
    if ( !compiled )
    {
        GLint infoLen = 0;
        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
        if ( infoLen > 1 )
        {
            char *infoLog = new char[infoLen];
            glGetShaderInfoLog ( shader, infoLen, nullptr, infoLog );
            XLOGE("compiling shader failed: type=%d, info:\n%s", type, infoLog);
            delete[] infoLog;
        }
        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

static GLuint load_program(const char *srcVertex, const char *srcFragment)
{
    GLuint shader_vertex   = 0;
    GLuint shader_fragment = 0;
    GLuint program_object  = 0;
    GLint  linked          = 0;

    shader_vertex = create_compile_shader(srcVertex, GL_VERTEX_SHADER);
    if(shader_vertex == 0)
    {
        goto EXIT;
    }

    shader_fragment = create_compile_shader(srcVertex, GL_FRAGMENT_SHADER);
    if(shader_fragment == 0)
    {
        goto EXIT;
    }

    // Create the program object
    program_object = glCreateProgram();
    if(0 == program_object)
    {
        XLOGE("create GL program failed");
        goto EXIT;
    }

    glAttachShader(program_object, shader_vertex);
    glAttachShader(program_object, shader_fragment);

    // Link the program
    glLinkProgram( program_object );

    // Check the link status
    glGetProgramiv ( program_object, GL_LINK_STATUS, &linked );
    if ( !linked )
    {
        GLint infoLen = 0;
        glGetProgramiv ( program_object, GL_INFO_LOG_LENGTH, &infoLen );
        if ( infoLen > 1 )
        {
            char *infoLog = new char[infoLen];
            glGetProgramInfoLog( program_object, infoLen, nullptr, infoLog );
            XLOGE("GL linking program failed:\n%s", infoLog);
            delete[] infoLog;
        }
        glDeleteShader ( shader_vertex );
        glDeleteShader ( shader_fragment );
        glDeleteProgram ( program_object );
        return 0;
    }

EXIT:
    if(0 != shader_vertex)
    {
        glDeleteShader ( shader_vertex );
    }
    if(0 != shader_fragment)
    {
        glDeleteShader ( shader_fragment );
    }
    return program_object;
}


bool XShader::init()
{
    mGLProgram = load_program(str_shader_vertex, str_shader_fragment_YUV420P);
    if(0 == mGLProgram)
    {
        XLOGE("GL create program failed");
        return false;
    }
    glUseProgram(mGLProgram);
    XLOGI("GL init create program success");

    GLuint aPosition = (GLuint)glGetAttribLocation(mGLProgram, "aPosition");
    glEnableVertexAttribArray(aPosition);
    // 传递顶点
    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 12, vertex_position);


    // 加载材质坐标数据
    GLuint aTexCoord = (GLuint)glGetAttribLocation(mGLProgram, "aTexCoord");
    glEnableVertexAttribArray(aTexCoord);
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 8, texture_position);


    // 材质纹理初始化
    // 设置纹理
    // 这里是 YUV 格式
    // 如果是硬解码出来数据为 nv12, 这样下面的代码程序将会不一样
    glUniform1i( glGetUniformLocation(mGLProgram, "yTexture"), 0 ); // 对应纹理第一层
    glUniform1i( glGetUniformLocation(mGLProgram, "uTexture"), 1 ); // 对应纹理第二层
    glUniform1i( glGetUniformLocation(mGLProgram, "vTexture"), 2 ); // 对应纹理第三层

    XLOGI("shader init success");

    return true;
}

void XShader::draw()
{
    if(mGLProgram == 0)
    {
        XLOGE("shader program is ZERO");
        return;
    }

    // 三角形绘制
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void XShader::getTexture(unsigned int index, int width, int height, unsigned char *buffer)
{

    if(mTextures[index] == 0)
    {
        // 材质初始化
        // 1- 创建材质
        glGenTextures(1, &mTextures[index]);
        // 2- 绑定纹理属性
        glBindTexture(GL_TEXTURE_2D, mTextures[index]);
        // 3- 设置最大 最小过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 4- 设置纹理格式和大小
        //    播放 NV12 时是有区别
        glTexImage2D(
            GL_TEXTURE_2D,
            0,                // 默认细节0
            GL_LUMINANCE,     // GPU内部格式, 亮度/灰度图
            width, height,
            0,                // 边框
            GL_LUMINANCE,     // 数据像素格式 亮度/灰度图  与上面保持一直
            GL_UNSIGNED_BYTE, // 像素数据类型
            nullptr           // 纹理数据
        );
    }

    // 激活相应层次纹理, 并绑定到创建的 OPENGL 纹理
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, mTextures[index]);
    // 替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
}

