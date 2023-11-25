#include "SharedQueue.h"

unsigned int SharedQueue::Count()
{
    std::lock_guard<std::mutex> l{ queue_mutex };
    return _count;
}

bool SharedQueue::empty()
{
    std::lock_guard<std::mutex> l{ queue_mutex };
    return _count == 0;
}

void SharedQueue::Push(std::wstring& Value)
{
    std::lock_guard<std::mutex> l{ queue_mutex };
    _queue.push(Value);
    _count++;
}

void SharedQueue::Push(std::wstring&& Value)
{
    std::lock_guard<std::mutex> l{ queue_mutex };
    _queue.push(std::move(Value));
    _count++;
}

std::wstring SharedQueue::Pop()
{
    std::lock_guard<std::mutex> l{ queue_mutex };
    if (_count == 0)
    {
        return nullptr;
    }
    auto tmp = _queue.front(); _queue.pop();
    _count--;
    return std::move(tmp);
}
