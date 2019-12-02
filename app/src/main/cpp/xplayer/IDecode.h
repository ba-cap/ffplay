//
//   author: patrick.dai
// datetime: 2019-12-02 19:28
//

#ifndef _IDECODE_H_INCLUDE
#define _IDECODE_H_INCLUDE

#include "XParameter.h"
#include "IObserver.h"

// 解码接口(支持硬解码)
class IDecode: public IObserver
{
public:
    // 打开解码器
    virtual bool open(XParameter parameter) = 0;

    // future模型 (ffmpeg)
    // 发送数据到线程解码
    virtual bool send_packet(XData pkt) = 0;

    // 从线程中获取解码结果
    virtual XData receive_frame() = 0;
};


#endif //_IDECODE_H_INCLUDE

