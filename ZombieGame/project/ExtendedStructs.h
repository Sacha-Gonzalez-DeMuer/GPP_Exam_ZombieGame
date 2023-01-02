#pragma once
#include "Exam_HelperStructs.h"
#include <functional>

struct EAgentInfo : AgentInfo
{
	EAgentInfo(const AgentInfo& info) : AgentInfo(info) {}

	Elite::Vector2 GetForward() const { return Elite::Vector2(cos(Orientation), sin(Orientation)); };
	void SetForward(Elite::Vector2 forward) { Orientation = atan2(forward.y, forward.x); };
};


struct EntityInfoHash 
{
    std::size_t operator()(const EntityInfo& ei) const
    {
        std::size_t h1 = std::hash<eEntityType>()(ei.Type);
        std::size_t h2 = std::hash<float>()(ei.Location.x);
        std::size_t h3 = std::hash<float>()(ei.Location.y);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
