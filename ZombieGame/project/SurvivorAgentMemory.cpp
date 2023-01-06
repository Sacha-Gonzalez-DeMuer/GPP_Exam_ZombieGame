#include "stdafx.h"
#include "SurvivorAgentMemory.h"
#include "IExamInterface.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphUtilities/EGraphRenderer.h"

SurvivorAgentMemory::SurvivorAgentMemory(IExamInterface* pInterface)
	: m_pInterface{ pInterface }
	,m_PropagationRadius{ pInterface->Agent_GetInfo().FOV_Range * 3 }

{
	//Initialize InfluenceMap
	m_pInfluenceMap = new Elite::InfluenceMap<InfluenceGrid>(false);
	float worldDimension{ max(pInterface->World_GetInfo().Dimensions.x, pInterface->World_GetInfo().Dimensions.y) };
	int celSize{ static_cast<int>(pInterface->Agent_GetInfo().GrabRange * 1.5f) };
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

	

}

void SurvivorAgentMemory::UpdateInfluenceMap(float deltaTime, IExamInterface* pInterface)
{
	m_pInfluenceMap->PropagateInfluence(deltaTime, pInterface->Agent_GetInfo().Location, m_PropagationRadius);
	pInterface->Draw_Circle(pInterface->Agent_GetInfo().Location, m_PropagationRadius, { 0,1,0 });
}

void SurvivorAgentMemory::DebugRender(IExamInterface* pInterface) const
{
	RenderInfluenceMap(pInterface);
}

std::unordered_set<int> SurvivorAgentMemory::GetHouseArea(const HouseInfo& house)
{
	return m_pInfluenceMap->GetNodeIndicesInRect(house.Center, house.Size);
}

bool SurvivorAgentMemory::OnPickUpItem(const ItemInfo& item)
{
	auto node{ m_pInfluenceMap->GetNodeAtWorldPos(item.Location) };
	if (node->GetItem() != item.Type)
		return false;

	node->RemoveItem();
	m_LocatedItems.erase(node->GetIndex());
	return true;
}

bool SurvivorAgentMemory::OnPickUpItem(const EntityInfo& entity)
{
	// Confirm entity is an item
	if (entity.Type != eEntityType::ITEM)
		return false;
	ItemInfo item{};
	if (!m_pInterface->Item_GetInfo(entity, item))
		return false;

	// If item type on node is not the one picked up, return false;
	auto node{ m_pInfluenceMap->GetNodeAtWorldPos(entity.Location) };
	if (node->GetItem() != item.Type)
		return false;

	// Remove from located items
	node->RemoveItem();
	m_LocatedItems.erase(node->GetIndex());
	return true;
}

void SurvivorAgentMemory::AddToSeenHouses(HouseInfo houseInfo) 
{
	// Check if house already seen
	const auto houseNode{ m_pInfluenceMap->GetNodeAtWorldPos(houseInfo.Center) };
	for (auto i = 0; i < m_SeenHouses.size(); i++)
	{
		if (m_pInfluenceMap->GetNodeAtWorldPos(m_SeenHouses[i].Center)->GetIndex() == houseNode->GetIndex())
			return;
	}

	for (auto i = 0; i < m_ClearedHouses.size(); i++)
	{
		if (m_pInfluenceMap->GetNodeAtWorldPos(m_ClearedHouses[i].Center)->GetIndex() == houseNode->GetIndex())
			return;
	}


	// If not seen save it
	m_SeenHouses.emplace_back(houseInfo);
}

