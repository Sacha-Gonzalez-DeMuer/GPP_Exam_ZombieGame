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
	std::cout << "Wandering\n";
	SteeringPlugin_Output steering{};

	Elite::Vector2 nextPos{ pAgent->Position };

	//apply offset
	nextPos += pAgent->LinearVelocity.GetNormalized() * m_OffsetDistance;

	m_WanderAngle += Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);

	//get random pos on circle
	nextPos.x += m_Radius * cosf(m_WanderAngle);
	nextPos.y += m_Radius * sinf(m_WanderAngle);
		
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

	steering.AutoOrient = true;
	steering.AngularVelocity = pAgent->MaxAngularSpeed;
	
	const float cosAngle{ std::cos(m_DegreesPerSecond) };
	const float sinAngle{ std::sin(m_DegreesPerSecond) };

	auto extendedAgent{  static_cast<const EAgentInfo*>(pAgent) };

	Elite::Vector2 rotated{};
	rotated.x = extendedAgent->GetForward().x * cosAngle - extendedAgent->GetForward().y * sinAngle;
	rotated.y = extendedAgent->GetForward().x * sinAngle - extendedAgent->GetForward().y * cosAngle;
	
	steering.LinearVelocity = rotated;

	return steering;
}

SteeringPlugin_Output LookAt::CalculateSteering(float deltaT, const AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;

	Elite::Vector2 dir_vector = m_Target.Position - pAgent->Position;
	dir_vector.Normalize();

	const float target_angle = atan2f(dir_vector.y, dir_vector.x);
	const float agent_angle = pAgent->Orientation;
	const float delta_angle = target_angle - agent_angle;

	if (abs(target_angle - agent_angle) <= .02f)
	{
		return steering;
	}

	steering.AngularVelocity = pAgent->MaxAngularSpeed;
	if (delta_angle < 0 || delta_angle > static_cast<float>(M_PI))
	{
		steering.AngularVelocity = -pAgent->MaxAngularSpeed;
	}

	return steering;
}
