#include "../../../stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../../../ExtendedStructs.h"
#include "../../EliteMath/EMatrix2x3.h"
#include "../../EliteMath/EVector2.h"
#include "../../EliteMath/EMathUtilities.h"
#include "../../EliteAI/EliteGraphs/EInfluenceMap.h"
#include "../../EliteGeometry/EGeometry2DUtilities.h"
#include "IExamInterface.h"

//
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering = {};
	
	steering.LinearVelocity = pInterface->NavMesh_GetClosestPathPoint(m_Target.Position) - pInterface->Agent_GetInfo().Location;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pInterface->Agent_GetInfo().MaxLinearSpeed;
	steering.RunMode = m_RunMode;
	steering.AutoOrient = m_AutoOrient;

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
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = false;
	const EAgentInfo& agentInfo = pInterface->Agent_GetInfo();

	Elite::Vector2 toTarget = m_Target.Position - agentInfo.Location;
	toTarget.Normalize();

	Elite::Vector2 forward = agentInfo.GetForward();

	float crossProduct = forward.Cross(toTarget);
	if (crossProduct >= 0)
	{
		steering.AngularVelocity = agentInfo.MaxAngularSpeed;;
	}
	else if (crossProduct < 0)
	{
		steering.AngularVelocity = -agentInfo.MaxAngularSpeed;
	}

	return steering;
}

SteeringPlugin_Output ClearArea::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Exploring Area\n";
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };


	if (m_ReachedTarget)
	{
		for (auto it = m_AreaToClear.begin(); it != m_AreaToClear.end(); ) {
			const auto& node{ m_pInfluenceMap->GetNode(*it) };
			if (node->GetScanned())
				it = m_AreaToClear.erase(it);
			else
				++it;
		}
		
		if(!m_AreaToClear.empty())
			m_Target.Position = m_pInfluenceMap->GetNode(*m_AreaToClear.begin())->GetPosition();
	}

	int arriveRange{ m_pInfluenceMap->GetCellSize() };
	m_ReachedTarget = m_Target.Position.DistanceSquared(agentInfo.Location) < arriveRange * arriveRange;


	//Go to
	SetTarget(m_Target.Position);
	steering = Seek::CalculateSteering(deltaT, pInterface);

	steering.RunMode = m_RunMode;
	return steering;
}


SteeringPlugin_Output NavigateInfluence::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Navigating Influence\n";
	SteeringPlugin_Output steering{};
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };


	const float influenceWeight = 1.f; // This variable can be adjusted to increase or decrease the weight of influence
	// Get nodes in fov range
	const auto& nodeIndices{ m_pInfluenceMap->GetNodeIndicesInRadius(agentInfo.Location, agentInfo.FOV_Range * 4) };
	const auto& nodes{ m_pInfluenceMap->GetNodes(nodeIndices) };

	for (const auto& node : nodes)
	{
		Elite::Vector2 nodePos{ node->GetPosition() };
		Elite::Vector2 toNodeVec = nodePos - agentInfo.Location;
		float distSq = toNodeVec.MagnitudeSquared();
		toNodeVec.Normalize();
		float influence = node->GetInfluence();

		// Subtract low influence and add high influence
		steering.LinearVelocity -= toNodeVec * influenceWeight * influence * (1.f - distSq / (agentInfo.FOV_Range * agentInfo.FOV_Range));
	}

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	// Normalize the vector
	const bool isDangerous{ m_pInfluenceMap->GetNodeAtWorldPos(agentInfo.Location)->GetInfluence() < -10.0f };
	steering.RunMode = isDangerous;

	steering.AngularVelocity = agentInfo.MaxAngularSpeed;
	steering.AutoOrient = false;
	return steering;
}

SteeringPlugin_Output CircularPatrol::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	std::cout << "Patrolling\n";
	SteeringPlugin_Output steering{};
	Elite::Vector2 currentPatrolPoint{ m_PatrolPoints[m_PatrolIdx] };
	const AgentInfo& agentInfo{ pInterface->Agent_GetInfo() };
	const WorldInfo worldInfo{ pInterface->World_GetInfo() };

	// Go to patrol point
	SetTarget(m_PatrolPoints[m_PatrolIdx]);

	steering = Seek::CalculateSteering(deltaT, pInterface);
	steering.LinearVelocity += Wander::CalculateSteering(deltaT, pInterface).LinearVelocity; // Add wander to make patrol a little random

	if (agentInfo.Location.DistanceSquared(currentPatrolPoint) < agentInfo.FOV_Range * agentInfo.FOV_Range)
	{
		m_PatrolIdx = (m_PatrolIdx+1) % static_cast<int>(m_PatrolPoints.size());

		// When entire path has been patrolled, extend patrol radius
		if (m_PatrolIdx == 0)
		{
			const int nrPoints{ static_cast<int>(m_PatrolPoints.size()) };
			m_PatrolPoints.clear();

			// If radius would extend beyong the map, start shrinking
			if (m_Radius + m_ExtensionLength > max(worldInfo.Dimensions.x, worldInfo.Dimensions.y))
				m_ExtensionLength = -m_ExtensionLength / 2;
			// Otherwise if radius would become to small, start extending again
			else if (m_Radius + m_ExtensionLength <= agentInfo.FOV_Range)
				m_ExtensionLength = abs(m_ExtensionLength);


			InitializeCircularPath(m_Center, m_Radius + m_ExtensionLength, nrPoints, m_ExtensionLength);
		}
	}

	return steering;
}

void CircularPatrol::InitializeCircularPath(const Elite::Vector2& center, float radius, int nrPoints, float extensionLength)
{
	m_Center = center;
	m_PatrolRadius = m_PatrolRadius;
	m_ExtensionLength = extensionLength;

	float deltaAngle{ Elite::ToRadians(360.0f / nrPoints) };
	Elite::Vector2 point{};

	for (int i{ 0 }; i < nrPoints; ++i)
	{
		point.x = radius * cosf(i * deltaAngle) + center.x;
		point.y = radius * sinf(i * deltaAngle) + center.y;
		m_PatrolPoints.push_back(point);
	}
}
