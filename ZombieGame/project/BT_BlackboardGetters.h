#include "Behaviors.h"
#include "ISurvivorAgent.h"

namespace BT_Functions
{
#define FAILURE Elite::BehaviorState::Failure
#define SUCCESS Elite::BehaviorState::Success
#define RUNNING Elite::BehaviorState::Running


	IExamInterface* GetInterface(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
			return nullptr;

		return pInterface;
	}

	ISurvivorAgent* GetSurvivor(Elite::Blackboard* pBlackboard)
	{
		ISurvivorAgent* pSurvivor{};
		if (!pBlackboard->GetData("Survivor", pSurvivor))
			return nullptr;

		return pSurvivor;
	}

	Inventory* GetInventory(Elite::Blackboard* pBlackboard)
	{
		Inventory* pInventory{};
		if (!pBlackboard->GetData("Inventory", pInventory))
			return nullptr;

		return pInventory;
	}

	bool GetTarget(Elite::Blackboard* pBlackboard, std::shared_ptr<Elite::Vector2>& t)
	{
		std::shared_ptr<Elite::Vector2> target{};
		if (!pBlackboard->GetData("Target", target) || target == nullptr)
			return false;

		t = target;
		return true;
	}

	Elite::InfluenceMap<InfluenceGrid>* GetInfluenceMap(Elite::Blackboard* pBlackboard)
	{
		Elite::InfluenceMap<InfluenceGrid>* influenceMap{};
		if (!pBlackboard->GetData("InfluenceMap", influenceMap))
			return nullptr;

		return influenceMap;
	}

	std::shared_ptr<SurvivorAgentMemory> GetMemory(Elite::Blackboard* pBlackboard)
	{
		std::shared_ptr<SurvivorAgentMemory> pMemory{};
		if (!pBlackboard->GetData("Memory", pMemory))
			return nullptr;

		return pMemory;
	}

	SurvivorState* GetSurvivorState(Elite::Blackboard* pBlackboard)
	{
		SurvivorState* pState{};
		if (!pBlackboard->GetData("State", pState))
			return nullptr;

		return pState;
	}

	std::vector<EntityInfo*> GetEntitiesInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return {};

		return pSurvivor->GetEntitiesInFOV();
	}

	std::vector<HouseInfo*> GetHousesInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return {};

		return pSurvivor->GetHousesInFOV();
	}
}