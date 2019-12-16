//
//   author: patrick.dai
// datetime: 2019-12-03 11:35
//

#ifndef _XTEXTURE_H_INCLUDE
#define _XTEXTURE_H_INCLUDE


class XTexture
{
public:
    virtual bool init(void *window) = 0;

    virtual void draw(unsigned char *data[], int width, int height) = 0;

public:
    static XTexture *create();

};


#endif//_XTEXTURE_H_INCLUDE
