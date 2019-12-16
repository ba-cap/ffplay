//
//   author: patrick.dai
// datetime: 2019-12-03 11:40
//

#ifndef _XSHADER_H_INCLUDE
#define _XSHADER_H_INCLUDE


#include <GLES2/gl2.h>

class XShader
{
private:
    GLuint mGLProgram    = 0;
    GLuint mTextures[64] = { 0x00 };

public:
    virtual bool init();

    // 获取材质, 并映射到内存
    virtual void getTexture(unsigned int index, int width, int height, unsigned char *buffer);

    virtual void draw();
};


#endif//_XSHADER_H_INCLUDE
