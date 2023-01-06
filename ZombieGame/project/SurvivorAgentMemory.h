#pragma once
#include <unordered_set>
#include "ExtendedStructs.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"

class IExamInterface;

struct intBool
{
	int i;
	bool b;
};

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
	std::vector<EHouseInfo> GetSeenHouses() const { return m_SeenHouses; };
	std::vector<EHouseInfo> GetClearedHouses() const { return m_ClearedHouses; };
	std::unordered_set<int> GetHouseArea(const HouseInfo& house);
	bool OnPickUpItem(const ItemInfo& item);
	bool OnPickUpItem(const EntityInfo& entity);
	 
	void AddToSeenHouses( HouseInfo houseInfo);
	void AddToVisitedHouses( HouseInfo houseInfo);
	bool IsHouseCleared(const HouseInfo& houseInfo);
	bool IsHouseCleared(const HouseInfo& houseInfo, std::unordered_set<int>& area);
	bool IsHouseCleared(std::unordered_set<int>& unscannedArea, const HouseInfo& houseInfo);

	void UpdateHouses(IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV);
	bool IsAreaExplored(std::unordered_set<int> area) const;
	bool IsAreaExplored(std::unordered_set<int> area, std::unordered_set<int>& unscannedArea) const;

private:
	IExamInterface* m_pInterface;

	//InfluenceMap
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceMap{ nullptr };
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };
	float m_PropagationRadius;

	std::unordered_set<int> m_LocatedItems{};

	int m_NrSeenHouses{};

	//std::unordered_map<intBool, EHouseInfo> m_LocatedHouses{};

	std::vector<EHouseInfo> m_SeenHouses{};
	std::vector<EHouseInfo> m_ClearedHouses{};
	float m_PercentageToClear{ .5f };

	void LocateItem(const ItemInfo& item);
	void UpdateInfluenceMap(float deltaTime, IExamInterface* pInterface);
	void UpdateSeenCells(IExamInterface* pInterface, std::vector<EntityInfo*> entitiesInFOV);
};

