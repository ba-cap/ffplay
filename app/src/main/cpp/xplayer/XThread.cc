//
//   author: patrick.dai
// datetime: 2019-12-02 16:33
//

#include "XThread.h"

#include <thread>

void XThread::sleep(int millisecond)
{
    std::chrono::milliseconds du(millisecond);
    std::this_thread::sleep_for(du);
}


void XThread::thread_main()
{
    mIsRunning = true;
    main();
    mIsRunning = false;
}


void XThread::start()
{
    mIsExit = false;
    std::thread th(&XThread::thread_main, this);
    // 放弃对线程的控制
    th.detach();
}


void XThread::stop()
{
    mIsExit = true;
    for (int i = 0; i < 200; ++i)
    {
        if(!mIsRunning)
        {
            return;
        }
        sleep(1);
    }
}