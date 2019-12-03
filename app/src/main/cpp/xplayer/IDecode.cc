//
//   author: patrick.dai
// datetime: 2019-12-02 19:28
//

#include "IDecode.h"


void IDecode::update(XData pkt)
{
    // 类型数据不相同 直接丢弃
    // 可能存在数据内存泄漏
    if(pkt.type != this->type)
    {
        return;
    }

    // 这样插入会耗尽
    // 数量量大, 需要阻塞

    while (!mIsExit)
    {
        pkt_mutex.lock();
        if (queue_packets.size() <= queue_size)
        {
            // 生产者
            queue_packets.push_back(pkt);
            pkt_mutex.unlock();
            break;
        }
        pkt_mutex.unlock();
        sleep(1);
    }
}


void IDecode::main()
{
    while (!mIsExit)
    {
        pkt_mutex.lock();
        if(queue_packets.empty())
        {
            pkt_mutex.unlock();
            sleep(1);
            continue;
        }

        // 取出 packet
        // 消费者
        XData data = queue_packets.front();
        queue_packets.pop_front();

        // 发送数据到解码线程
        // 一个数据包, 可能解码多个结果
        if(send_packet(data))
        {
            while(!mIsExit)
            {
                // 获取解码数据
                XData frame = receive_frame();
                if(frame.data == nullptr)
                {
                    break;
                }

                // 读到了数据
                // 发送数据给观察者
                this->notify(frame);
            }
        }

        // 不能省略
        data.drop();

        pkt_mutex.unlock();
    }
}