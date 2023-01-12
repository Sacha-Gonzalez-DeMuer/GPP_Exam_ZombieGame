#include "../../stdafx.h"
#include "SteeringAgent.h"
#include "../SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "../SteeringBehaviors/Steering/CombinedSteeringBehaviors.h"
#include "../../SurvivorAgentMemory.h"
#include "IExamInterface.h"

void SteeringAgent::Update(float dt, SteeringPlugin_Output& steering)
{
	if(m_pCurrentSteering)
	{
		steering = m_pCurrentSteering->CalculateSteering(dt, m_pInterface);
	}
}

void SteeringAgent::Render(float dt)
{
	for (const auto& patrolPoint : std::dynamic_pointer_cast<CircularPatrol>(m_pPatrol)->GetPath())
	{
		m_pInterface->Draw_Point(patrolPoint, 10, Elite::Vector3(0, 1, 0));
	}
	BaseAgent::Render(dt);
}

void SteeringAgent::Initialize(std::shared_ptr<SurvivorAgentMemory> pMemory)
{
	m_pSeek = std::make_shared<Seek>();
	m_pLookAround = std::make_shared<LookAround>();
	m_pLookAt = std::make_shared<LookAt>();

	// Navigation to clear unseen cells of our influence map
	m_pExploreArea = std::make_shared<ClearArea>();
	std::dynamic_pointer_cast<ClearArea>(m_pExploreArea)->Initialize(pMemory);

	// Navigation guided by positive or negative influence
	m_pNavigateInfluence = std::make_shared<NavigateInfluence>();
	std::dynamic_pointer_cast<NavigateInfluence>(m_pNavigateInfluence)->Initialize(pMemory);

	// Make a circular path around the center of the map 
	//which increases range every time the path is completed
	const auto& worldInfo{ m_pInterface->World_GetInfo() };
	m_pPatrol = std::make_shared<CircularPatrol>();
	std::dynamic_pointer_cast<CircularPatrol>(m_pPatrol)
		->InitializeCircularPath(worldInfo.Center, GetInfo().FOV_Range, 6, worldInfo.Dimensions.x / 4.f);
}


void SteeringAgent::SetToSeek(const Elite::Vector2& target, bool run)
{
	m_pSeek->SetRunMode(run);
	m_pSeek->SetTarget(target);
	m_pCurrentSteering = m_pSeek;
}

void SteeringAgent::SetTarget(const Elite::Vector2& target)
{
	m_Target = std::make_shared<Elite::Vector2>(target);
}

void SteeringAgent::SetToLookAt(const Elite::Vector2& target)
{
	m_pLookAt->SetTarget(target);
	m_pCurrentSteering = m_pLookAt;
	m_pCurrentSteering->SetAutoOrient(false);
}

void SteeringAgent::SetToLookAround()
{
	m_pCurrentSteering = m_pLookAround;
}

void SteeringAgent::SetRunMode(bool enabled)
{
	if(m_pCurrentSteering)
		m_pCurrentSteering->SetRunMode(enabled);
}

void SteeringAgent::SetToNavigateInfluenceMap()
{
	m_pCurrentSteering = m_pNavigateInfluence;
}

void SteeringAgent::SetToPatrol()
{
	m_pCurrentSteering = m_pPatrol;
}

void SteeringAgent::SetToExploreArea(std::unordered_set<int> area)
{
	// Reset behavior (so that he doesn't go to his old target)
	std::dynamic_pointer_cast<ClearArea>(m_pExploreArea)->SetReachedTarget(true);

	m_pCurrentSteering = m_pExploreArea;
	std::dynamic_pointer_cast<ClearArea>(m_pExploreArea)->SetArea(area);
}

bool SteeringAgent::IsAreaExplored() const
{
	return std::dynamic_pointer_cast<ClearArea>(m_pExploreArea)->IsExplored();
}
