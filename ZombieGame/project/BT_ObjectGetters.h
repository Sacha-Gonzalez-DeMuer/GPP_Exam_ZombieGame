#pragma once
#include "Behaviors.h"

namespace BT_ObjectGetters
{
	using namespace BT_Functions;

	Elite::Vector2 GetClosestHousePos(Elite::Blackboard* pBlackboard)
	{
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return INVALID_VECTOR2;

		auto locatedHouses{ pMemory->GetLocatedHouses() };

		if (locatedHouses.empty())
			return INVALID_VECTOR2;

		Elite::Vector2 closestPos{ FLT_MAX, FLT_MAX };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (auto& house : locatedHouses)
		{
			if (house.second.Center.Distance(agentPos) < closestPos.Distance(agentPos))
				closestPos = house.second.Center;
		}

		return closestPos;
	}


	eItemType GetNeededItemType(Elite::Blackboard* pBlackboard)
	{
		// Get necessary data
		const auto& pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return eItemType::INVALID;
		const auto& pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return eItemType::INVALID;

		// Determine needed item (top to bottom priority)
		if (!pInventory->HasWeapon())
			return eItemType::WEAPON;

		if (pSurvivor->GetInfo().Energy < pSurvivor->GetInfo().LowEnergyThreshold)
			return eItemType::FOOD;

		if (pSurvivor->GetInfo().Health < pSurvivor->GetInfo().LowHealthThreshold)
			return eItemType::MEDKIT;

		return eItemType::INVALID;
	}

	std::unordered_set<int> GetUnclearedHouseArea(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return {};
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return {};

		auto locatedHouses{ pMemory->GetLocatedHouses() };
		if (locatedHouses.empty())
			return {};



		//Determine unvisited closest house
		std::unordered_set<int> area{ };
		bool closeUnexploredFound{ false };
		std::pair<int, EHouseInfo> houseInfo{};
		houseInfo.second.Center = { FLT_MAX, FLT_MAX };

		for (auto& house : locatedHouses)
		{
			if (!pMemory->IsHouseCleared(house.second) 
				&& house.second.Center.DistanceSquared(pSurvivor->GetLocation()) < houseInfo.second.Center.DistanceSquared(pSurvivor->GetLocation()))
			{
				houseInfo.first = houseInfo.first;
				houseInfo.second = house.second;
				closeUnexploredFound = true;
			}
		}

		if (closeUnexploredFound)
			return pMemory->GetHouseArea(houseInfo.second);

		return {};
	}

	eItemType GetMissingItemType(Elite::Blackboard* pBlackboard)
	{
		const auto& pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return eItemType::INVALID;

		for (int i{ static_cast<int>(eItemType::PISTOL) }; i < static_cast<int>(eItemType::FOOD); ++i)
			if (!pInventory->HasItem(static_cast<eItemType>(i)))
				return static_cast<eItemType>(i);

		return eItemType::INVALID;
	}


	Elite::Vector2 GetEnemyInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return INVALID_VECTOR2;

		auto entitiesInFOV{ GetEntitiesInFOV(pBlackboard) };
		if (entitiesInFOV.empty())
			return INVALID_VECTOR2;

		for (const auto& e : entitiesInFOV)
		{
			if (e->Type == eEntityType::ENEMY)
				return e->Location;
		}

		return entitiesInFOV[0]->Location;
	}

	Elite::Vector2 GetClosestKnownItemPos(Elite::Blackboard* pBlackboard)
	{
		// Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;
		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return INVALID_VECTOR2;
		auto& items{ pMemory->GetLocatedItems() };
		if (items.empty())
			return INVALID_VECTOR2;

		// Find closest located item
		Elite::Vector2 closestItem{ INVALID_VECTOR2 };
		const auto& pInfluenceMap(pMemory->GetInfluenceMap());
		const auto& agentInfo{ pSurvivor->GetInfo() };
		for (auto& item : items)
		{
			const auto& itemNode{ pInfluenceMap->GetNode(item) };

			if (agentInfo.Location.DistanceSquared(itemNode->GetPosition()) <
				agentInfo.Location.DistanceSquared(closestItem))
			{
				closestItem = itemNode->GetItemPos();
			}
		}

		return closestItem;
	}

	Elite::Vector2 GetClosestKnownItemTypePos(Elite::Blackboard* pBlackboard, eItemType type)
	{
		if (type == eItemType::INVALID)
			return { FLT_MAX, FLT_MAX };

		// Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;
		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return INVALID_VECTOR2;
		auto& itemIndices{ pMemory->GetLocatedItems() };
		if (itemIndices.empty())
			return INVALID_VECTOR2;

		// Find closest located item
		Elite::Vector2 closestItem{ FLT_MAX, FLT_MAX };
		const auto& pInfluenceMap(pMemory->GetInfluenceMap());
		const auto& agentInfo{ pSurvivor->GetInfo() };
		for (auto& itemIdx : itemIndices)
		{
			const auto& itemNode{ pInfluenceMap->GetNode(itemIdx) };

			if (itemNode->GetItem() == eItemType::INVALID)
				continue;

			// If needed type is a weapon but item on node is not, continue
			if (type == eItemType::WEAPON &&
				!(itemNode->GetItem() == eItemType::PISTOL
					|| itemNode->GetItem() == eItemType::SHOTGUN))
				continue;

			// Continue if item on node is not needed type
			// Ignore when need a weapon, since the item on node will never be "weapon"
			if (type != eItemType::WEAPON && itemNode->GetItem() != type)
				continue;

			if (agentInfo.Location.DistanceSquared(itemNode->GetPosition()) <
				agentInfo.Location.DistanceSquared(closestItem))
			{
				closestItem = itemNode->GetItemPos();
			}
		}

		return closestItem;
	}


	//TODO: allow behavior action nodes to take bools to pass to actions
	bool GetTrue(Elite::Blackboard* pBlackboard)
	{
		return true;
	}

	bool GetFalse(Elite::Blackboard* pBlackboard)
	{
		return false;
	}


}