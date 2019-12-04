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
    GLuint mGLProgram;

public:
    virtual bool init();

};


#endif//_XSHADER_H_INCLUDE
