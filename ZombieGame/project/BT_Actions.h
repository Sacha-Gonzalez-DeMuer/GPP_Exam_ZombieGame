#pragma once
#include "Behaviors.h"
#include "BT_BlackboardGetters.h"

namespace BT_Actions
{
#define FAILURE Elite::BehaviorState::Failure
#define SUCCESS Elite::BehaviorState::Success
#define RUNNING Elite::BehaviorState::Running


	using namespace BT_Functions;

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
		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		IExamInterface* pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return FAILURE;


		EntityInfo* closestEntity{ pSurvivor->GetEntitiesInFOV().front() };
		for (EntityInfo* ei : pSurvivor->GetEntitiesInFOV())
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


	//Elite::BehaviorState GrabItem(Elite::Blackboard* pBlackboard)
	//{
	//	auto pSurvivor{ GetSurvivor(pBlackboard) };
	//	if (!pSurvivor)
	//		return FAILURE;

	//	auto pInventory{ GetInventory(pBlackboard) };
	//	if (!pInventory)
	//		return FAILURE;

	//	auto pMemory{ GetMemory(pBlackboard) };
	//	if (!pMemory)
	//		return FAILURE;

	//	ItemInfo item{};
	//	for (const auto& entity : pSurvivor->GetEntitiesInFOV())
	//	{
	//		if (entity->Type == eEntityType::ITEM && pInventory->GrabItem(*entity, item) && pMemory->OnPickUpItem(item))
	//		{
	//			return SUCCESS;
	//		}
	//	}

	//	return FAILURE;
	//}

