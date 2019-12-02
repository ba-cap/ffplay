//
//   author: patrick.dai
// datetime: 2019-12-02 18:00
//

#ifndef _IOBSERVER_H_INCLUDE
#define _IOBSERVER_H_INCLUDE

#include "XData.h"
#include "XThread.h"

#include <vector>
#include <mutex>

// 观察者 和 主体
class IObserver: public XThread
{
public:
    // 观察者接收函数
    virtual void update(XData data) {}

    // 主体函数 添加观察者 (线程安全)
    void add_observer(IObserver *observer);

    // 通知所有观察者 (线程安全)
    void notify(XData data);

protected:
    std::mutex mutex;
    std::vector<IObserver *> mObservers;


};


#endif//_IOBSERVER_H_INCLUDE