void SurvivorAgentMemory::AddToVisitedHouses(HouseInfo houseInfo)
{
	const auto houseNode{ m_pInfluenceMap->GetNodeAtWorldPos(houseInfo.Center) };

	// Remove house from seen houses
	for (auto it = m_SeenHouses.begin(); it != m_SeenHouses.end();)
	{
		if (m_pInfluenceMap->GetNodeAtWorldPos(it->Center)->GetIndex() == houseNode->GetIndex()) {
			it = m_SeenHouses.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	// Check if house visited seen
	for (auto i = 0; i < m_ClearedHouses.size(); i++)
	{
		if (m_pInfluenceMap->GetNodeAtWorldPos(m_ClearedHouses[i].Center)->GetIndex() == houseNode->GetIndex())
			return;
	}

	// If not seen save it and save it
	m_ClearedHouses.emplace_back(houseInfo);
}

bool SurvivorAgentMemory::IsHouseCleared(const HouseInfo& houseInfo)
{
	const auto houseNodes{ m_pInfluenceMap->GetNodeIndicesInRect(houseInfo.Center, houseInfo.Size) };
	
	return IsAreaExplored(houseNodes);
}

bool SurvivorAgentMemory::IsHouseCleared(const HouseInfo& houseInfo, std::unordered_set<int>& area)
{
	area = m_pInfluenceMap->GetNodeIndicesInRect(houseInfo.Center, houseInfo.Size);
	return IsAreaExplored(area);
}

bool SurvivorAgentMemory::IsHouseCleared(std::unordered_set<int>& unscannedArea, const HouseInfo& houseInfo)
{
	return IsAreaExplored(m_pInfluenceMap->GetNodeIndicesInRect(houseInfo.Center, houseInfo.Size), unscannedArea);
}

void SurvivorAgentMemory::UpdateHouses(IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV)
{
	for (const auto& house : housesInFOV)
	{
		AddToSeenHouses(*house);
	}

	if (m_SeenHouses.empty())
		return;

	auto agentPos{ pInterface->Agent_GetInfo().Location };
	auto GetNodeIdxAtPos
	{ [&](Elite::Vector2 pos)->int { return m_pInfluenceMap->GetNodeAtWorldPos(pos)->GetIndex(); } };

	
	for (const auto& house : m_SeenHouses)
	{
		//if (house.Center.DistanceSquared(agentPos) > pInterface->Agent_GetInfo().FOV_Range * pInterface->Agent_GetInfo().FOV_Range)
		//	continue;

		if (IsHouseCleared(house))
			AddToVisitedHouses(house);
	}

	//for (const auto& house : housesInFOV)
	//{
	//	bool houseCleared{ true };
	//	int count{ 0 };
	//	for (const auto& nodeIdx : GetHouseArea(*house))
	//	{
	//		++count;
	//		if (!m_pInfluenceMap->GetNode(nodeIdx)->GetScanned())
	//		{
	//			houseCleared = false;
	//			break;
	//		}
	//	}

	//	if (houseCleared)
	//	{
	//		AddToVisitedHouses(*house);
	//	}
	//}
}


void SurvivorAgentMemory::LocateItem(const ItemInfo& item)
{
	auto node{ m_pInfluenceMap->GetNodeAtWorldPos(item.Location) };

	node->SetItem(item);
	m_LocatedItems.insert(node->GetIndex());
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
		{
			ItemInfo info{  };
			if (!pInterface->Item_GetInfo(*e, info))
				return;

			LocateItem(info);
		}

		if (e->Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo purgeZone{};
			if (pInterface->PurgeZone_GetInfo(*e, purgeZone))
			{
				m_pInfluenceMap->SetInfluenceAtPosition(purgeZone.Center, -purgeZone.Radius);
			}
		}
	}

	if (eAgentInfo.WasBitten) m_pInfluenceMap->SetInfluenceAtPosition(eAgentInfo.Location, -100);
}


void SurvivorAgentMemory::RenderInfluenceMap(IExamInterface* pInterface) const
{
	m_pInfluenceMap->SetNodeColorsBasedOnInfluence();

	auto visibleNodes{ m_pInfluenceMap->GetNodeIndicesInRadius(pInterface->Agent_GetInfo().Location, m_PropagationRadius) };
	m_pGraphRenderer->RenderNodes(m_pInfluenceMap, pInterface, visibleNodes, true, false, false, false);
}

bool SurvivorAgentMemory::HasVisitedHouse(const HouseInfo& house) const
{
	Elite::Vector2 houseSize{ house.Size.x - m_pInfluenceMap->GetCellSize(), house.Size.y - m_pInfluenceMap->GetCellSize() };
	auto nodesInHouse{ m_pInfluenceMap->GetNodeIndicesInRect(house.Center, houseSize) };

	int nrCellsCleared{ 0 };
	bool houseCleared{ true };
	for (const auto& nodeIdx : nodesInHouse)
	{
		if (m_pInfluenceMap->GetNode(nodeIdx)->GetScanned())
			++nrCellsCleared;
	}

	return static_cast<float>(nrCellsCleared) / static_cast<float>(nodesInHouse.size()) > m_PercentageToClear;
}

bool SurvivorAgentMemory::IsAreaExplored(std::unordered_set<int> area) const
{
	int nrCellsCleared{ 0 };
	for (int i : area)
	{
		if (m_pInfluenceMap->GetNode(i)->GetScanned())
			++nrCellsCleared;
	}

	return static_cast<float>(nrCellsCleared) / static_cast<float>(area.size()) > m_PercentageToClear;
}

bool SurvivorAgentMemory::IsAreaExplored(std::unordered_set<int> area, std::unordered_set<int>& unscannedArea) const
{
	int nrCellsCleared{ 0 };
	for (int i : area)
	{
		if (m_pInfluenceMap->GetNode(i)->GetScanned())
		{
			++nrCellsCleared;
			unscannedArea.insert(i);
		}
	}

	return static_cast<float>(nrCellsCleared) / static_cast<float>(area.size()) > m_PercentageToClear;
}

