#include "../../../stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../../../ExtendedStructs.h"
#include "../../EliteMath/EMatrix2x3.h"
#include "../../EliteMath/EVector2.h"
#include "../../EliteMath/EMathUtilities.h"

//
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	std::cout << "Seeking\n";
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

SteeringPlugin_Output Flee::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	const Elite::Vector2 fromTarget{ pAgent->Position - m_Target.Position };
	const float sqrtDistance{ fromTarget.MagnitudeSquared() };

	if (sqrtDistance > m_Radius * m_Radius)
	{
		return steering;
	}
	steering.LinearVelocity = fromTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	const float arriveDistance{ 50.f };
	const float stopDistance{ 10.f };
	const Elite::Vector2 vecToTarget{ m_Target.Position - pAgent->Position };


	steering.LinearVelocity = vecToTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	if (vecToTarget.Magnitude() < arriveDistance)
	{
		steering.LinearVelocity *= steering.LinearVelocity.Magnitude() * (vecToTarget.Magnitude() / arriveDistance);
	}

	if (vecToTarget.Magnitude() <= stopDistance)
	{
		steering.LinearVelocity *= 0;
	} 


	return steering;
}

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};

	Elite::Vector2 nextPos{ pAgent->Position };

	// Apply offset
	nextPos += pAgent->LinearVelocity.GetNormalized() * m_OffsetDistance;

	// Update wander angle
	m_WanderAngle += Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);

	// Get random position on circle
	nextPos.x += m_Radius * cosf(m_WanderAngle);
	nextPos.y += m_Radius * sinf(m_WanderAngle);

	// Calculate steering force
	steering.LinearVelocity = nextPos - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

void Wander::SetWanderOffset(float offset)
{
	m_OffsetDistance = offset;
}

SteeringPlugin_Output Evade::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};

	return steering;
}

SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	Elite::Vector2 futurePos{ pAgent->Position + pAgent->LinearVelocity };
	

	return steering;
}

SteeringPlugin_Output LookAround::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	std::cout << "Looking Around\n";

	steering.AutoOrient = false;
	steering.AngularVelocity = pAgent->MaxAngularSpeed;

	return steering;
}

SteeringPlugin_Output LookAt::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;

	Elite::Vector2 dir_vector = m_Target.Position - pAgent->Position;
	dir_vector.Normalize();

	const float targetAngle = atan2f(dir_vector.y, dir_vector.x);
	const float agentAngle = pAgent->Orientation;
	const float deltaAngle = targetAngle - agentAngle;

	if (abs(targetAngle - agentAngle) <= .02f)
	{
		return steering;
	}

	steering.AngularVelocity = pAgent->MaxAngularSpeed;

	// Check the shortest rotation direction
	if (deltaAngle < -M_PI)
	{
		steering.AngularVelocity = -steering.AngularVelocity;
	}
	else if (deltaAngle > M_PI)
	{
		steering.AngularVelocity = -steering.AngularVelocity;
	}
	else if (deltaAngle < 0)
	{
		steering.AngularVelocity = -steering.AngularVelocity;
	}

	return steering;

	//SteeringPlugin_Output steering = {};
	//steering.AutoOrient = false;

	//Elite::Vector2 dir_vector = m_Target.Position - pAgent->Position;
	//dir_vector.Normalize();

	//const float target_angle = atan2f(dir_vector.y, dir_vector.x);
	//const float agent_angle = pAgent->Orientation;
	//const float delta_angle = target_angle - agent_angle;

	//if (abs(target_angle - agent_angle) <= .02f)
	//{
	//	return steering;
	//}

	//steering.AngularVelocity = pAgent->MaxAngularSpeed;
	//if (delta_angle < 0 || delta_angle > static_cast<float>(M_PI))
	//{
	//	steering.AngularVelocity = -pAgent->MaxAngularSpeed;
	//}

	//return steering;
}
