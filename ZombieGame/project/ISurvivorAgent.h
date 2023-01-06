#pragma once
#include "ExtendedStructs.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "SurvivorAgentMemory.h"
#include "framework/Agent/SteeringAgent.h"
#include <set>

class PrioritySteering;
class IExamInterface;
class ISteeringBehavior;
class Inventory;
using InfluenceGrid = Elite::GridGraph<Elite::WorldNode, Elite::GraphConnection>;

enum class SurvivorState
{
	EXPLORING,
	LOOTING,
	DEFENSIVE,
	AGGRO,
};

class ISurvivorAgent : public SteeringAgent
{
public:
	ISurvivorAgent(IExamInterface* pInterface);
	ISurvivorAgent(const ISurvivorAgent* other) = delete;
	ISurvivorAgent(ISurvivorAgent&& other) = delete;
	ISurvivorAgent& operator=(const ISurvivorAgent& other) = delete;
	ISurvivorAgent& operator=(ISurvivorAgent&& other) = delete;
	~ISurvivorAgent();

	void Initialize(IExamInterface* pInterface);
	void Update(float deltaTime, IExamInterface* pInterface, SteeringPlugin_Output& steering);
	void Render(float deltaTime, IExamInterface* pInterface);

	std::shared_ptr<ISteeringBehavior> GetCurrentSteering() const { return m_pCurrentSteering; };
	std::vector<EntityInfo*> GetEntitiesInFOV() const { return m_pEntitiesInFOV; };
	std::vector<HouseInfo*> GetHousesInFOV() const { return m_pHousesInFOV; };
	bool IsInFOV(const EntityInfo& e) const;
	
protected:
	std::vector<EntityInfo*> m_pEntitiesInFOV{};
	std::vector<HouseInfo*> m_pHousesInFOV{};

	HouseInfo* GetNearestHouse(const Elite::Vector2& fromPos) const;

private:
	//Data
	IExamInterface* m_pInterface{ nullptr };
	void UpdateObjectsInFOV(IExamInterface* pInterface);

	//DecisionMaking
	Elite::IDecisionMaking* m_pDecisionMaking{ nullptr };
	Elite::Blackboard* m_pBlackboard{ nullptr };
	void InitializeBehaviorTree(IExamInterface* pInterface);
	Elite::Blackboard* CreateBlackboard(IExamInterface* pAgentInfo);

	//Memory
	std::shared_ptr<SurvivorAgentMemory> m_pMemory;

	//Inventory
	Inventory* m_pInventory;
};

