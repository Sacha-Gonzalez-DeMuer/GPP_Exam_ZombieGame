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
#include "framework/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
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
		if (!pBlackboard->GetData("Target", t))
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

		pSurvivor->SetToWander();

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
			if (ei->Location.DistanceSquared(pInterface->Agent_GetInfo().Position) < closestEntity->Location.Distance(pInterface->Agent_GetInfo().Position))
			{
				closestEntity = ei;
			}
		}
		
		pSurvivor->SetToSeek(closestEntity->Location);

		return SUCCESS; 
	}

	Elite::BehaviorState GoToClosestKnownItem(Elite::Blackboard* pBlackboard)
	{
		std::cout << "going to closest Item\n";
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return FAILURE;

		IExamInterface* pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return FAILURE;

		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto& items{ pMemory->GetSeenItems() };
		Elite::Vector2 closestItem{ FLT_MAX, FLT_MAX };

		auto agentInfo{ pInterface->Agent_GetInfo() };

		for (auto& item : items)
		{
			if (agentInfo.Position.DistanceSquared(item) < agentInfo.Position.DistanceSquared(closestItem))
				closestItem = item;
		}

		if (closestItem.x == FLT_MAX || closestItem.y == FLT_MAX)
			return FAILURE;

		pSurvivor->SetToSeek(closestItem);
		if (agentInfo.Position.DistanceSquared(closestItem) > agentInfo.GrabRange * agentInfo.GrabRange)
			return RUNNING;

		return SUCCESS;
	}

	Elite::BehaviorState GoToClosestHouse(Elite::Blackboard* pBlackboard)
	{
		auto housesInFOV{ GetHousesInFOV(pBlackboard) };
		if (!housesInFOV)
			return FAILURE;

		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return FAILURE;

		HouseInfo* closestHouse{ housesInFOV->front() };
		for (HouseInfo* hi : *housesInFOV)
		{
			if (hi->Center.DistanceSquared(pInterface->Agent_GetInfo().Position) < closestHouse->Center.DistanceSquared(pInterface->Agent_GetInfo().Position))
			{
				closestHouse = hi;
			}
		}

		pSurvivor->SetToSeek(closestHouse->Center);
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

		ItemInfo item;
		for (const auto& entity : *pEntities)
		{
			if (pInventory->GrabItem(*entity))
			{
				auto pMemory{ GetMemory(pBlackboard) };
				pMemory->OnPickUpItem(entity);
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

	Elite::BehaviorState SetClosestEnemyAsTarget(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };
		if (!entitiesInFOV || entitiesInFOV->empty())
			return FAILURE;

		IExamInterface* pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return FAILURE;

		EntityInfo* closestEntity{ entitiesInFOV->front() };
		for (EntityInfo* ei : *entitiesInFOV)
		{
			if (ei->Type == eEntityType::ENEMY && ei->Location.DistanceSquared(pInterface->Agent_GetInfo().Position) < closestEntity->Location.Distance(pInterface->Agent_GetInfo().Position))
			{
				closestEntity = ei;
			}
		}

		std::shared_ptr<Elite::Vector2> target{};
		if (!GetTarget(pBlackboard, target))
			return FAILURE;

		target = std::make_shared<Elite::Vector2>(closestEntity->Location);

		return SUCCESS;
	}

	Elite::BehaviorState TurnAround(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return FAILURE;


		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };
		EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();
		const Elite::Vector2 agentForward{ eAgentInfo.GetForward() };

		pSurvivor->SetToLookAround();

		const float angleBetween{ Elite::ToDegrees(atan2(agentForward.Dot(-agentForward), agentForward.Cross(-agentForward))) };
		constexpr float errorMargin{ 2.f };
		
	}

	Elite::BehaviorState PrintTest(Elite::Blackboard* pBlackboard)
	{
		std::cout << "TESTING BEHAVIOR\n";
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
				return true;
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
			if (entity->Location.DistanceSquared(agentInfo.Position) < agentInfo.GrabRange * agentInfo.GrabRange)
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
		std::shared_ptr<Elite::Vector2> t;
		if(!GetTarget(pBlackboard, t))
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;
		
		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };
		EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();
		const Elite::Vector2 agentForward{ eAgentInfo.GetForward() };
		const Elite::Vector2 toTarget{ (*t - agentPos).GetNormalized() };

		const float angleBetween{ Elite::ToDegrees(atan2(agentForward.Dot(toTarget), agentForward.Cross(toTarget)) ) };
		constexpr float errorMargin{ 2.f };
		std::cout << "AngleBetween: " << angleBetween << "\n";
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

	bool AreaScanned(Elite::Blackboard* pBlackboard)
	{
		auto pInfluenceMap{ GetInfluenceMap(pBlackboard) };
		if (!pInfluenceMap)
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		//check influence on neighboring squares 
		const auto& node = pInfluenceMap->GetNodeAtWorldPos(pInterface->Agent_GetInfo().Position);

		float scannedCount{ 0 };

		for (const auto& connection : pInfluenceMap->GetConnections(node->GetIndex()))
		{
			if (pInfluenceMap->GetNode(connection->GetTo())->GetScanned())
				++scannedCount;
		}

		return scannedCount > pInfluenceMap->GetConnections(node->GetIndex()).size() / 1.7f;
	}

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

		return pInterface->Agent_GetInfo().WasBitten;
	}
}

#endif