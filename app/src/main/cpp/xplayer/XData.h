//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#ifndef _XDATA_H_INCLUDE
#define _XDATA_H_INCLUDE

enum data_type
{
    UNKNOWN = 0,
    DATA_AUDIO = 1,
    DATA_VIDEO = 2,
};

struct XData
{
    unsigned char *data = nullptr;
    int size = 0;
    data_type type = UNKNOWN;

    unsigned char *datas[8] = { 0x00 };

    int width  = 0;
    int height = 0;

    void drop();
};


#endif//_XDATA_H_INCLUDE
