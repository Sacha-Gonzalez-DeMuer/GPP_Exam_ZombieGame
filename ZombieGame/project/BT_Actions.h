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

		float grabRange{ pSurvivor->GetInfo().GrabRange * .8f };
		// Position correctly
		if (entityPos.DistanceSquared(pSurvivor->GetLocation()) > grabRange * grabRange)
		{
			pSurvivor->SetToSeek(entityPos);
			return RUNNING;
		}

		// Orient correctly
		EntityInfo e{};
		e.Location = entityPos;
		float angleBetween{ Elite::AngleBetween(pSurvivor->GetDirection(), (e.Location - pSurvivor->GetLocation()).GetNormalized()) };
		if (Elite::ToDegrees(angleBetween) > 5.0f)
		{
			pSurvivor->SetToLookAround();
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

	Elite::Vector2 GetClosestUnvisitedHousePos(Elite::Blackboard* pBlackboard)
	{
		//Get necessary data
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return INVALID_VECTOR2;
		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
			return INVALID_VECTOR2;

		auto locatedHouses{ pMemory->GetLocatedHouses() };
		if (locatedHouses.empty())
			return INVALID_VECTOR2;

	
		//Determine closest unvisited house location
		Elite::Vector2 closestPos{ INVALID_VECTOR2 };
		const Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Location };
		for (auto& house : locatedHouses)
		{
			if (!pMemory->IsHouseCleared(house.second) && house.second.Center.Distance(agentPos) < closestPos.Distance(agentPos))
				closestPos = house.second.Center;
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

		auto locatedHouses{ pMemory->GetLocatedHouses() };
		if (locatedHouses.empty())
			return {};

		//Determine unvisited closest house
		EHouseInfo* closestHouse{ nullptr };
		std::unordered_set<int> area{ };

		for (auto& house : locatedHouses)
		{
			if (!closestHouse && !house.second.Cleared)
				closestHouse = &house.second;

			if (!pMemory->IsHouseCleared(house.second, area)
				&& house.second.Center.DistanceSquared(pSurvivor->GetLocation()) < closestHouse->Center.DistanceSquared(pSurvivor->GetLocation()))
			{
				closestHouse = &house.second;
			}
		}

		if (closestHouse == nullptr)
			return {};

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
			if(type != eItemType::WEAPON && itemNode->GetItem() != type)
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
		std::cout << "healing\n";

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

		if (pInventory->DropItem(eItemType::GARBAGE))
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



		//Drop item with least value
		if (pInventory->DropItem(pInventory->GetEmptyItemIdx()))
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

		if(!(pInventory->HasItem(eItemType::SHOTGUN) || pInventory->HasItem(eItemType::PISTOL)))
			return FAILURE;


		const float errorMargin{ 5.0f };
		pSurvivor->SetToLookAt(target);
		if (Elite::ToDegrees(Elite::AngleBetween(pSurvivor->GetDirection(), target - pSurvivor->GetLocation())) > errorMargin)
			return RUNNING;

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
		if (area.empty())
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return FAILURE;

		if(pSurvivor->IsAreaExplored())
			pSurvivor->SetToExploreArea(area);

		if (!pMemory->IsAreaExplored(area))
		{
			std::cout << "Exploring Area\n";
			pSurvivor->SetToExploreArea();
			return RUNNING;
		}

		std::cout << "area is explored\n";
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

	Elite::BehaviorState FleeToNearestHouse(Elite::Blackboard* pBlackboard)
	{
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		Elite::Vector2 closestHousePos{ INVALID_VECTOR2 };
		for (auto house : pMemory->GetLocatedHouses())
		{
			if (house.second.Center.DistanceSquared(pSurvivor->GetLocation()) < closestHousePos.DistanceSquared(pSurvivor->GetLocation()))
			{
				closestHousePos = house.second.Center;
			}
		}


		if (closestHousePos == INVALID_VECTOR2)
			return FAILURE;

		pSurvivor->SetToSeek(closestHousePos);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange / 2 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(closestHousePos) > arriveRange * arriveRange)
			return RUNNING;

		return SUCCESS;
	}
}
