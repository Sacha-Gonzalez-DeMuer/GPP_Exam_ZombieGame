#include "stdafx.h"
#include "SurvivorAgentMemory.h"
#include "IExamInterface.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphUtilities/EGraphRenderer.h"
#include "framework/EliteGeometry/EGeometry2DUtilities.h"

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
	m_pInfluenceMap->SetMomentum(.3f);
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
	UpdateHouses(deltaTime, pInterface, housesInFOV);
	UpdateEntities(pInterface, entitiesInFOV);
	UpdateInfluenceMap(deltaTime, pInterface);
}

void SurvivorAgentMemory::UpdateInfluenceMap(float deltaTime, IExamInterface* pInterface)
{
	m_pInfluenceMap->PropagateInfluence(deltaTime, pInterface->Agent_GetInfo().Location, m_PropagationRadius);
}

// Get indices of the cells in the house area
std::unordered_set<int> SurvivorAgentMemory::GetHouseArea(const HouseInfo& house)
{
	return m_pInfluenceMap->GetNodeIndicesInRect(house.Center, { house.Size.x - m_pInfluenceMap->GetCellSize(), house.Size.y - m_pInfluenceMap->GetCellSize()});
}

void SurvivorAgentMemory::ForgetArea(std::unordered_set<int> area)
{
	// Reset all the given nodes' scanned status
	for (const auto& idx : area)
	{
		m_pInfluenceMap->GetNode(idx)->SetScanned(false);
	}
}

bool SurvivorAgentMemory::OnPickUpItem(const ItemInfo& item)
{
	// Make sure we're picking up the correct node since we're only saving 1 item per node right now
	auto node{ m_pInfluenceMap->GetNodeAtWorldPos(item.Location) };
	if (node->GetItem() != item.Type)
		return false;

	// Remove item from node
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

void SurvivorAgentMemory::LocateHouse(const HouseInfo& houseInfo)
{
	// Check if house already seen
	const auto houseNode{ m_pInfluenceMap->GetNodeAtWorldPos(houseInfo.Center) };
	if (m_LocatedHouses.find(houseNode->GetIndex()) == m_LocatedHouses.end()) 
	{
		// If not seen save it
		m_LocatedHouses[houseNode->GetIndex()] = houseInfo;
	}
}

// Checks if given house is cleared
// Also updates if house already located
bool SurvivorAgentMemory::IsHouseCleared(const HouseInfo& houseInfo)
{
	const auto houseNodeIdx{ m_pInfluenceMap->GetNodeIdxAtWorldPos(houseInfo.Center) };

	// Check if the house has been marked as cleared before
	if (m_LocatedHouses.count(houseNodeIdx) && m_LocatedHouses[houseNodeIdx].Cleared)
		return true;

	// Get node indices in house area
	const auto area{ GetHouseArea(houseInfo) };

	return IsAreaExplored(area);
}

// Checks if given house is cleared, if it is, pass its area, otherwise pass nothing
// Also updates if house already located
bool SurvivorAgentMemory::IsHouseCleared(const HouseInfo& houseInfo, std::unordered_set<int>& area)
{
	const auto houseNodeIdx{ m_pInfluenceMap->GetNodeIdxAtWorldPos(houseInfo.Center) };

	// Check if the house has been marked as cleared before
	if (m_LocatedHouses.count(houseNodeIdx) && m_LocatedHouses[houseNodeIdx].Cleared)
	{
		// If house has been cleared we don't need it's area
		area = {};
		return true;
	}
	const auto houseArea{ GetHouseArea(houseInfo) };

	// Pass house area
	area = houseArea;

	bool isExplored{ IsAreaExplored(area) };
	m_LocatedHouses[houseNodeIdx].Cleared = isExplored;

	return isExplored;
}

// Checks if given house is cleared, if it is, pass its unscanned area, otherwise pass nothing
// Also updates if house already located
bool SurvivorAgentMemory::IsHouseCleared(std::unordered_set<int>& unscannedArea, const HouseInfo& houseInfo)
{
	const auto houseNodeIdx{ m_pInfluenceMap->GetNodeIdxAtWorldPos(houseInfo.Center) };

	// Check if the house has been marked as cleared before
	if (m_LocatedHouses.count(houseNodeIdx) && m_LocatedHouses[houseNodeIdx].Cleared)
	{
		// If house has been cleared we don't need it's area
		unscannedArea = {};
		return true;
	}

	// Check if area is explored and update cleared status
	m_LocatedHouses[houseNodeIdx].Cleared 
		= IsAreaExplored(GetHouseArea(houseInfo), unscannedArea);


	return IsAreaExplored(m_pInfluenceMap->GetNodeIndicesInRect(houseInfo.Center, houseInfo.Size), unscannedArea);
}


// Locate houses and update their cleared status
void SurvivorAgentMemory::UpdateHouses(float deltaTime, IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV)
{
	// Locate all houses in sight
	for (const auto& house : housesInFOV)
	{
		LocateHouse(*house);
	}

	for (auto& house : m_LocatedHouses)
	{
		// Reset areas to unexplored after a certain time so the agent continues going house to house
		if (house.second.UpdateResetTime(deltaTime))
		{
			ForgetArea(GetHouseArea(house.second));
		}

		// Save cleared status
		if (IsHouseCleared(house.second))
		{
			house.second.Cleared = true;
		}
	}
}

// Locate item
void SurvivorAgentMemory::LocateItem(const ItemInfo& item)
{
	auto node{ m_pInfluenceMap->GetNodeAtWorldPos(item.Location) };
	node->SetItem(item);
	m_LocatedItems.insert(node->GetIndex());
}

void SurvivorAgentMemory::UpdateEntities(IExamInterface* pInterface, std::vector<EntityInfo*> entitiesInFOV) 
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();
	const Elite::Vector2 scanPos{ eAgentInfo.Location + (eAgentInfo.GetForward() * eAgentInfo.FOV_Range / 2.0f) };
	const float scanRadius{ eAgentInfo.FOV_Range / 2.0f };
	const auto& indices{ m_pInfluenceMap->GetNodeIndicesInRadius(scanPos, scanRadius) };


	// Mark cell that agent finds himself in as seen
	m_pInfluenceMap->SetScannedAtPosition(m_pInfluenceMap->GetNodeAtWorldPos(eAgentInfo.Location)->GetIndex(), true);
	// Mark the cells in his FOV as seen
	m_pInfluenceMap->SetScannedAtPosition(indices, true);

	for (const auto& e : entitiesInFOV)
	{
		// Locate items in sight
		if (e->Type == eEntityType::ITEM)
		{
			ItemInfo info{  };
			if (!pInterface->Item_GetInfo(*e, info))
				return;

			LocateItem(info);
		}

		// Watch for danger from purge zones
		if (e->Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo purgeZone{};
			if (pInterface->PurgeZone_GetInfo(*e, purgeZone))
			{

				// If FOV overlaps with purgezone
				if (Elite::IsCirclesOverlapping(scanPos, purgeZone.Center, scanRadius, purgeZone.Radius))
				{
					m_pInfluenceMap->SetInfluenceAtPosition(scanPos, -50); // Set Danger
				}
			}
		}
	}

	if (eAgentInfo.WasBitten) m_pInfluenceMap->SetInfluenceAtPosition(eAgentInfo.Location, -100);
}

