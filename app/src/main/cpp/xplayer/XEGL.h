//
//   author: patrick.dai
// datetime: 2019-12-03 11:45
//
#ifndef _XEGL_H_INCLUDE
#define _XEGL_H_INCLUDE


class XEGL
{
public:
    virtual bool init(void *win) = 0;

public:
    static XEGL *get();

protected:
    XEGL() {}

};





#endif//_XEGL_H_INCLUDE

