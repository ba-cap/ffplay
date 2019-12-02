//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#ifndef _XDATA_H_INCLUDE
#define _XDATA_H_INCLUDE

struct XData
{
    unsigned char *data = nullptr;
    int size = 0;

    void drop();

};


#endif//_XDATA_H_INCLUDE
