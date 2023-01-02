/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "EBehaviorTree.h"
#include "framework/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "IExamInterface.h"
#include "framework/EliteMath/EVector2.h"
#include "Inventory.h"
#include "ExtendedStructs.h"
#include "SurvivorAgentMemory.h"
#include <memory>

class IBaseInterface;

//#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------


namespace BT_Functions
{
	IExamInterface* GetInterface(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
			return nullptr;

		return pInterface;
	}


	std::vector<EntityInfo*>* GetEntitiesInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo*>* entitiesInFOV{};
		if (!pBlackboard->GetData("EntitiesInFOV", entitiesInFOV))
			return nullptr;

		return entitiesInFOV;
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

	std::vector<HouseInfo*>* GetHousesInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo*>* housesInFOV{};
		if (!pBlackboard->GetData("HousesInFOV", housesInFOV))
			return nullptr;

		return housesInFOV;
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

	SurvivorAgentMemory* GetMemory(Elite::Blackboard* pBlackboard)
	{
		SurvivorAgentMemory* pMemory{};
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
}



namespace BT_Actions
{
	using namespace BT_Functions;
#define FAILURE Elite::BehaviorState::Failure
#define SUCCESS Elite::BehaviorState::Success
#define RUNNING Elite::BehaviorState::Running

	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;
		std::cout << "Changing to wander\n";
		pSurvivor->SetToWander(true, true);
		return SUCCESS;
	}

	Elite::BehaviorState GoToClosestEntity(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };
		if (!entitiesInFOV || entitiesInFOV->empty()) 
			return FAILURE;

		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		IExamInterface* pInterface{ GetInterface(pBlackboard)};
		if (!pInterface)
			return FAILURE;


		EntityInfo* closestEntity{entitiesInFOV->front()};
		for (EntityInfo* ei : *entitiesInFOV)
		{
			if (ei->Location.DistanceSquared(pInterface->Agent_GetInfo().Location) < closestEntity->Location.Distance(pInterface->Agent_GetInfo().Location))
			{
				closestEntity = ei;
			}
		}
		const auto& agentInfo{ pInterface->Agent_GetInfo() };
		pSurvivor->SetToSeek(closestEntity->Location);
		if (agentInfo.Location.DistanceSquared(closestEntity->Location) > agentInfo.GrabRange * agentInfo.GrabRange)
			return RUNNING;

