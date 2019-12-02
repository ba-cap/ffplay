//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#ifndef _IDEMUX_H_INCLUDE
#define _IDEMUX_H_INCLUDE

#include "XData.h"

// 解封装接口
class IDemux
{
protected:
    // 总时长(毫秒)
    int mTotalMillisecond;

public:
    // 打开文件, 流媒体(rtmp, http rtsp)
    virtual bool Open(const char *url) = 0;

    // 读取(帧数据) 数据由调用者清理
    virtual XData Read() = 0;

};


#endif//_IDEMUX_H_INCLUDE
