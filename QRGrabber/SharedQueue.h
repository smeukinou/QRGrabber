#pragma once
#include <queue>
#include <mutex>
#include <string>

class SharedQueue
{
public:
	unsigned int Count();
	bool empty();	
	void Push(std::wstring& Value);
	void Push(std::wstring&& Value);
	std::wstring Pop();

private:	
	std::mutex queue_mutex{};
	std::queue<std::wstring> _queue{};
	unsigned int _count{ 0 };
};

