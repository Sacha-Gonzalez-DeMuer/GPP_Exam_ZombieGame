#pragma once
#include "Exam_HelperStructs.h"
#include <functional>

struct EAgentInfo : AgentInfo
{
	EAgentInfo(const AgentInfo& info) : AgentInfo(info) {}

    float LowEnergyThreshold{ 8.0f };
    float LowHealthThreshold{ 8.0f };

	Elite::Vector2 GetForward() const { return Elite::Vector2(cosf(Orientation), sinf(Orientation)); };
	void SetForward(Elite::Vector2 forward) { Orientation = atan2(forward.y, forward.x); };
};

struct EHouseInfo : HouseInfo
{
	EHouseInfo() : HouseInfo() {}
	EHouseInfo(const HouseInfo& info) : HouseInfo(info){}

	bool Cleared{false};
	float ResetTime{ 600.0f };
	float ResetTimer{ 600.0f };

	bool UpdateResetTime(float deltaTime)
	{
		if (ResetTimer > 0)
			ResetTimer -= deltaTime;
		else
		{
			Cleared = false;
			ResetTimer = ResetTime;
			return true;
		}
		return false;
	}

	bool operator==(const EHouseInfo& other) const
	{
		return Center == other.Center && Size == other.Size;
	}

	bool operator==(HouseInfo&& other) const
	{
		return Center == other.Center && Size == other.Size;
	}
};

