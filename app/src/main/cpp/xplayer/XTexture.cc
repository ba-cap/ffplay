//
//   author: patrick.dai
// datetime: 2019-12-03 11:35
//

#include "XTexture.h"
#include "XLog.h"
#include "XEGL.h"
#include "XShader.h"

class CXTexture: public XTexture
{
private:
    XShader mShader;

public:
    virtual bool  init(void *window)
    {
        if(nullptr == window)
        {
            XLOGE("window is null, init CXTexture failed");
            return false;
        }

        // init egl
        if(!XEGL::get()->init(window))
        {
            return false;
        }

        // init shader
        mShader.init();
    }

    virtual void draw(unsigned char *data[], int width, int height)
    {
        mShader.getTexture(0, width,     height,     data[0]); // Y
        mShader.getTexture(1, width / 2, height / 2, data[1]); // U
        mShader.getTexture(2, width / 2, height / 2, data[2]); // V
        mShader.draw();
        XEGL::get()->draw();
    }
};


XTexture *XTexture::create()
{
    return new CXTexture{};
}
