#include "stdafx.h"
#include "SurvivorAgentMemory.h"
#include "IExamInterface.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphUtilities/EGraphRenderer.h"


SurvivorAgentMemory::SurvivorAgentMemory(IExamInterface* pInterface)
	: m_PropagationRadius{ pInterface->Agent_GetInfo().FOV_Range }
{
	//Initialize InfluenceMap
	m_pInfluenceMap = new Elite::InfluenceMap<InfluenceGrid>(false);
	float worldDimension{ max(pInterface->World_GetInfo().Dimensions.x, pInterface->World_GetInfo().Dimensions.y) };
	int celSize{ static_cast<int>(pInterface->Agent_GetInfo().GrabRange) };
	int colRows{ static_cast<int>(worldDimension) / celSize };

	m_pInfluenceMap->InitializeGrid({ -worldDimension / 2.f, -worldDimension / 2.f }, colRows, colRows, celSize, false, true);
	m_pInfluenceMap->InitializeBuffer();
	m_pInfluenceMap->SetMomentum(.5f);
	m_pInfluenceMap->SetDecay(.2f);

	m_pGraphRenderer = new Elite::GraphRenderer();
}

SurvivorAgentMemory::~SurvivorAgentMemory()
{
	delete m_pInfluenceMap;
	m_pInfluenceMap = nullptr;

	delete m_pGraphRenderer;
	m_pGraphRenderer = nullptr;
}

void SurvivorAgentMemory::Update(float deltaTime, IExamInterface* pInterface, std::vector<EntityInfo*> entitiesInFOV, std::vector<HouseInfo*> housesInFOV)
{
	UpdateHouses(pInterface, housesInFOV);
	UpdateSeenCells(pInterface, entitiesInFOV);
	UpdateInfluenceMap(deltaTime, pInterface);

	if (m_CooldownTimer > m_CooldownTime)
	{
		for (const auto& entity : entitiesInFOV)
		{
			AddToSeenItems(*entity);
		}

		for (const auto& house : housesInFOV)
		{
			AddToSeenHouses(*house);
		}
	}
	else
	{
		m_CooldownTimer += deltaTime;
	}
}

void SurvivorAgentMemory::UpdateInfluenceMap(float deltaTime, IExamInterface* pInterface)
{
	m_pInfluenceMap->PropagateInfluence(deltaTime, pInterface->Agent_GetInfo().Location, m_PropagationRadius);
	pInterface->Draw_Circle(pInterface->Agent_GetInfo().Location, m_PropagationRadius, { 0,1,0 });
}

void SurvivorAgentMemory::DebugRender(IExamInterface* pInterface) const
{
	RenderInfluenceMap(pInterface);

	//Render circle around seen items
	for (const auto& item : m_SeenItems)
	{
		pInterface->Draw_Circle(item.Location, 3, { 1,.5f,.5f });
	}
}

void SurvivorAgentMemory::RenderInfluenceMap(IExamInterface* pInterface) const
{
	m_pInfluenceMap->SetNodeColorsBasedOnInfluence();

	auto visibleNodes{ GetVisibleNodes(pInterface) };
	m_pGraphRenderer->RenderNodes(m_pInfluenceMap, pInterface, visibleNodes, true, false, false, false);

}

void SurvivorAgentMemory::AddToSeenItems(const EntityInfo& item)
{
	bool alreadySeen{ false };
	for (auto seenItem : m_SeenItems)
	{
		if (seenItem.Location == item.Location)
			alreadySeen = true;
	}

	if (item.Type == eEntityType::ITEM && !alreadySeen)
		m_SeenItems.emplace_back(item);
}


void SurvivorAgentMemory::OnPickUpItem(const EntityInfo& item)
{
	// Iterate over all items in the m_SeenItems set
	for (auto it = m_SeenItems.begin(); it != m_SeenItems.end(); )
	{
		// Calculate the distance between the given item and the current seen item
		float distance = item.Location.DistanceSquared(it->Location);
		float threshold{ 5.f };
		// If the distance is within a certain threshold, delete the item from the set
		if (distance < threshold)
		{
			it = m_SeenItems.erase(it);
		}
		else
		{
			++it;
		}
	}

	m_CooldownTimer = 0;
}

void SurvivorAgentMemory::AddToSeenHouses( HouseInfo houseInfo) 
{
	for (auto i = 0; i != m_SeenHouses.size(); i++)
	{
		if (m_SeenHouses[i].Center == houseInfo.Center)
			return;
	}

	m_SeenHouses.emplace_back(houseInfo);
}

void SurvivorAgentMemory::AddToVisitedHouses(HouseInfo houseInfo)
{
	for (auto i = 0; i != m_VisitedHouses.size(); i++)
	{
		if (m_VisitedHouses[i].Center == houseInfo.Center)
			return;
	}

	m_VisitedHouses.emplace_back(houseInfo);
}

bool SurvivorAgentMemory::IsHouseVisited(const HouseInfo& houseInfo)
{
	if (m_VisitedHouses.empty())
		return false;

	for (const auto& house : m_VisitedHouses)
	{
		if (houseInfo.Center.DistanceSquared(house.Center) < min(houseInfo.Size.x, houseInfo.Size.y))
		{
			return true;
		}

	}

	return false;
}

void SurvivorAgentMemory::UpdateHouses(IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV)
{
	for (const auto& house : housesInFOV)
	{
		if (pInterface->Agent_GetInfo().Location.DistanceSquared(house->Center) < m_pInfluenceMap->GetCellSize())
			AddToVisitedHouses(*house);
	}
}

void SurvivorAgentMemory::UpdateSeenItems(IExamInterface* pInterface)
{
	if (m_SeenItems.empty())
		return;
}


void SurvivorAgentMemory::UpdateSeenCells(IExamInterface* pInterface, std::vector<EntityInfo*> entitiesInFOV) 
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();
	const Elite::Vector2 scanPos{ eAgentInfo.Location + (eAgentInfo.GetForward() * eAgentInfo.FOV_Range / 2.0f) };
	const float scanRadius{ eAgentInfo.FOV_Range / 2.0f };
	const auto& indices{ m_pInfluenceMap->GetNodeIndicesInRadius(scanPos, scanRadius) };

	m_pInfluenceMap->SetScannedAtPosition(m_pInfluenceMap->GetNodeAtWorldPos(eAgentInfo.Location)->GetIndex(), true);
	m_pInfluenceMap->SetScannedAtPosition(indices, true);

	for (const auto& e : entitiesInFOV)
	{
		if (e->Type == eEntityType::ENEMY)
			m_pInfluenceMap->SetInfluenceAtPosition(e->Location, -20);

		if (e->Type == eEntityType::ITEM)
			m_pInfluenceMap->SetInfluenceAtPosition(e->Location, 50);
	}

	if (eAgentInfo.WasBitten) m_pInfluenceMap->SetInfluenceAtPosition(eAgentInfo.Location, -100);
}

std::unordered_set<int> SurvivorAgentMemory::GetVisibleNodes(IExamInterface* pInterface) const
{
	return m_pInfluenceMap->GetNodeIndicesInRadius(pInterface->Agent_GetInfo().Location, m_PropagationRadius);
}