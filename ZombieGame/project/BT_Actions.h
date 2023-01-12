#pragma once
#include "Behaviors.h"
#include "BT_BlackboardGetters.h"
#include "Time.h"
namespace BT_Actions
{
#define FAILURE Elite::BehaviorState::Failure
#define SUCCESS Elite::BehaviorState::Success
#define RUNNING Elite::BehaviorState::Running


	using namespace BT_Functions;

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
		float distanceToItemSq{ entityPos.DistanceSquared(pSurvivor->GetLocation()) };
		// Position correctly
		if (distanceToItemSq > grabRange * grabRange)
		{
			pSurvivor->SetToSeek(entityPos, false);
			return RUNNING;
		}

		// Orient correctly
		EntityInfo e{};
		e.Location = entityPos;
		float angleBetween{ Elite::AngleBetween(pSurvivor->GetDirection(), (e.Location - pSurvivor->GetLocation()).GetNormalized()) };
		if (abs(Elite::ToDegrees(angleBetween)) > 5.0f)
		{
			pSurvivor->SetToLookAround();
			return RUNNING;
		}

		// Get Item
		ItemInfo item{};
		for (const auto& entity : pSurvivor->GetEntitiesInFOV())
		{
			if (entity->Type == eEntityType::ITEM && pInventory->GrabItem(*entity, item) && pMemory->OnPickUpItem(item))
				return SUCCESS;
		}

		return FAILURE;
	}


	Elite::BehaviorState RemoveDuplicate(Elite::Blackboard* pBlackboard)
	{
		auto pInventory{ GetInventory(pBlackboard) };
		if (!pInventory)
			return FAILURE;

		const auto& lowestValueDuplicated{ pInventory->GetLowestValueDuplicates() };

		for (const auto& duplicate : lowestValueDuplicated)
		{
			if (pInventory->DropItem(duplicate))
				return SUCCESS;
		}

		return FAILURE;
	}

	Elite::BehaviorState ChangeToLookAround(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToLookAround();
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


		pSurvivor->SetToLookAt(target);
		if (abs(angleBetween) > errorMargin)
			return RUNNING;

		return SUCCESS;
	}

	Elite::BehaviorState GoTo(Elite::Blackboard* pBlackboard, Elite::Vector2 target)
	{
		if (target == INVALID_VECTOR2)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		pSurvivor->SetToSeek(target, false);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange - 1 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(target) > arriveRange * arriveRange)
		{
			return RUNNING;
		}

		return SUCCESS;
	}

	Elite::BehaviorState RunTo(Elite::Blackboard* pBlackboard, Elite::Vector2 target)
	{
		if (target == INVALID_VECTOR2)
			return FAILURE;

		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;


		pSurvivor->SetToSeek(target, true);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange - 1 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(target) > arriveRange * arriveRange)
		{
			return RUNNING; //literally hah
		}

		return SUCCESS;
	}

	Elite::BehaviorState ChangeToRun(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetRunMode(true);
		return SUCCESS;
	}

	Elite::BehaviorState ChangeToWalk(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetRunMode(false);
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

		if(!pInventory->HasWeapon())
			return FAILURE;
		std::cout << "Shooting target\n";

		const float errorMargin{ 4.0f };
		pSurvivor->SetToLookAt(target);
		float angleBetween{ Elite::ToDegrees(Elite::AngleBetween(pSurvivor->GetDirection(), target - pSurvivor->GetLocation())) };
		
		if (abs(angleBetween) > errorMargin)
			return RUNNING;


		// Equip weapon with priority to shotgun
		if (pInventory->EquipItem(eItemType::SHOTGUN) || pInventory->EquipItem(eItemType::PISTOL))
		{
			if (!pSurvivor->GunOnCooldown() && pInventory->UseItem())
			{
				pSurvivor->OnShoot(); // Set cooldown time
				return RUNNING;
			}
		}

		return FAILURE;
	}

	Elite::BehaviorState ChangeToExploreArea(Elite::Blackboard* pBlackboard, std::unordered_set<int> area)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;
		auto pMemory{ GetMemory(pBlackboard) };
		if (!pMemory)
			return FAILURE;
		

		if (!pMemory->IsAreaExplored(area))
		{
			pSurvivor->SetToExploreArea(area);
			return RUNNING;
		}

		return SUCCESS;
	}

	Elite::BehaviorState ChangeToPatrol(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		pSurvivor->SetToPatrol();
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
				pos = GetRandomPointOutsideRect(max(house->Size.x, house->Size.y) * 2, house->Center, house->Size);
				break;
			}
		}

		//Go to point outside house
		pSurvivor->SetToSeek(pos, false);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange / 2 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(pos) > arriveRange * arriveRange)
			return RUNNING;

		return SUCCESS;
	}

	Elite::BehaviorState EscapeDanger(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto pInfluenceMap{ GetInfluenceMap(pBlackboard) };
		if (!pInfluenceMap)
			return FAILURE;

		// Continue navigating influence map whilst in danger
		const float errorMargin{ 5.0f };
		const float currentNodeInfluence{ pInfluenceMap->GetNodeAtWorldPos(pSurvivor->GetLocation())->GetInfluence() };
		if (currentNodeInfluence < -errorMargin)
		{
			pSurvivor->SetToNavigateInfluenceMap();
			return SUCCESS;
		}

		return FAILURE;
	}

	Elite::BehaviorState ScanDanger(Elite::Blackboard* pBlackboard)
	{
		auto pSurvivor{ GetSurvivor(pBlackboard) };
		if (!pSurvivor)
			return FAILURE;

		auto pInfluenceMap{ GetInfluenceMap(pBlackboard) };
		if (!pInfluenceMap)
			return FAILURE;

		auto pInterface{ GetInterface(pBlackboard) };
		if (!pInterface)
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
		for (const auto& house : pMemory->GetLocatedHouses())
		{
			if (house.second.Center.DistanceSquared(pSurvivor->GetLocation()) < closestHousePos.DistanceSquared(pSurvivor->GetLocation()))
			{
				closestHousePos = house.second.Center;
			}
		}

		if (closestHousePos == INVALID_VECTOR2)
			return FAILURE;

		pSurvivor->SetToSeek(closestHousePos, true);
		const float arriveRange{ pSurvivor->GetInfo().GrabRange / 2 };
		if (pSurvivor->GetInfo().Location.DistanceSquared(closestHousePos) > arriveRange * arriveRange)
			return RUNNING;

		return SUCCESS;
	}

	Elite::BehaviorState PrintTest(Elite::Blackboard* pBlackboard)
	{
		std::cout << "Print test hit!\n";
		return SUCCESS;
	}
}
