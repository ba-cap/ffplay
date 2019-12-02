//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#ifndef _IDEMUX_H_INCLUDE
#define _IDEMUX_H_INCLUDE

#include "XData.h"
#include "IObserver.h"
#include "XParameter.h"

// 解封装接口
class IDemux: public IObserver
{
protected:
    // 总时长(毫秒)
    int mTotalMillisecond;

    virtual void main();

public:
    // 打开文件, 流媒体(rtmp, http rtsp)
    virtual bool Open(const char *url) = 0;

    // 获取视频参数
    virtual XParameter getParameter() = 0;

    // 读取(帧数据) 数据由调用者清理
    virtual XData Read() = 0;

};


#endif//_IDEMUX_H_INCLUDE
