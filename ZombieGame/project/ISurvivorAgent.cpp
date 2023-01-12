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
using namespace BT_ObjectGetters;

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
	if (m_CooldownTimer < m_ShotCooldown)
		m_CooldownTimer += deltaTime;

	UpdateObjectsInFOV(pInterface);
	m_pMemory->Update(deltaTime, pInterface, m_pEntitiesInFOV, m_pHousesInFOV);

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaTime);

	SteeringAgent::Update(deltaTime, steering);
}

void ISurvivorAgent::Render(float deltaTime, IExamInterface* pInterface)
{
	SteeringAgent::Render(deltaTime);
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
			new BehaviorSelector
			({
				new BehaviorSequence// Fight
				({
					new BehaviorConditional(IsEnemyInFOV),
					new TBehaviorAction<Elite::Vector2>(ShootTarget, GetEnemyInFOV),
				}),
				
				new BehaviorSequence
				({
					new BehaviorConditional(SeesPurgeZone),
					new TBehaviorAction<Elite::Vector2>(RunTo, GetPositionOutsidePurgeZone)
				}),

				new BehaviorSequence // Flight
				({
					new BehaviorConditional(IsDangerNear),
					new NotDecorator(IsEnemyInFOV),
					new BehaviorAction(EscapeDanger), // away from negative influence
				}),
			}),

			//============= INVENTORY MANAGEMENT SELECTOR =============//
			new BehaviorSelector
			({
				new BehaviorSequence // Inventory cleaning
				({
					new BehaviorConditional(HasEmptyItem),
					new BehaviorAction(DropEmptyItems)
				}),

				new BehaviorSequence // Garbage management
				({
					new BehaviorConditional(HasGarbage),
					new BehaviorAction(DropGarbage)
				})
			}),


			//============= SURVIVOR SELECTOR =============//
			new BehaviorSelector
			({
				new BehaviorSequence // Health sequence
				({
					new BehaviorConditional(IsHealthLow),
					new BehaviorAction(Heal),
				}),

				new BehaviorSequence // Energy sequence
				({
					new BehaviorConditional(IsEnergyLow),
					new BehaviorAction(Eat)
				}),
			}),


			//============= LOOTING SELECTOR =============//
			new BehaviorSelector 
			({
				//Free looting Sequence
				//====
				new BehaviorSelector
				({
					new BehaviorSequence // Try having every item
					({
						new BehaviorConditional(IsInventoryFull),
						new NotDecorator(HasEveryItemType),
						new TBehaviorAction<Elite::Vector2, eItemType>(GoTo, GetClosestKnownItemTypePos, GetMissingItemType),
						new BehaviorAction(RemoveDuplicate),
						new TBehaviorAction<Elite::Vector2, eItemType>(GrabItem, GetClosestKnownItemTypePos, GetMissingItemType)
					}),

					new BehaviorSequence // Make sure inventory is always full
					({
						new NotDecorator(IsInventoryFull),
						new BehaviorConditional(HasSeenItem),
						new TBehaviorAction<Elite::Vector2>(GrabItem, GetClosestKnownItemPos),
					})
				}),

				//Urgent Looting Sequence		
				//====
				new BehaviorSequence
				({
					new BehaviorConditional(NeedsItem),

					new BehaviorSelector // Try finding needed item
					({
						new TBehaviorAction<Elite::Vector2, eItemType>(GrabItem, GetClosestKnownItemTypePos, GetNeededItemType),

						new BehaviorSequence
						({
							new BehaviorConditional(IsInventoryFull),
							new BehaviorAction(DropLeastValuableItem)
						}),

						new BehaviorSequence
						({
							new NotDecorator(ClearedAllLocatedHouses),
							new TBehaviorAction<std::unordered_set<int>>(ChangeToExploreArea, GetUnclearedHouseArea)
						})
					}),
				}),
			}), 


			//============= EXPLORATION SELECTOR =============//
			new BehaviorSelector
			({
				new BehaviorSequence // Clear known houses
				({
					new NotDecorator(ClearedAllLocatedHouses),
					new TBehaviorAction<std::unordered_set<int>>(ChangeToExploreArea, GetUnclearedHouseArea)
				}),

				new BehaviorAction(ChangeToPatrol) // Fall back to patrol
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

