//
//   author: patrick.dai
// datetime: 2019-12-03 11:31
//

#ifndef _IVIDEOVIEW_H_INCLUDE
#define _IVIDEOVIEW_H_INCLUDE


#include "XData.h"

class IVideoView
{
public:
    virtual void setRender(void *win) = 0;

    virtual void render(XData data) = 0;

    virtual void update(XData data);
};


#endif//_IVIDEOVIEW_H_INCLUDE

