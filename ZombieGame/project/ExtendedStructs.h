#pragma once
#include "Exam_HelperStructs.h"

struct EAgentInfo : AgentInfo
{
	Elite::Vector2 GetForward() const { return Elite::Vector2(cos(Orientation), sin(Orientation)); };
	void SetForward(Elite::Vector2 forward) { Orientation = atan2(forward.y, forward.x); };
};