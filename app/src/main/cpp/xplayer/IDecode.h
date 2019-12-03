//
//   author: patrick.dai
// datetime: 2019-12-02 19:28
//

#ifndef _IDECODE_H_INCLUDE
#define _IDECODE_H_INCLUDE

#include "XParameter.h"
#include "IObserver.h"

#include <list>
#include <mutex>

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

    // 主体 notify 的数据 阻塞
    virtual void update(XData pkt);

protected:
    virtual void main();

    data_type type = data_type::UNKNOWN;

private:
    // 所有缓冲帧
    int queue_size   = 100;
    std::list<XData> queue_packets;
    std::mutex       pkt_mutex;
};


#endif //_IDECODE_H_INCLUDE

