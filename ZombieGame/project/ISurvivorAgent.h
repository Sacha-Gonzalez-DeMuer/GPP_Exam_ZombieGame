#pragma once
#include "ExtendedStructs.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "SurvivorAgentMemory.h"
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

class ISurvivorAgent
{
public:
	ISurvivorAgent(IExamInterface* pInterface);
	ISurvivorAgent(const ISurvivorAgent* other) = delete;
	ISurvivorAgent(ISurvivorAgent&& other) = delete;
	ISurvivorAgent& operator=(const ISurvivorAgent& other);
	ISurvivorAgent& operator=(ISurvivorAgent&& other);
	~ISurvivorAgent();

	void Initialize(IExamInterface* pInterface);
	void Update(float deltaTime, IExamInterface* pInterface, SteeringPlugin_Output& steering);
	void Render(float deltaTime, IExamInterface* pInterface);

	void SetSteeringBehavior(ISteeringBehavior* pBehavior) {}
	void SetToWander(bool autoOrient, bool sprint);   
	void SetToSeek(const Elite::Vector2& target);
	void SetTarget(const Elite::Vector2& target);
	void SetToLookAt();
	void SetToLookAt(const Elite::Vector2& target);
	void SetToLookAround();
	void SetToFlee();
	void SetToFleeLookingAt();
	void SetToFleeLookingAt(const Elite::Vector2& target);
	void SetToExplore();
	bool SetToFleeLookingAround();
	void SetToFleeLookingAround(const Elite::Vector2& target);

	void SetSurvivorState(SurvivorState toState) { m_SurvivorState = toState; };

	std::shared_ptr<Elite::Vector2> GetTarget() const { return m_Target; };


	std::shared_ptr<ISteeringBehavior> GetCurrentSteering() const { return m_pCurrentSteering; };

	EAgentInfo GetInfo() const;
	UINT GetInventorySlot() const { return m_InventorySlot; };

protected:
	std::vector<EntityInfo*> m_pEntitiesInFOV{};
	std::vector<HouseInfo*> m_pHousesInFOV{};

	HouseInfo* GetNearestHouse(const Elite::Vector2& fromPos) const;

private:
	//Data
	IExamInterface* m_pInterface{ nullptr };
	void UpdateObjectsInFOV(IExamInterface* pInterface);

	//DecisionMaking
	SurvivorState m_SurvivorState{ SurvivorState::EXPLORING };
	Elite::IDecisionMaking* m_pDecisionMaking{ nullptr };
	Elite::Blackboard* m_pBlackboard{ nullptr };
	void InitializeBehaviorTree(IExamInterface* pInterface);
	Elite::Blackboard* CreateBlackboard(IExamInterface* pAgentInfo);

	//Steering
	bool m_ForceSteering{ false };
	SteeringPlugin_Output m_ForcedSteering{};
	std::shared_ptr<ISteeringBehavior> m_pCurrentSteering{ nullptr };
	std::shared_ptr<ISteeringBehavior> m_pWander;
	std::shared_ptr<ISteeringBehavior> m_pSeek;
	std::shared_ptr<ISteeringBehavior> m_pLookAround;
	std::shared_ptr<ISteeringBehavior> m_pLookAt;
	std::shared_ptr<ISteeringBehavior> m_pFlee;
	std::shared_ptr<ISteeringBehavior> m_pFleeLookingAt;
	std::shared_ptr<ISteeringBehavior> m_pFleeLookingAround;
	std::shared_ptr<ISteeringBehavior> m_pExplore;

	void InitializeSteering();

	//Memory
	SurvivorAgentMemory* m_pMemory;

	//Inventory
	Inventory* m_pInventory;

	std::shared_ptr<Elite::Vector2> m_Target{};
	UINT m_InventorySlot = 0;
};

