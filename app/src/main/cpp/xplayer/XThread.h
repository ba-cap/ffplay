//
//   author: patrick.dai
// datetime: 2019-12-02 16:33
//

#ifndef _XTHREAD_H_INCLUDE
#define _XTHREAD_H_INCLUDE

// 使用 c++11 线程库来实现
class XThread
{
public:
    // 启动线程
    virtual void start();

    // 安全停止线程(不一定成功)
    virtual void stop();

    // 入口主函数
    virtual void main() {}

public:
    static void sleep(int millisecond);

protected:
    bool mIsExit = false;
    bool mIsRunning = false;

private:
    void thread_main();
};


#endif//_XTHREAD_H_INCLUDE