		return SUCCESS; 
	}



	Elite::BehaviorState GoToClosestHouseInFOV(Elite::Blackboard* pBlackboard)
	{
		const auto housesInFOV{ GetHousesInFOV(pBlackboard) };
		if (!housesInFOV || housesInFOV->empty())
			return FAILURE;

		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		const auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return FAILURE;

		HouseInfo* closestHouse{ housesInFOV->front() };
		for (HouseInfo* hi : *housesInFOV)
		{
			if (hi->Center.DistanceSquared(pInterface->Agent_GetInfo().Location) < closestHouse->Center.DistanceSquared(pInterface->Agent_GetInfo().Location))
			{
				closestHouse = hi;
			}
		}

		const auto agentInfo{ pInterface->Agent_GetInfo() };
		pSurvivor->SetToSeek(pInterface->NavMesh_GetClosestPathPoint(closestHouse->Center));
		if (agentInfo.Location.DistanceSquared(closestHouse->Center) > agentInfo.GrabRange * agentInfo.GrabRange)
		{


			return RUNNING;
		}


		return SUCCESS;
	}

	Elite::BehaviorState GrabItem(Elite::Blackboard* pBlackboard)
	{
		auto pEntities{ GetEntitiesInFOV(pBlackboard) };
		if (!pEntities || pEntities->empty())
			return FAILURE;

		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;
		std::cout << "Grabbing\n";
		ItemInfo item;
		for (const auto& entity : *pEntities)
		{
			if (pInventory->GrabItem(*entity))
			{
				auto pMemory{ GetMemory(pBlackboard) };
				pMemory->OnPickUpItem(*entity);
 				return SUCCESS;
			}
		}

		return FAILURE;
	}

	Elite::BehaviorState UseItem(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		if (!pInventory->UseItem())
			return FAILURE;

		return SUCCESS;
	}


	Elite::BehaviorState ChangeToLookAround(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToLookAround();
		return SUCCESS;
	}

	Elite::BehaviorState SetToLooting(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetSurvivorState(SurvivorState::LOOTING);
		return SUCCESS;
	}

	Elite::BehaviorState SetToExploring(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetSurvivorState(SurvivorState::EXPLORING);
		return SUCCESS;
	}

	Elite::BehaviorState SetToDefensive(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;
		pSurvivor->SetSurvivorState(SurvivorState::DEFENSIVE);
		return SUCCESS;
	}

	Elite::BehaviorState SetToAggro(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetSurvivorState(SurvivorState::AGGRO);
		return SUCCESS;
	}


	Elite::BehaviorState EquipWeapon(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		pInventory->EquipWeapon();
		return SUCCESS;
	}

	Elite::BehaviorState ChangeToLookAt(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToLookAt();
		return SUCCESS;
	}

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor(GetSurvivor(pBlackboard));
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToFlee();
		return SUCCESS;
	}

	Elite::BehaviorState ChangeToFleeLookingAt(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor(GetSurvivor(pBlackboard));
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToFleeLookingAt();
		return SUCCESS;
	}

	Elite::BehaviorState LookAtTarget(Elite::Blackboard* pBlackboard, const Elite::Vector2& target)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToLookAt(target);
		return SUCCESS;
	}

	Elite::BehaviorState SetClosestEnemyAsTarget(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };
		if (!entitiesInFOV || entitiesInFOV->empty())
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE; 

		EntityInfo* closestEntity{ entitiesInFOV->front() };
		for (EntityInfo* ei : *entitiesInFOV)
		{
			if (ei->Type == eEntityType::ENEMY && ei->Location.DistanceSquared(pSurvivor->GetInfo().Location) < closestEntity->Location.Distance(pSurvivor->GetInfo().Location))
			{
				closestEntity = ei;
			}
		}

		if (closestEntity == nullptr) return FAILURE;
		pSurvivor->SetTarget(closestEntity->Location);
		return SUCCESS;
	}

	Elite::BehaviorState GoTo(Elite::Blackboard* pBlackboard, Elite::Vector2 target)
	{
		if (target.x == FLT_MAX)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		pSurvivor->SetToSeek(target);
		if (pSurvivor->GetInfo().Location.DistanceSquared(target) > pSurvivor->GetInfo().GrabRange * pSurvivor->GetInfo().GrabRange)
		{
			return RUNNING;
		}

		return SUCCESS;
	}

	Elite::BehaviorState ChangeToFleeLookingAround(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		if (pSurvivor->SetToFleeLookingAround())
			return SUCCESS;

		return FAILURE;
	}


	Elite::BehaviorState PrintTest(Elite::Blackboard* pBlackboard)
	{
		std::cout << "TESTING BEHAVIOR\n";
		return SUCCESS;
	}

	Elite::Vector2 GetClosestHousePos(Elite::Blackboard* pBlackboard)
	{
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return { FLT_MAX, FLT_MAX };

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return { FLT_MAX, FLT_MAX };

		std::vector<HouseInfo> houses{ pMemory->GetSeenHouses() };
		HouseInfo* closestHouse{ &houses[0] };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (auto& house : houses)
		{
			if (house.Center.Distance(agentPos) < closestHouse->Center.Distance(agentPos))
				closestHouse = &house;
		}

		return closestHouse->Center;
	}
	HouseInfo* GetClosestHouse(Elite::Blackboard* pBlackboard)
	{
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return nullptr;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return nullptr;

		std::vector<HouseInfo> houses{ pMemory->GetSeenHouses() };
		HouseInfo* closestHouse{ &houses[0] };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (auto& house : houses)
		{
			if (house.Center.Distance(agentPos) < closestHouse->Center.Distance(agentPos))
				closestHouse = &house;
		}

		return closestHouse;
	}

	HouseInfo* GetClosestHouseInFOV(Elite::Blackboard* pBlackboard)
	{
		auto housesInFOV{ GetHousesInFOV(pBlackboard) };
		if (!housesInFOV || housesInFOV->empty())
			return nullptr;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return nullptr;

		HouseInfo* closestHouse{ housesInFOV->front() };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (const auto& house : *housesInFOV)
		{
			if (house->Center.Distance(agentPos) < closestHouse->Center.Distance(agentPos))
				closestHouse = house;
		}

		return closestHouse;
	}

	Elite::Vector2 GetClosestKnownItemPos(Elite::Blackboard* pBlackboard)
	{
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return { FLT_MAX, FLT_MAX };


		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return { FLT_MAX, FLT_MAX };

		auto& items{ pMemory->GetSeenItems() };
		if (items.empty()) 	
			return { FLT_MAX, FLT_MAX };


		Elite::Vector2 closestItem{ FLT_MAX, FLT_MAX };

		for (auto& item : items)
		{
			if (pSurvivor->GetInfo().Location.DistanceSquared(item.Location) < pSurvivor->GetInfo().Location.DistanceSquared(closestItem))
				closestItem = item.Location;
		}

		if (closestItem.x != FLT_MAX)
			return closestItem;
	}

	Elite::BehaviorState ChangeToExplore(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		pSurvivor->SetToExplore();
		return SUCCESS;
	}
}

