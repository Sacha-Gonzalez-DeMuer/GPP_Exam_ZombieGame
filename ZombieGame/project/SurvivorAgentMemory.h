#pragma once
#include <unordered_set>
#include "ExtendedStructs.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"

class IExamInterface;

class SurvivorAgentMemory final
{
	using InfluenceGrid = Elite::GridGraph<Elite::WorldNode, Elite::GraphConnection>;

public:
	SurvivorAgentMemory(IExamInterface* pInterface);
	~SurvivorAgentMemory();
	void Update(float deltaTime, IExamInterface* pInterface, std::vector<EntityInfo*> seenEntities, std::vector<HouseInfo*> seenHouses);


	void DebugRender(IExamInterface* pInterface) const;
	void RenderInfluenceMap(IExamInterface* pInterface) const;

	bool HasSeenItems() const { return !m_LocatedItems.empty(); };
	bool HasVisitedHouse(const HouseInfo& house) const;

	Elite::InfluenceMap<InfluenceGrid>* GetInfluenceMap() const { return m_pInfluenceMap; };
	std::unordered_set<int> GetLocatedItems() const { return m_LocatedItems; };
	std::vector<HouseInfo> GetSeenHouses() const { return m_SeenHouses; };
	std::vector<HouseInfo> GetClearedHouses() const { return m_ClearedHouses; };

	bool OnPickUpItem(const ItemInfo& item);
	 
	void AddToSeenHouses( HouseInfo houseInfo);
	void AddToVisitedHouses( HouseInfo houseInfo);
	bool IsHouseVisited(const HouseInfo& houseInfo);
	void UpdateHouses(IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV);

private:
	IExamInterface* m_pInterface;

	//InfluenceMap
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceMap{ nullptr };
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };
	float m_PropagationRadius;

	float m_CooldownTime{ 1.f };
	float m_CooldownTimer{ 0 };

	std::unordered_set<int> m_LocatedItems{};

	std::vector<HouseInfo> m_SeenHouses{};
	std::vector<HouseInfo> m_ClearedHouses{};

	void LocateItem(const ItemInfo& item);
	void UpdateInfluenceMap(float deltaTime, IExamInterface* pInterface);
	void UpdateSeenCells(IExamInterface* pInterface, std::vector<EntityInfo*> entitiesInFOV);
};