	Elite::BehaviorState GrabItem(Elite::Blackboard* pBlackboard, const Elite::Vector2& entityPos)
	{
		if (entityPos == INVALID_VECTOR2)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return FAILURE;


		// Position correctly
		if (entityPos.DistanceSquared(pSurvivor->GetLocation()) > pSurvivor->GetInfo().GrabRange * pSurvivor->GetInfo().GrabRange)
		{
			pSurvivor->SetToSeek(entityPos);
			return RUNNING;
		}

		// Orient correctly
		EntityInfo e{};
		e.Location = entityPos;
		if (!pSurvivor->IsInFOV(e))
		{
			pSurvivor->SetToLookAt(entityPos);
			return RUNNING;
		}

		// Get Item
		ItemInfo item{};
		for (const auto& entity : pSurvivor->GetEntitiesInFOV())
		{
			if (entity->Type == eEntityType::ITEM && pInventory->GrabItem(*entity, item) && pMemory->OnPickUpItem(item))
			{
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

		if (pInventory->EquipItem(eItemType::SHOTGUN) || pInventory->EquipItem(eItemType::PISTOL))
			return SUCCESS;

		return FAILURE;
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
		if (target == INVALID_VECTOR2)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		EAgentInfo eAgentInfo{ pSurvivor->GetInfo() };
		const Elite::Vector2 agentForward{ eAgentInfo.GetForward() };
		const Elite::Vector2 toTarget{ (target - eAgentInfo.Location).GetNormalized() };
		const float angleBetween
		{
			Elite::ToDegrees(
				Elite::AngleBetween(agentForward, toTarget))
		};
		constexpr float errorMargin{ 15.f };

		std::cout << "Angle between: " << angleBetween << "\n";

		pSurvivor->SetToLookAt(target);
		if (angleBetween > errorMargin)
			return RUNNING;

		return SUCCESS;
	}

	Elite::BehaviorState SetClosestEnemyAsTarget(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		EntityInfo* closestEntity{ pSurvivor->GetEntitiesInFOV().front() };
		for (EntityInfo* ei : pSurvivor->GetEntitiesInFOV())
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
		if (target == INVALID_VECTOR2)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		pSurvivor->SetToSeek(target);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange - 1 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(target) > arriveRange * arriveRange)
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
			return INVALID_VECTOR2;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return INVALID_VECTOR2;

		auto seenHouses{ pMemory->GetSeenHouses() };

		if (seenHouses.empty() || seenHouses.size() <= pMemory->GetClearedHouses().size())
			return INVALID_VECTOR2;

		Elite::Vector2 closestPos{ FLT_MAX, FLT_MAX };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (auto& house : seenHouses)
		{
			if (house.Center.Distance(agentPos) < closestPos.Distance(agentPos))
				closestPos = house.Center;
		}

		return closestPos;
	}

	Elite::Vector2 GetClosestUnvisitedHousePos(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;
		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return INVALID_VECTOR2;
		auto seenHouses{ pMemory->GetSeenHouses() };
		if (seenHouses.empty() || seenHouses.size() <= pMemory->GetClearedHouses().size())
			return INVALID_VECTOR2;

		//Determine closest unvisited house location
		Elite::Vector2 closestPos{ INVALID_VECTOR2 };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (auto& house : seenHouses)
		{
			if (!pMemory->HasVisitedHouse(house) && house.Center.Distance(agentPos) < closestPos.Distance(agentPos))
				closestPos = house.Center;
		}

		return closestPos;
	}

	std::unordered_set<int> GetClosestUnvisitedHouseArea(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return {};
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return {};
		auto seenHouses{ pMemory->GetSeenHouses() };
		if (seenHouses.empty() || seenHouses.size() <= pMemory->GetClearedHouses().size())
			return {};

		//Determine unvisited closest house
		auto closestHouse{ seenHouses[0] };
		for (auto& house : seenHouses)
		{
			if (!pMemory->HasVisitedHouse(house)
				&& house.Center.Distance(pSurvivor->GetLocation()) < closestHouse.Center.Distance(pSurvivor->GetLocation()))
				closestHouse = house;
		}

		//Get its unvisited area
		auto area{ pMemory->GetHouseArea(closestHouse) };
		std::unordered_set<int> unclearedArea{};
		for (int idx : area)
		{
			if (!pMemory->GetInfluenceMap()->GetNode(idx)->GetScanned())
				unclearedArea.insert(idx);
		}

		return unclearedArea;
	}

	eItemType GetNeededItemType(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		const auto& pSurvivor{ GetSurvivor(pBlackboard) };


		//Determine needed item
		if (pSurvivor->GetInfo().Energy < pSurvivor->GetInfo().LowEnergyThreshold)
			return eItemType::FOOD;

		if (pSurvivor->GetInfo().Health < pSurvivor->GetInfo().LowHealthThreshold)
			return eItemType::MEDKIT;

		return eItemType::INVALID;
	}

	Elite::Vector2 GetClosestKnownItemPos(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;
		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return INVALID_VECTOR2;
		auto& items{ pMemory->GetLocatedItems() };
		if (items.empty())
			return INVALID_VECTOR2;

		//Find closest located item
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

		//Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;
		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return INVALID_VECTOR2;
		auto& itemIndices{ pMemory->GetLocatedItems() };
		if (itemIndices.empty())
			return INVALID_VECTOR2;

		//Find closest located item
		Elite::Vector2 closestItem{ FLT_MAX, FLT_MAX };
		const auto& pInfluenceMap(pMemory->GetInfluenceMap());
		const auto& agentInfo{ pSurvivor->GetInfo() };
		for (auto& itemIdx : itemIndices)
		{
			const auto& itemNode{ pInfluenceMap->GetNode(itemIdx) };

			if (itemNode->GetItem() != type)
				continue;

			if (agentInfo.Location.DistanceSquared(itemNode->GetPosition()) <
				agentInfo.Location.DistanceSquared(closestItem))
			{
				closestItem = itemNode->GetItemPos();
			}
		}

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


	Elite::BehaviorState Heal(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		if (pInventory->UseItem(eItemType::MEDKIT))
			return SUCCESS;

		return FAILURE;
	}

	Elite::BehaviorState Eat(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		if (pInventory->UseItem(eItemType::FOOD))
			return SUCCESS;

		return FAILURE;
	}

	Elite::BehaviorState DropGarbage(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		if (pInventory->EquipItem(eItemType::GARBAGE) && pInventory->DropItem())
			return SUCCESS;

		return FAILURE;
	}

	Elite::BehaviorState DropLeastValuableItem(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		//Drop item with least value
		if (pInventory->DropItem(pInventory->GetLowestValueItem()))
			return SUCCESS;

		return FAILURE;
	}

	Elite::BehaviorState DropEmptyItems(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		if (pInventory->DropEmptyItem())
			return SUCCESS;

		return FAILURE;
	}

	Elite::BehaviorState ShootTarget(Elite::Blackboard* pBlackboard, const Elite::Vector2& target)
	{
		if (target == INVALID_VECTOR2)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		const float errorMargin{ 5.0f };
		pSurvivor->SetToLookAt(target);
		if (Elite::ToDegrees(Elite::AngleBetween(pSurvivor->GetDirection(), target - pSurvivor->GetLocation())) > errorMargin)
			return RUNNING;


		if (pInventory->EquipItem(eItemType::SHOTGUN) || pInventory->EquipItem(eItemType::PISTOL))
		{
			pInventory->UseItem();
			return SUCCESS;
		}

		return FAILURE;
	}

	Elite::Vector2 GetClosestEnemyInFOV(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return INVALID_VECTOR2;

		auto entitiesInFOV{ pSurvivor->GetEntitiesInFOV() };
		if (entitiesInFOV.empty())
			return INVALID_VECTOR2;

		Elite::Vector2 closestEnemyPos{ FLT_MAX, FLT_MAX };
		for (const auto& e : entitiesInFOV)
		{
			if (e->Type == eEntityType::ENEMY
				&& pSurvivor->GetLocation().DistanceSquared(e->Location) < pSurvivor->GetLocation().DistanceSquared(closestEnemyPos))
				closestEnemyPos = e->Location;
		}

		return closestEnemyPos;
	}

	HouseInfo* GetClosestHouseInFOV(Elite::Blackboard* pBlackboard)
	{

		ISurvivorAgent* pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return nullptr;

		const auto housesInFOV{ pSurvivor->GetHousesInFOV() };
		if (housesInFOV.empty())
			return nullptr;


		HouseInfo* closestHouse{ housesInFOV.front() };
		for (HouseInfo* hi : housesInFOV)
		{
			if (hi->Center.DistanceSquared(pSurvivor->GetLocation()) < closestHouse->Center.DistanceSquared(pSurvivor->GetLocation()))
			{
				closestHouse = hi;
			}
		}

		return closestHouse;
	}


	Elite::BehaviorState ChangeToExploreArea(Elite::Blackboard* pBlackboard, std::unordered_set<int> area)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		if (area.size() <= 1)
			return FAILURE;

		pSurvivor->SetToExploreArea(area);

		if (!pSurvivor->IsAreaExplored())
			return RUNNING;

		return SUCCESS;
	}

	Elite::BehaviorState SetRunMode(Elite::Blackboard* pBlackboard, bool runMode)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetRunMode(runMode);
		return SUCCESS;
	}

	Elite::BehaviorState ExitHouse(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		const auto& pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		const auto& housesInFOV{ pSurvivor->GetHousesInFOV() };
		if (housesInFOV.empty())
			return FAILURE;

		Elite::Vector2 pos{ INVALID_VECTOR2 };
		//Get random position outside of house
		for (const auto& house : housesInFOV)
		{
			if (IsPointInRect(pSurvivor->GetInfo().Location, house->Center, house->Size))
			{
				pos = GetRandomPointOutsideRect(pSurvivor->GetInfo().FOV_Range * 3, house->Center, house->Size);
				break;
			}
		}

		std::cout << "Exiting house\n";

		//Go to point outside house
		pSurvivor->SetToSeek(pos);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange / 2 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(pos) > arriveRange * arriveRange)
			return RUNNING;

		std::cout << "House exited\n";

		return SUCCESS;
	}

	Elite::BehaviorState ChangeToNavigateInfluenceMap(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		pSurvivor->SetToNavigateInfluenceMap();
		return SUCCESS;
	}
}
