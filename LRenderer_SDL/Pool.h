#pragma once

#include <vector>
#include <queue>

template <typename T>
class Pool
{
public:
	int Push(T obj);
	void Remove(int index);

	std::vector< T> data = std::vector<T>();
	std::vector<bool> valid = std::vector<bool>(); // TODO: 改为位图
	std::queue<int> freeIndex = std::queue<int>();
};

template <typename T>
int Pool<T>::Push(T obj)
{
	if (freeIndex.size() == 0)
	{
		valid.push_back(true);
		data.push_back(obj);
		return data.size() - 1;
	}

	int index = freeIndex.front();
	freeIndex.pop();
	data[index] = obj;
	valid[index] = true;
	return index;
}

template <typename T>
void Pool<T>::Remove(int index)
{
	freeIndex.push(index);
	valid[index] = false;
}