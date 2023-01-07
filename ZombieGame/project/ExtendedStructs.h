#pragma once
#include "Exam_HelperStructs.h"
#include <functional>

struct EAgentInfo : AgentInfo
{
	EAgentInfo(const AgentInfo& info) : AgentInfo(info) {}


    float LowEnergyThreshold{ 6.0f };
    float LowHealthThreshold{ 5.0f };

	Elite::Vector2 GetForward() const { return Elite::Vector2(cos(Orientation), sin(Orientation)); };
	void SetForward(Elite::Vector2 forward) { Orientation = atan2(forward.y, forward.x); };
};

struct EHouseInfo : HouseInfo
{
	EHouseInfo() : HouseInfo() {}
	EHouseInfo(const HouseInfo& info) : HouseInfo(info){}

	bool Cleared{false};

	bool operator==(const EHouseInfo& other) const
	{
		return Center == other.Center && Size == other.Size;
	}

	bool operator==(HouseInfo&& other) const
	{
		return Center == other.Center && Size == other.Size;
	}
};

