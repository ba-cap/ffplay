//
//   author: patrick.dai
// datetime: 2019-12-02 18:00
//
#include "IObserver.h"


void IObserver::add_observer(IObserver *observer)
{
    if(nullptr == observer)
    {
        return;
    }

    mutex.lock();
    mObservers.push_back(observer);
    mutex.unlock();
}

void IObserver::notify(XData data)
{
    mutex.lock();
    for(int i = 0; i < mObservers.size(); ++i)
    {
        mObservers[i]->update(data);
    }
    mutex.unlock();
}