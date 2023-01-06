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

	WorldInfo wi{ pInterface->World_GetInfo() };
	Elite::Vector2 vecToCenter{ wi.Center - nextPos };
	float weightToCenter{ vecToCenter.MagnitudeSquared() / (wi.Dimensions.x * wi.Dimensions.x)  };

	// Apply offset
	nextPos += pInterface->Agent_GetInfo().LinearVelocity.GetNormalized() * m_OffsetDistance;

	// Update wander angle
	m_WanderAngle += Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);

	// Get random position on circle
	nextPos.x += m_Radius * cosf(m_WanderAngle);
	nextPos.y += m_Radius * sinf(m_WanderAngle);

	nextPos += vecToCenter.GetNormalized() * weightToCenter;

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
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };


	if (m_ReachedTarget)
	{
		const auto& nodesInRadius{ m_pInfluenceMap->GetNodeIndicesInRadius(agentInfo.Location, agentInfo.FOV_Range * 2) };

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

		std::cout << "Explore; Setting new target\n";
		m_Target.Position = closestUnexploredNode->GetPosition();
	}
	


	if (m_Target.Position.DistanceSquared(agentInfo.Location) < agentInfo.GrabRange * agentInfo.GrabRange)
	{
		std::cout << "Explore; Reached target\n";

		m_ReachedTarget = true;
	}
	else
	{
		m_ReachedTarget = false;
	}

	Seek seek{};
	seek.SetTarget(m_Target.Position);
	steering = seek.CalculateSteering(deltaT, pInterface);

	return steering;
}

SteeringPlugin_Output ExploreArea::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };

	if (m_ReachedTarget)
	{
		////Find closest unexplored node
		//auto closestUnexploredNode{ m_pInfluenceMap->GetNode(*m_AreaToExplore.begin()) };
		//for (const int nodeIdx : m_AreaToExplore)
		//{
		//	const auto& node{ m_pInfluenceMap->GetNode(nodeIdx) };
		//	if (node->GetScanned())
		//		m_AreaToExplore.erase(nodeIdx);

		//	if (node->GetPosition().DistanceSquared(agentInfo.Location) < closestUnexploredNode->GetPosition().DistanceSquared(agentInfo.Location))
		//		closestUnexploredNode = node;
		//}

		for (auto it = m_AreaToExplore.begin(); it != m_AreaToExplore.end(); ) {
			const auto& node{ m_pInfluenceMap->GetNode(*it) };
			if (node->GetScanned())
				it = m_AreaToExplore.erase(it);
			else
				++it;
		}
		

		if(!m_AreaToExplore.empty())
			m_Target.Position = m_pInfluenceMap->GetNode(*m_AreaToExplore.begin())->GetPosition();

	}

	//TODO: grabRange * 1.5f = m_CellSize
	float arriveRange{ agentInfo.GrabRange * 1.5f };
	m_ReachedTarget = m_Target.Position.DistanceSquared(agentInfo.Location) < arriveRange * arriveRange;


	//Go to
	Seek seek{};
	seek.SetTarget(m_Target.Position);
	steering = seek.CalculateSteering(deltaT, pInterface);

	return steering;
}


SteeringPlugin_Output NavigateInfluence::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Navigating Influence\n";
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };

	// Get nodes in fov range
	const auto& nodeIndices{ m_pInfluenceMap->GetNodeIndicesInRadius(agentInfo.Location, agentInfo.FOV_Range) };
	const auto& nodes{ m_pInfluenceMap->GetNodes(nodeIndices) };

	float highestInfluence{ -100.0f };
	int highestIdx{ -1 };

	//find node with best influence
	for (const auto& node : nodes)
	{
		std::cout << "Node influence; " << node->GetInfluence() << "\n";
		if (node->GetInfluence() > highestInfluence)
		{
			highestInfluence = node->GetInfluence();
			highestIdx = node->GetIndex();
		}
	}

	if (highestIdx == -1)
	{
		LookAround lookAround{};
		steering = lookAround.CalculateSteering(deltaT, pInterface);
		return steering;
	}

	//const auto& lowestInfluenceNode{ m_pInfluenceMap->GetNode(lowestIdx) };
	const bool isDangerous{ m_pInfluenceMap->GetNodeAtWorldPos(agentInfo.Location)->GetInfluence() < -1.0f };
	SetRunMode(isDangerous); //danger, run

	// Look at danger
	LookAt lookAt{}; 
	lookAt.SetTarget(-agentInfo.LinearVelocity);
	SteeringPlugin_Output lookAtSteering{};
	lookAtSteering = lookAt.CalculateSteering(deltaT, pInterface);
	
	// Go to
	Seek seek{};
	if (m_Target.Position.DistanceSquared(agentInfo.Location) < agentInfo.GrabRange * agentInfo.GrabRange)
	{
		m_Target.Position = m_pInfluenceMap->GetNode(highestIdx)->GetPosition();
	}

	seek.SetTarget(m_Target.Position);
	steering = seek.CalculateSteering(deltaT, pInterface);

	steering.AngularVelocity += lookAtSteering.AngularVelocity;
	steering.AutoOrient = !isDangerous;
	return steering;
}
