#include "../../stdafx.h"
#include "SteeringAgent.h"
#include "../SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "../SteeringBehaviors/Steering/CombinedSteeringBehaviors.h"
#include "../../SurvivorAgentMemory.h"

void SteeringAgent::Update(float dt, SteeringPlugin_Output& steering)
{
	if(m_pCurrentSteering)
	{
		auto output = m_pCurrentSteering->CalculateSteering(dt, m_pInterface);
	}
}

void SteeringAgent::Render(float dt)
{
	//Use Default Agent Rendering
	BaseAgent::Render(dt);
}

void SteeringAgent::Initialize(std::shared_ptr<SurvivorAgentMemory> pMemory)
{
	m_pWander = std::make_shared<Wander>();
	m_pWander->SetAutoOrient(false);

	m_pSeek = std::make_shared<Seek>();
	m_pLookAround = std::make_shared<LookAround>();
	m_pLookAt = std::make_shared<LookAt>();
	m_pFlee = std::make_shared<Flee>();
	m_pFlee->SetRadius(1000);

	m_pFleeLookingAt = std::make_shared<AdditiveSteering>(std::vector<ISteeringBehavior*>
	{m_pFlee.get(), m_pLookAt.get()});
	m_pFleeLookingAt->SetRunMode(true);
	m_pFleeLookingAt->SetAutoOrient(false);

	m_pFleeLookingAround = std::make_shared<AdditiveSteering>(std::vector<ISteeringBehavior*>
	{m_pFlee.get(), m_pLookAround.get(), m_pWander.get()});
	m_pFleeLookingAround->SetRadius(GetInfo().FOV_Range * 2);
	m_pFleeLookingAround->SetAutoOrient(false);

	m_pExplore = std::make_shared<Explore>();
	std::dynamic_pointer_cast<Explore>(m_pExplore)->Initialize(pMemory);

	m_pExploreArea = std::make_shared<ExploreArea>();
	std::dynamic_pointer_cast<ExploreArea>(m_pExploreArea)->Initialize(pMemory);

	m_pNavigateInfluence = std::make_shared<NavigateInfluence>();
	std::dynamic_pointer_cast<NavigateInfluence>(m_pNavigateInfluence)->Initialize(pMemory);

}



void SteeringAgent::SetToWander(bool autoOrient, bool sprint)
{
	m_pCurrentSteering = m_pWander;
}

void SteeringAgent::SetToSeek(const Elite::Vector2& target)
{
	m_pSeek->SetTarget(target);
	m_pCurrentSteering = m_pSeek;
}

void SteeringAgent::SetTarget(const Elite::Vector2& target)
{
	m_Target = std::make_shared<Elite::Vector2>(target);
}

void SteeringAgent::SetToLookAt()
{
	if (!m_Target) return;

	m_pLookAt->SetTarget(*m_Target);
	m_pCurrentSteering = m_pLookAt;
}

void SteeringAgent::SetToLookAt(const Elite::Vector2& target)
{
	m_pLookAt->SetTarget(target);
	m_pCurrentSteering = m_pLookAt;
}

void SteeringAgent::SetToLookAround()
{
	m_pCurrentSteering = m_pLookAround;
}

void SteeringAgent::SetToFlee()
{
	m_pFlee->SetTarget(*m_Target);
	m_pCurrentSteering = m_pFlee;
}
void SteeringAgent::SetToFleeLookingAt()
{
	m_pWander->SetAutoOrient(false);
	m_pFlee->SetTarget(*m_Target);
	m_pLookAt->SetTarget(*m_Target);
	m_pCurrentSteering = m_pFleeLookingAt;
}

void SteeringAgent::SetToFleeLookingAt(const Elite::Vector2& target)
{
	m_pFlee->SetTarget(target);
	m_pLookAt->SetTarget(target);
	m_pCurrentSteering = m_pFleeLookingAt;
}

void SteeringAgent::SetToExplore()
{
	// Reset behavior (so that he doesn't go to his old target)
	std::dynamic_pointer_cast<Explore>(m_pExplore)->SetReachedTarget(true);

	m_pCurrentSteering = m_pExplore;
}

bool SteeringAgent::SetToFleeLookingAround()
{
	if (!m_Target)
		return false;
	m_pFlee->SetTarget(*m_Target);
	m_pCurrentSteering = m_pFleeLookingAround;
	return true;
}

void SteeringAgent::SetToFleeLookingAround(const Elite::Vector2& target)
{
	m_pFlee->SetTarget(target);
	m_pCurrentSteering = m_pFleeLookingAround;
}

void SteeringAgent::SetRunMode(bool enabled)
{
	m_pCurrentSteering->SetRunMode(enabled);
}

void SteeringAgent::SetToNavigateInfluenceMap()
{
	m_pCurrentSteering = m_pNavigateInfluence;
}


void SteeringAgent::SetToExploreArea()
{
	// Reset behavior (so that he doesn't go to his old target)
	std::dynamic_pointer_cast<ExploreArea>(m_pExploreArea)->SetReachedTarget(true);

	m_pCurrentSteering = m_pExploreArea;
}

void SteeringAgent::SetToExploreArea(std::unordered_set<int> area)
{
	// Reset behavior (so that he doesn't go to his old target)
	std::dynamic_pointer_cast<ExploreArea>(m_pExploreArea)->SetReachedTarget(true);

	m_pCurrentSteering = m_pExploreArea;
	std::dynamic_pointer_cast<ExploreArea>(m_pExploreArea)->SetArea(area);
}

void SteeringAgent::AddToExploreArea(std::unordered_set<int> toAdd)
{
	std::dynamic_pointer_cast<ExploreArea>(m_pExploreArea)->AddToArea(toAdd);
}

bool SteeringAgent::IsAreaExplored()
{
	return std::dynamic_pointer_cast<ExploreArea>(m_pExploreArea)->IsExplored();
}
