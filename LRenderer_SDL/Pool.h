#pragma once

#include <vector>
#include <queue>

#include "cppcoro/generator.hpp"

template <typename T>
class Pool
{
public:
	int Push(T obj);
	void Remove(int index);
	cppcoro::generator<T&> GetValidItems();

	int count;
	std::vector<T> data = std::vector<T>();
	std::vector<bool> valid = std::vector<bool>(); // TODO: 改为位图
	std::queue<int> freeIndex = std::queue<int>();
};

// 定义协程生成器（遍历有效元素）
template <typename T>
cppcoro::generator<T&> Pool<T>::GetValidItems() {
	for (size_t i = 0; i < data.size(); ++i) {
		if (valid[i]) {
			co_yield data[i]; // 生成有效元素
		}
	}
}

template <typename T>
int Pool<T>::Push(T obj)
{
	if (freeIndex.size() == 0)
	{
		valid.push_back(true);
		data.push_back(obj);
		count++;
		return data.size() - 1;
	}

	int index = freeIndex.front();
	freeIndex.pop();
	data[index] = obj;
	valid[index] = true;
	count++;
	return index;
}

template <typename T>
void Pool<T>::Remove(int index)
{
	count--;
	freeIndex.push(index);
	valid[index] = false;
}