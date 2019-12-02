//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#include "XData.h"

extern "C" {
#include <libavcodec/avcodec.h>
}


void XData::drop()
{
    if(!data)
    {
        return;
    }

    av_packet_free((AVPacket **)&data);
    data = nullptr;
    size = 0;
}