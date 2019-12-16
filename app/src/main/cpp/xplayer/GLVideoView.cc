//
//   author: patrick.dai
// datetime: 2019-12-03 11:33
//

#include "GLVideoView.h"
#include "XLog.h"
#include "XTexture.h"

void GLVideoView::setRender(void *win)
{
    mView = win;
}

void GLVideoView::render(XData data)
{
    if(!mView)
    {
        XLOGE("null object of window");
        return;
    }

    // 创建 texture
    if(!mTexture)
    {
        mTexture = XTexture::create();
        mTexture->init(mView);
    }

    mTexture->draw(data.datas, data.width, data.height);
}
