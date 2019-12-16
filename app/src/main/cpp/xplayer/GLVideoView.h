//
//   author: patrick.dai
// datetime: 2019-12-03 11:33
//

#ifndef _GLVIDEOVIEW_H_INCLUDE
#define _GLVIDEOVIEW_H_INCLUDE


#include "IVideoView.h"

class XTexture;

class GLVideoView: public IVideoView
{
protected:
    void     *mView    = nullptr;
    XTexture *mTexture = nullptr;

public:
    virtual void setRender(void *win);

    virtual void render(XData data);
};


#endif//_GLVIDEOVIEW_H_INCLUDE

