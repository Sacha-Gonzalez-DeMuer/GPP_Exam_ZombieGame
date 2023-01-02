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

	bool HasSeenItems() const { return !m_SeenItems.empty(); };

	Elite::InfluenceMap<InfluenceGrid>* GetInfluenceMap() const { return m_pInfluenceMap; };
	std::vector<EntityInfo> GetSeenItems() const { return m_SeenItems; };
	std::vector<HouseInfo> GetSeenHouses() const { return m_SeenHouses; };
	std::vector<HouseInfo> GetVisitedHouses() const { return m_VisitedHouses; };

	void AddToSeenItems(const EntityInfo& item);
	void OnPickUpItem(const EntityInfo& item);

	void AddToSeenHouses( HouseInfo houseInfo);
	void AddToVisitedHouses( HouseInfo houseInfo);
	bool IsHouseVisited(const HouseInfo& houseInfo);
	void UpdateHouses(IExamInterface* pInterface, const std::vector<HouseInfo*>& housesInFOV);

private:

	//InfluenceMap
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceMap{ nullptr };
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };
	std::unordered_set<int> GetNodesInPropagationRadius(IExamInterface* pInterface) const;
	float m_PropagationRadius;


	void UpdateSeenItems(IExamInterface* pInterface);
	float m_CooldownTime{ 1.f };
	float m_CooldownTimer{ 0 };


	std::vector<EntityInfo> m_SeenItems{};
	std::vector<EntityInfo> m_SeenItemsBuffer{};


	std::vector<HouseInfo> m_SeenHouses{};
	std::vector<HouseInfo> m_VisitedHouses{};


	void UpdateInfluenceMap(float deltaTime, IExamInterface* pInterface);
	void UpdateSeenCells(IExamInterface* pInterface, std::vector<EntityInfo*> entitiesInFOV);
};

