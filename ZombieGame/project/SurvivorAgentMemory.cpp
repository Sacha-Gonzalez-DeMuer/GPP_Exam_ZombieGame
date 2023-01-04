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

	if (m_CooldownTimer > m_CooldownTime)
	{
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
	for (const auto& item : m_LocatedItems)
	{
		pInterface->Draw_Circle(m_pInfluenceMap->GetNode(item)->GetPosition(), 3, {1,.5f,.5f});
	}
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

void SurvivorAgentMemory::AddToSeenHouses(HouseInfo houseInfo) 
{
	const auto houseNode{ m_pInfluenceMap->GetNodeAtWorldPos(houseInfo.Center) };
	for (auto i = 0; i != m_SeenHouses.size(); i++)
	{
		if (m_pInfluenceMap->GetNodeAtWorldPos(m_SeenHouses[i].Center)->GetIndex() == houseNode->GetIndex())
			return;
	}

	m_SeenHouses.emplace_back(houseInfo);
}

void SurvivorAgentMemory::AddToVisitedHouses(HouseInfo houseInfo)
{
	for (auto i = 0; i != m_ClearedHouses.size(); i++)
	{
		if (m_ClearedHouses[i].Center == houseInfo.Center)
			return;
	}

	m_ClearedHouses.emplace_back(houseInfo);
}

bool SurvivorAgentMemory::IsHouseVisited(const HouseInfo& houseInfo)
{
	if (m_ClearedHouses.empty())
		return false;

	for (const auto& house : m_ClearedHouses)
	{
		if (houseInfo.Center.DistanceSquared(house.Center) < m_pInfluenceMap->GetCellSize())
		{
			return true;
		}
	}

	return false;
}

void SurvivorAgentMemory::UpdateHouses(IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV)
{
	auto agentPos{ pInterface->Agent_GetInfo().Location };
	auto GetNodeIdxAtPos
	{ [&](Elite::Vector2 pos)->int {return m_pInfluenceMap->GetNodeAtWorldPos(pos)->GetIndex(); } };

	for (const auto& house : housesInFOV)
	{
		Elite::Vector2 houseSize{ house->Size.x - m_pInfluenceMap->GetCellSize(), house->Size.y - m_pInfluenceMap->GetCellSize() };
		auto nodesInHouse{ m_pInfluenceMap->GetNodeIndicesInRect(house->Center, houseSize) };
		std::cout << "nr nodes " << nodesInHouse.size() << "\n";

		bool houseCleared{ true };
		int count{ 0 };
		for (const auto& nodeIdx : nodesInHouse)
		{
			++count;
			if (!m_pInfluenceMap->GetNode(nodeIdx)->GetScanned())
			{
				houseCleared = false;
				break;
			}
		}

		if (houseCleared)
		{
			std::cout << "house is cleared; " << count << " nodes scanned\n";
			AddToVisitedHouses(*house);
		}

		//if(GetNodeIdxAtPos(agentPos) == GetNodeIdxAtPos(house->Center))
		//	AddToVisitedHouses(*house);
	}
}

void SurvivorAgentMemory::LocateItem(const ItemInfo& item)
{
	auto node{ m_pInfluenceMap->GetNodeAtWorldPos(item.Location) };

	node->SetItem(item);
	m_LocatedItems.insert(node->GetIndex());
	//m_pInfluenceMap->SetInfluenceAtPosition(item.Location, 20); //TODO: maybe not needed anymore
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

	bool houseCleared{ true };
	for (const auto& nodeIdx : nodesInHouse)
	{
		if (!m_pInfluenceMap->GetNode(nodeIdx)->GetScanned())
		{
			return false;
		}
	}

	return true;

	//for (const auto& house : m_VisitedHouses)
	//{
	//	if (house.Center == location)
	//		return true;
	//}
	//return false;
}
