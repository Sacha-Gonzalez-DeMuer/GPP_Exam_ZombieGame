#pragma once
#include "stdafx.h"
#include "ISurvivorAgent.h"
#include "Behaviors.h"
#include "IExamInterface.h"
#include "Inventory.h"
#include "framework/SteeringBehaviors/Steering/CombinedSteeringBehaviors.h"
#include "framework/EliteData/EBlackboard.h"
#include <memory>

using namespace Elite;
using namespace BT_Actions;
using namespace BT_Conditions;

ISurvivorAgent::ISurvivorAgent(IExamInterface* pInterface)
	: m_pInventory{new Inventory(pInterface, pInterface->Inventory_GetCapacity())}
	, m_pMemory{std::make_shared<SurvivorAgentMemory>(pInterface)}
	, m_pInterface{pInterface}
	, SteeringAgent(pInterface)
{
	SteeringAgent::Initialize(m_pMemory);
	Initialize(pInterface);
}

ISurvivorAgent::~ISurvivorAgent()
{
	m_pMemory = nullptr;
}

void ISurvivorAgent::Initialize(IExamInterface* pInterface)
{
	InitializeBehaviorTree(pInterface);
}

void ISurvivorAgent::Update(float deltaTime, IExamInterface* pInterface, SteeringPlugin_Output& steering)
{
	UpdateObjectsInFOV(pInterface);
	m_pMemory->Update(deltaTime, pInterface, m_pEntitiesInFOV, m_pHousesInFOV);

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaTime);

	if(m_pCurrentSteering)
		steering = m_pCurrentSteering->CalculateSteering(deltaTime, pInterface);

	auto node{ m_pMemory->GetInfluenceMap()->GetNodeAtWorldPos(GetLocation()) };
}

void ISurvivorAgent::Render(float deltaTime, IExamInterface* pInterface)
{
	m_pMemory->DebugRender(pInterface);
}

bool ISurvivorAgent::IsInFOV(const EntityInfo& e) const
{
	for (auto entity : m_pEntitiesInFOV)
	{
		if (entity->Location.DistanceSquared(e.Location) < 4.0f) 
		{
			return true;
		}
	}
	return false;
}

HouseInfo* ISurvivorAgent::GetNearestHouse(const Elite::Vector2& fromPos) const
{
	HouseInfo* closestHouse{ m_pHousesInFOV[0] };
	for (const auto& house : m_pHousesInFOV)
	{
		if (house->Center.DistanceSquared(fromPos) < closestHouse->Center.DistanceSquared(fromPos))
			closestHouse = house;
	}

	return closestHouse;
}

void ISurvivorAgent::UpdateObjectsInFOV(IExamInterface* pInterface)
{
	for (HouseInfo* house : m_pHousesInFOV)
	{
		delete house;
		house = nullptr;
	}
	for (EntityInfo* pEntity : m_pEntitiesInFOV)
	{
		delete pEntity;
		pEntity = nullptr;
	}
	m_pHousesInFOV.clear();
	m_pEntitiesInFOV.clear();


	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (pInterface->Fov_GetHouseByIndex(i, hi))
		{
			m_pHousesInFOV.emplace_back(new HouseInfo(hi));
			continue;
		}

		break;
	}

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			m_pEntitiesInFOV.emplace_back(new EntityInfo(ei));
			continue;
		}

		break;
	}
}

void ISurvivorAgent::InitializeBehaviorTree(IExamInterface* pInterface)
{
	//Create and add necessary blackboard data
	//1. Create Blackboard
	Blackboard* pBlackboard = CreateBlackboard(pInterface);

	//2. Create BehaviorTree
	//BehaviorTree* pBehaviorTree{ new BehaviorTree(pBlackboard, new BehaviorSelector()) };
	BehaviorTree* pBehaviorTree{ new BehaviorTree(pBlackboard, new BehaviorSelector
	(
		{
			//============= SELF DEFENCE SEQUENCE =============//
			new BehaviorSequence
			({
				new BehaviorConditional(IsDangerNear),
				new BehaviorWhile(new BehaviorConditional(IsEnemyInFOV), new BehaviorAction(ChangeToNavigateInfluenceMap), true),
				new BehaviorSelector //Fight/Flight Selector
				({
					//new BehaviorSequence
					//({
					//	new BehaviorAction(EquipWeapon),
					//	new TBehaviorAction<Elite::Vector2>(ShootTarget, GetClosestEnemyInFOV),
					//}),


					//new BehaviorAction(FleeToNearestHouse)
				})
			}),

			//============= INVENTORY MANAGEMENT SELECTOR =============//
			new BehaviorSelector
			({
				new BehaviorSequence
				({
					new BehaviorConditional(HasEmptyItem),
					new BehaviorAction(DropEmptyItems)
				}),

				new BehaviorSequence
				({
					new BehaviorConditional(HasGarbage),
					new BehaviorAction(DropGarbage)
				})
			}),


			//============= SURVIVOR SELECTOR =============//
			new BehaviorSelector
			({
				new BehaviorSequence //Health sequence
				({
					new BehaviorConditional(IsHealthLow),
					new BehaviorAction(Heal)
				}),

				new BehaviorSequence
				({
					new BehaviorConditional(IsEnergyLow),
					new BehaviorAction(Eat)
				}),
			}),


			//============= LOOTING SELECTOR =============//
			new BehaviorSelector 
			({
				new BehaviorSequence //Free looting Sequence
				({
					//new NotDecorator(IsInventoryFull),
					new BehaviorConditional(IsInventoryFull, true),
					new BehaviorConditional(HasSeenItem),
					new TBehaviorAction<Elite::Vector2>(GrabItem, GetClosestKnownItemPos),
				}),

				new BehaviorSequence //Urgent Looting Sequence
				({
					new BehaviorConditional(NeedsItem),

					new BehaviorSelector //try finding needed item
					({
						new TBehaviorAction<Elite::Vector2, eItemType>(GoTo, GetClosestKnownItemTypePos, GetNeededItemType),
						new TBehaviorAction<std::unordered_set<int>>(ChangeToExploreArea, GetClosestUnvisitedHouseArea)
					}),

					new BehaviorSelector
					({
						new BehaviorSequence
						({
							new BehaviorConditional(IsInventoryFull),
							new BehaviorAction(DropLeastValuableItem)
						}),

						new TBehaviorAction<Elite::Vector2, eItemType>(GrabItem, GetClosestKnownItemTypePos, GetNeededItemType)
					}),
				}),
			}), 


			//============= EXPLORATION SELECTOR =============//
			new BehaviorSelector
			({
				new TBehaviorAction<std::unordered_set<int>>(ChangeToExploreArea, GetClosestUnvisitedHouseArea),

				new BehaviorAction(ChangeToExplore)
			}),
		}
	)) 
	};

	//3. Set BehaviorTree active on the agent
	m_pDecisionMaking = pBehaviorTree;
}

Elite::Blackboard* ISurvivorAgent::CreateBlackboard(IExamInterface* pInterface)
{
	Blackboard* pBlackboard = new Blackboard();

	pBlackboard->AddData("Survivor", this);
	pBlackboard->AddData("Interface", pInterface);
	pBlackboard->AddData("SurvivorSteering", &m_pCurrentSteering);
	pBlackboard->AddData("Inventory", m_pInventory);
	pBlackboard->AddData("InfluenceMap", m_pMemory->GetInfluenceMap());
	pBlackboard->AddData("Memory", m_pMemory);

	return pBlackboard;
}