bool SurvivorAgentMemory::IsAreaExplored(std::unordered_set<int> area) const
{
	int nrCellsCleared{ 0 };
	for (int i : area)
	{
		if (m_pInfluenceMap->GetNode(i)->GetScanned())
			++nrCellsCleared;
	}

	return static_cast<float>(nrCellsCleared) / static_cast<float>(area.size()) >= m_PercentageToClear;
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

	return static_cast<float>(nrCellsCleared) / static_cast<float>(area.size()) >= m_PercentageToClear;
}


//====== DEBUG =====\\

void SurvivorAgentMemory::DebugRender(IExamInterface* pInterface) const
{
	pInterface->Draw_Circle(pInterface->Agent_GetInfo().Location, m_PropagationRadius, { 0,1,0 });
	RenderInfluenceMap(pInterface);
}

void SurvivorAgentMemory::RenderInfluenceMap(IExamInterface* pInterface) const
{
	m_pInfluenceMap->SetNodeColorsBasedOnInfluence();

	for (const auto& house : m_LocatedHouses)
	{
		Elite::Vector3 color{};
		house.second.Cleared ? color = { 0,1,0 } : color = { 1,0,0 };
		pInterface->Draw_Circle(house.second.Center, min(house.second.Size.x, house.second.Size.y), color);
	}
	auto visibleNodes{ m_pInfluenceMap->GetNodeIndicesInRadius(pInterface->Agent_GetInfo().Location, m_PropagationRadius) };
	m_pGraphRenderer->RenderNodes(m_pInfluenceMap, pInterface, visibleNodes, true, false, false, false);
}