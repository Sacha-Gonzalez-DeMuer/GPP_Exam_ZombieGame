#pragma once
#include "Behaviors.h"

namespace BT_Conditions
{
	using namespace BT_Functions;

	bool IsEntityInFOV(Elite::Blackboard* pBlackboard)
	{
		return !GetEntitiesInFOV(pBlackboard).empty();
	}

	bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pEntities{ GetEntitiesInFOV(pBlackboard) };
		if (pEntities.empty())
			return false;

		for (auto pEntity : pEntities)
		{
			if (pEntity->Type == eEntityType::ENEMY)
			{
				return true;
			}
		}

		return false;
	}

	bool IsItemInFOV(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };

		if (entitiesInFOV.empty())
			return false;

		for (const auto& entity : entitiesInFOV)
		{
			if (entity->Type == eEntityType::ITEM)
				return true;
		}

		return false;
	}


	bool IsInRangeOfItem(Elite::Blackboard* pBlackboard)
	{
		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };
		if (entitiesInFOV.empty())
			return false;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		auto agentInfo{ pInterface->Agent_GetInfo() };

		for (auto entity : entitiesInFOV)
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

		return pInventory->HasItem(eItemType::SHOTGUN) || pInventory->HasItem(eItemType::PISTOL);
	}

	bool HasTarget(Elite::Blackboard* pBlackboard)
	{
		std::shared_ptr<Elite::Vector2> t;
		return GetTarget(pBlackboard, t);
	}

	bool TAlignedWithTarget(Elite::Blackboard* pBlackboard, const Elite::Vector2& target)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return false;

		EAgentInfo eAgentInfo{ pSurvivor->GetInfo() };
		const Elite::Vector2 agentForward{ eAgentInfo.GetForward() };
		const Elite::Vector2 toTarget{ (target - eAgentInfo.Location).GetNormalized() };

		const float angleBetween
		{
			Elite::ToDegrees(
				Elite::AngleBetween(agentForward, toTarget))
		};

		constexpr float errorMargin{ 5.f };

		return angleBetween < errorMargin;
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

		const float angleBetween{ Elite::ToDegrees(Elite::AngleBetween(agentForward, toTarget)) };
		constexpr float errorMargin{ 15.f };

		return angleBetween < errorMargin;
	}

	bool IsHouseInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pHouses{ GetHousesInFOV(pBlackboard) };

		return !pHouses.empty();
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

		if (scannedCount < pInfluenceMap->GetConnections(node->GetIndex()).size() / 2)
			std::cout << "Area not scanned\n";

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

		//Get nodes around fov radius
		const auto& nodes = pInfluenceMap->GetNodeIndicesInRadius(pInterface->Agent_GetInfo().Location, pInterface->Agent_GetInfo().FOV_Range);

		const float errorMargin{ 5.0f };
		for (const auto& node : nodes)
		{
			if (pInfluenceMap->GetNode(node)->GetInfluence() < -errorMargin)
			{
				return true;
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

		const float errorMargin{ 5.0f };
		for (const auto& node : nodes)
		{
			for (const auto& connection : pInfluenceMap->GetNodeConnections(node))
			{
				if (pInfluenceMap->GetNode(connection->GetTo())->GetInfluence() > errorMargin)
				{
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
		const auto& pState{ GetSurvivorState(pBlackboard) };
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

		return !pMemory->GetLocatedHouses().empty();
	}

	bool IsHouseCleared(Elite::Blackboard* pBlackboard, HouseInfo* house)
	{
		const auto& pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return false;


		return pMemory->IsHouseCleared(*house);
	}

	bool IsHealthLow(Elite::Blackboard* pBlackboard)
	{
		const auto& pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		return pInterface->Agent_GetInfo().Health < 8.0f;
	}

	bool IsEnergyLow(Elite::Blackboard* pBlackboard)
	{
		const auto& pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return false;

		return pInterface->Agent_GetInfo().Energy < 8.0f;
	}

	bool HasGarbage(Elite::Blackboard* pBlackboard)
	{
		const auto& pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return false;

		return pInventory->HasItem(eItemType::GARBAGE);
	}

	bool NeedsItem(Elite::Blackboard* pBlackboard)
	{
		const auto& pSurvivor{ GetSurvivor(pBlackboard) };

		if (pSurvivor->GetInfo().Energy < pSurvivor->GetInfo().LowEnergyThreshold
			|| pSurvivor->GetInfo().Health < pSurvivor->GetInfo().LowHealthThreshold)
			return true;

		return false;
	}

	bool HasEmptyItem(Elite::Blackboard* pBlackboard)
	{
		const auto& pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return false;

		return pInventory->HasEmptyItem();
	}

	bool IsInHouse(Elite::Blackboard* pBlackboard)
	{
		const auto& pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return false;

		const auto& housesInFOV{ pSurvivor->GetHousesInFOV() };
		if (housesInFOV.empty())
			return false;

		for (const auto& house : housesInFOV)
		{
			if (IsPointInRect(pSurvivor->GetInfo().Location, house->Center, house->Size * .8f))
				return true;
		}

		return false;
	}
}
