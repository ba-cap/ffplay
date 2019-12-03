//
//   author: patrick.dai
// datetime: 2019-12-02 19:28
//

#ifndef _FFDECODE_H_INCLUDE
#define _FFDECODE_H_INCLUDE


#include "XParameter.h"
#include "IDecode.h"

struct AVCodecContext;
struct AVFrame;

class FFDecode: public IDecode
{
public:
    virtual bool open(XParameter parameter);

    virtual bool send_packet(XData pkt);

    // 从线程中获取解码结果, 再次调用复用上次空间(线程不安全)
    virtual XData receive_frame();

protected:
    AVCodecContext *codec = nullptr;
    AVFrame        *frame = nullptr;

};


#endif//_FFDECODE_H_INCLUDE

