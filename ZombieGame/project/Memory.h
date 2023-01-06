#pragma once
#include <unordered_map>

template<typename T_MemoryObject>
class Memory
{
public:
	Memory() {};


private:
	std::unordered_map<int, T_MemoryObject> m_Memories{};
};

