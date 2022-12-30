#pragma once
#include "Exam_HelperStructs.h"

class IExamInterface;
class ISteeringBehavior;
class Inventory;
class InfluenceMap;

class ISurvivorAgent
{
public:
	ISurvivorAgent(IExamInterface* pInterface, InfluenceMap* pInfluenceMap);
	ISurvivorAgent(const ISurvivorAgent* other) = delete;
	ISurvivorAgent(ISurvivorAgent&& other) = delete;
	ISurvivorAgent& operator=(const ISurvivorAgent& other);
	ISurvivorAgent& operator=(ISurvivorAgent&& other);
	~ISurvivorAgent();

	void Initialize(IExamInterface* pInterface);
	void Update(float deltaTime, IExamInterface* pInterface, SteeringPlugin_Output& steering);

	//Steering methods
	void SetSteeringBehavior(ISteeringBehavior* pBehavior) {}
	void SetToWander();   
	void SetToSeek(Elite::Vector2 target);
	void SetLookAtTarget(std::shared_ptr<Elite::Vector2> target);
	void SetToLookAt();
	void SetToLookAround();
	void SetToFlee();

	std::shared_ptr<ISteeringBehavior> GetCurrentSteering() const { return m_pCurrentSteering; };

	UINT GetInventorySlot() const { return m_InventorySlot; };
private:
	//Data
	//IExamInterface* m_pInterface{ nullptr };
	std::vector<EntityInfo*> m_pEntitiesInFOV{};
	std::vector<HouseInfo*> m_pHousesInFOV{};
	void UpdateObjectsInFOV(IExamInterface* pInterface);

	//DecisionMaking
	Elite::IDecisionMaking* m_pDecisionMaking{ nullptr };
	Elite::Blackboard* m_pBlackboard{ nullptr };
	void InitializeBehaviorTree(IExamInterface* pInterface);
	Elite::Blackboard* CreateBlackboard(IExamInterface* pAgentInfo);

	//Steering
	std::shared_ptr<ISteeringBehavior> m_pCurrentSteering{ nullptr };
	std::shared_ptr<ISteeringBehavior> m_pWander;
	std::shared_ptr<ISteeringBehavior> m_pSeek;
	std::shared_ptr<ISteeringBehavior> m_pLookAround;
	std::shared_ptr<ISteeringBehavior> m_pLookAt;
	std::shared_ptr<ISteeringBehavior> m_pFlee;
	void InitializeSteering();

	//Inventory
	Inventory* m_pInventory;

	//InfluenceMap
	InfluenceMap* m_pInfluenceMap;

	std::shared_ptr<Elite::Vector2> m_Target{};
	UINT m_InventorySlot = 0;
};