//Conditions
namespace BT_Conditions
{
	using namespace BT_Functions;

	bool IsEntityInFOV(Elite::Blackboard* pBlackboard)
	{
		return !GetEntitiesInFOV(pBlackboard)->empty();
	}

	bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pEntities{ GetEntitiesInFOV(pBlackboard) };
		if (!pEntities || pEntities->empty())
			return false;

		for (auto pEntity : *pEntities)
		{
			if (pEntity->Type == eEntityType::ENEMY)
			{
				std::cout << "EnemyInFOV!\n";
				return true;
			}
		}

		return false;
	}

	bool IsItemInFOV(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };

		if (entitiesInFOV->empty())
			return false;

		for (const auto& entity : *entitiesInFOV)
		{
			if (entity->Type == eEntityType::ITEM)
				return true;
		}

		return false;
	}

	bool IsInRangeOfItem(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };
		if (!entitiesInFOV || entitiesInFOV->empty())
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		auto agentInfo{ pInterface->Agent_GetInfo() };

		for (auto entity : *entitiesInFOV)
		{
			if (entity->Type != eEntityType::ITEM) 
				continue;

			//check if item is in pickup range
			if (entity->Location.DistanceSquared(agentInfo.Location) < agentInfo.GrabRange * agentInfo.GrabRange)
				return true;
		}

		return false;
	}

	bool HasWeapon(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return false;
		
		return pInventory->HasWeapon();
	}

	bool HasTarget(Elite::Blackboard* pBlackboard)
	{
		std::shared_ptr<Elite::Vector2> t;
		return GetTarget(pBlackboard, t);
	}

	bool AlignedWithTarget(Elite::Blackboard* pBlackboard)
	{
		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return false;

		auto t = pSurvivor->GetTarget();

		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();
		const Elite::Vector2 agentForward{ eAgentInfo.GetForward() };
		const Elite::Vector2 toTarget{ (*t - agentPos).GetNormalized() };

		const float angleBetween{ Elite::ToDegrees(acos(agentForward.Dot(toTarget))) };
		constexpr float errorMargin{ 15.f };

		return angleBetween < errorMargin;
	}

	bool IsHouseInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pHouses{GetHousesInFOV(pBlackboard)};
		if (!pHouses)
			return false;

		return !pHouses->empty();
	}

	bool IsLookingAround(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return false;

		//cast current ISteeringBehavior to LookAround to check if that's the current steering behavior
		std::shared_ptr<LookAround> la{ std::dynamic_pointer_cast<LookAround>(pSurvivor->GetCurrentSteering()) };
		return !la;
	}



	//Influence Map Conditions
	bool AreaScanned(Elite::Blackboard* pBlackboard)
	{
		auto pInfluenceMap{ GetInfluenceMap(pBlackboard) };
		if (!pInfluenceMap)
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		//check influence on neighboring squares 
		const auto& node = pInfluenceMap->GetNodeAtWorldPos(pInterface->Agent_GetInfo().Location);
		float scannedCount{ 0 };

		for (const auto& connection : pInfluenceMap->GetConnections(node->GetIndex()))
		{
			if (pInfluenceMap->GetNode(connection->GetTo())->GetScanned())
				++scannedCount;
		}

		return scannedCount > pInfluenceMap->GetConnections(node->GetIndex()).size() / 2;
	}

	bool IsDangerNear(Elite::Blackboard* pBlackboard)
	{
		auto pInfluenceMap{ GetInfluenceMap(pBlackboard) };
		if (!pInfluenceMap)
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		//check influence on neighboring squares 
		const auto& nodes = pInfluenceMap->GetNodeIndicesInRadius(pInterface->Agent_GetInfo().Location, pInterface->Agent_GetInfo().FOV_Range * 2);

		float scannedCount{ 0 };

		for (const auto& node : nodes)
		{
			for (const auto& connection : pInfluenceMap->GetNodeConnections(node))
			{
				if (pInfluenceMap->GetNode(connection->GetTo())->GetInfluence() < -10)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IsRewardNear(Elite::Blackboard* pBlackboard)
	{
		auto pInfluenceMap{ GetInfluenceMap(pBlackboard) };
		if (!pInfluenceMap)
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		//check influence on neighboring squares 
		const auto& nodes = pInfluenceMap->GetNodeIndicesInRadius(pInterface->Agent_GetInfo().Location, pInterface->Agent_GetInfo().FOV_Range * 2);

		float scannedCount{ 0 };

		for (const auto& node : nodes)
		{
			for (const auto& connection : pInfluenceMap->GetNodeConnections(node))
			{
				if (pInfluenceMap->GetNode(connection->GetTo())->GetInfluence() > 10)
				{
					std::cout << "Reward is near\n";
					return true;
				}
			}
		}

		return false;
	}


	//Finite State Conditions
	bool HasSeenItem(Elite::Blackboard* pBlackboard)
	{
		const auto& pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return false;

		return pMemory->HasSeenItems();
	}

	bool WasBitten(Elite::Blackboard* pBlackboard)
	{
		const auto& pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		if (pInterface->Agent_GetInfo().WasBitten)
			std::cout << "Was bitten\n";

		return pInterface->Agent_GetInfo().WasBitten;
	}

	bool IsDefensiveState(Elite::Blackboard* pBlackboard)
	{
		const auto& pState { GetSurvivorState(pBlackboard) };
		if (!pState)
			return false;

		return *pState == SurvivorState::DEFENSIVE;
	}

	bool IsLootingState(Elite::Blackboard* pBlackboard)
	{
		const auto& pState{ GetSurvivorState(pBlackboard) };
		if (!pState)
			return false;

		return *pState == SurvivorState::LOOTING;
	}

	bool IsExploringState(Elite::Blackboard* pBlackboard)
	{
		const auto& pState{ GetSurvivorState(pBlackboard) };
		if (!pState)
			return false;

		return *pState == SurvivorState::EXPLORING;
	}

	bool IsAggroState(Elite::Blackboard* pBlackboard)
	{
		const auto& pState{ GetSurvivorState(pBlackboard) };
		if (!pState)
			return false;

		return *pState == SurvivorState::AGGRO;
	}

	bool IsInventoryFull(Elite::Blackboard* pBlackboard)
	{
		const auto& pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return false;

		return pInventory->IsFull();
	}

	bool HasSeenHouse(Elite::Blackboard* pBlackboard)
	{
		const auto& pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return false;

		if (!pMemory->GetSeenHouses().empty())
		{
			std::cout << "has seen house\n";
				return true;
		}
		return !pMemory->GetSeenHouses().empty();
	}

	bool IsHouseVisited(Elite::Blackboard* pBlackboard, HouseInfo* house)
	{
		const auto& pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return false;
		

		return pMemory->IsHouseVisited(*house);
	}

	bool HasVisitedAllSeenHouses(Elite::Blackboard* pBlackboard)
	{
		const auto& pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return false;

		return pMemory->GetVisitedHouses().size() <= pMemory->GetSeenHouses().size() ;
	}

}

#endif