#include "../../../stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../../../ExtendedStructs.h"
#include "../../EliteMath/EMatrix2x3.h"
#include "../../EliteMath/EVector2.h"
#include "../../EliteMath/EMathUtilities.h"
#include "../../EliteAI/EliteGraphs/EInfluenceMap.h"
#include "IExamInterface.h"

//
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering = {};

	
	steering.LinearVelocity = pInterface->NavMesh_GetClosestPathPoint(m_Target.Position) - pInterface->Agent_GetInfo().Location;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pInterface->Agent_GetInfo().MaxLinearSpeed;

	return steering;
}

SteeringPlugin_Output Flee::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{

	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;
	const Elite::Vector2 fromTarget{ pInterface->Agent_GetInfo().Location - m_Target.Position };
	const float sqrtDistance{ fromTarget.MagnitudeSquared() };

	if (sqrtDistance > m_Radius * m_Radius)
	{
		return steering;
	}
	steering.LinearVelocity = fromTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pInterface->Agent_GetInfo().MaxLinearSpeed;
	return steering;
}

SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering = {};
	const float arriveDistance{ 50.f };
	const float stopDistance{ 10.f };
	const Elite::Vector2 vecToTarget{ m_Target.Position - pInterface->Agent_GetInfo().Location };


	steering.LinearVelocity = vecToTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pInterface->Agent_GetInfo().MaxLinearSpeed;

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

SteeringPlugin_Output Wander::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering{};

	Elite::Vector2 nextPos{ pInterface->Agent_GetInfo().Location };

	// Apply offset
	nextPos += pInterface->Agent_GetInfo().LinearVelocity.GetNormalized() * m_OffsetDistance;

	// Update wander angle
	m_WanderAngle += Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);

	// Get random position on circle
	nextPos.x += m_Radius * cosf(m_WanderAngle);
	nextPos.y += m_Radius * sinf(m_WanderAngle);

	// Calculate steering force
	steering.LinearVelocity = nextPos - pInterface->Agent_GetInfo().Location;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pInterface->Agent_GetInfo().MaxLinearSpeed;

	return steering;
}

void Wander::SetWanderOffset(float offset)
{
	m_OffsetDistance = offset;
}

SteeringPlugin_Output Evade::CalculateSteering(float deltaT, const IExamInterface* pAgent)
{
	SteeringPlugin_Output steering{};

	return steering;
}

SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering{};
	Elite::Vector2 futurePos{ pInterface->Agent_GetInfo().Location + pInterface->Agent_GetInfo().LinearVelocity };
	

	return steering;
}

SteeringPlugin_Output LookAround::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering{};
	std::cout << "Looking Around\n";

	steering.AutoOrient = false;
	steering.AngularVelocity = pInterface->Agent_GetInfo().MaxAngularSpeed;

	return steering;
}

SteeringPlugin_Output LookAt::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Looking at\n";
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;

	Elite::Vector2 dir_vector = m_Target.Position - pInterface->Agent_GetInfo().Location;
	dir_vector.Normalize();

	const float targetAngle = atan2f(dir_vector.y, dir_vector.x);
	const float agentAngle = pInterface->Agent_GetInfo().Orientation;
	const float deltaAngle = targetAngle - agentAngle;

	if (abs(targetAngle - agentAngle) <= .02f)
	{
		return steering;
	}

	steering.AngularVelocity = pInterface->Agent_GetInfo().MaxAngularSpeed;

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
}


SteeringPlugin_Output Explore::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Exploring\n";
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };


	const auto& nodesInRadius{ m_pInfluenceMap->GetNodeIndicesInRadius(agentInfo.Location, agentInfo.FOV_Range*3) };
	std::vector<int> unexploredNodes{};
	for (const auto& nodeIdx : nodesInRadius)
	{
		const auto& node{ m_pInfluenceMap->GetNode(nodeIdx) };

		if (!node->GetScanned())
			unexploredNodes.emplace_back(nodeIdx);
	}

	if (unexploredNodes.empty())
		return Wander::CalculateSteering(deltaT, pInterface);

	auto closestUnexploredNode{ m_pInfluenceMap->GetNode(unexploredNodes[0]) };
	for (const int unexploredIdx : unexploredNodes)
	{
		const auto& unexploredNode{ m_pInfluenceMap->GetNode(unexploredIdx) };

		if (unexploredNode->GetPosition().DistanceSquared(agentInfo.Location) < closestUnexploredNode->GetPosition().DistanceSquared(agentInfo.Location))
			closestUnexploredNode = unexploredNode;
	}

	Seek seek{};
	seek.SetTarget(closestUnexploredNode->GetPosition());
	steering = seek.CalculateSteering(deltaT, pInterface);

	return steering;
}

SteeringPlugin_Output ExploreArea::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Exploring\n";
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };


	std::unordered_set<int> unexploredNodes{};
	for (const auto& nodeIdx : m_pAreaToExplore)
	{
		const auto& node{ m_pInfluenceMap->GetNode(nodeIdx) };

		if (!node->GetScanned())
			unexploredNodes.insert(nodeIdx);
		else
			m_pAreaToExplore.erase(nodeIdx);
	}

	if (unexploredNodes.empty())
		return Wander::CalculateSteering(deltaT, pInterface);

	auto closestUnexploredNode{ m_pInfluenceMap->GetNode(*unexploredNodes.begin()) };
	for (const int unexploredIdx : unexploredNodes)
	{
		const auto& unexploredNode{ m_pInfluenceMap->GetNode(unexploredIdx) };

		if (unexploredNode->GetPosition().DistanceSquared(agentInfo.Location) < closestUnexploredNode->GetPosition().DistanceSquared(agentInfo.Location))
			closestUnexploredNode = unexploredNode;
	}

	Seek seek{};
	seek.SetTarget(closestUnexploredNode->GetPosition());
	steering = seek.CalculateSteering(deltaT, pInterface);

	return steering;
}
