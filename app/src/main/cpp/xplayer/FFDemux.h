//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#ifndef _FFDEMUX_H_INCLUDE
#define _FFDEMUX_H_INCLUDE

#include "IDemux.h"
#include "XData.h"

struct AVFormatContext;

class FFDemux: public IDemux
{
private:
    AVFormatContext *ic = nullptr;
    int audio_stream_idx = -1;
    int video_stream_idx = -1;

public:
    FFDemux();

    // 打开文件, 流媒体(rtmp, http rtsp)
    virtual bool Open(const char *url);

    virtual XParameter getVideoParameter();

    virtual XParameter getAudioParameter();


    // 读取(帧数据) 数据由调用者清理
    virtual XData Read();



};


#endif//_FFDEMUX_H_INCLUDE
