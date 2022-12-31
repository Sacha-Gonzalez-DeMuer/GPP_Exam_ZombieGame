#include "stdafx.h"
#include "ISurvivorAgent.h"
#include "framework/EliteData/EBlackboard.h"
#include "Behaviors.h"
#include "IExamInterface.h"
#include "Inventory.h"
#include <memory>
using namespace Elite;
using namespace BT_Conditions;
using namespace BT_Actions;

ISurvivorAgent::ISurvivorAgent(IExamInterface* pInterface, Elite::InfluenceMap<InfluenceGrid>* pInfluenceMap)
	: m_pInventory{new Inventory(pInterface)}
	, m_pInfluenceMap{pInfluenceMap}
{
	Initialize(pInterface);
}

ISurvivorAgent::~ISurvivorAgent()
{
	m_pInfluenceMap = nullptr;
}

void ISurvivorAgent::Initialize(IExamInterface* pInterface)
{
	InitializeBehaviorTree(pInterface);
	InitializeSteering();
}

void ISurvivorAgent::Update(float deltaTime, IExamInterface* pInterface, SteeringPlugin_Output& steering)
{
	UpdateObjectsInFOV(pInterface);
	UpdateSeenCells(pInterface);

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaTime);

	steering = m_pCurrentSteering->CalculateSteering(deltaTime, &pInterface->Agent_GetInfo());
}

void ISurvivorAgent::Render(float deltaTime, IExamInterface* pInterface)
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();

	Elite::Vector2 pos{ pInterface->Agent_GetInfo().Position };
	Elite::Vector2 lookDir{ eAgentInfo.GetForward() };
	Elite::Vector2 scanPos{ pos + (lookDir * eAgentInfo.FOV_Range) };
	pInterface->Draw_Segment(pos, scanPos, {1,1,1});
	//pInterface->Draw_Point(scanPos, 40.0f, { 1,1,1 });
}

void ISurvivorAgent::SetToWander()
{
	m_pCurrentSteering = m_pWander;
}

void ISurvivorAgent::SetToSeek(Vector2 target)
{
	m_pSeek->SetTarget(target);
	m_pCurrentSteering = m_pSeek;
}

void ISurvivorAgent::SetLookAtTarget(std::shared_ptr<Elite::Vector2> target)
{
	m_Target = target;
}

void ISurvivorAgent::SetToLookAt()
{
	if (!m_Target) return;

	m_pLookAt->SetTarget(*m_Target);
	m_pCurrentSteering = m_pLookAt;
}

void ISurvivorAgent::SetToLookAround()
{
	m_pCurrentSteering = m_pLookAround;
}

void ISurvivorAgent::SetToFlee()
{
	m_pFlee->SetTarget(*m_Target);
	m_pCurrentSteering = m_pFlee;
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
			//std::unique_ptr<EntityInfo> pEntity = std::make_unique<EntityInfo>(ei);
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
			/*new BehaviorSequence 
			({
				new BehaviorConditional(IsLookingAround),
			})*/
			new BehaviorSequence
			({
				new BehaviorConditional(IsEnemyInFOV),
				new BehaviorAction(SetClosestEnemyAsTarget),
				new BehaviorAction(ChangeToLookAt),
				new BehaviorSelector
				({
					new BehaviorSequence
					({
						new BehaviorConditional(HasWeapon),
						new BehaviorAction(EquipWeapon),
						new BehaviorAction(UseItem)
					}),
					new BehaviorConditional(HasTarget),
					new BehaviorAction(ChangeToFlee)
				})

			}),
			new BehaviorSelector //grab closest item in vision
			({
				new BehaviorSequence 
				({
					new BehaviorConditional(IsItemInFOV),
					new BehaviorAction(GoToClosestEntity),
					new BehaviorConditional(IsInRangeOfItem),
					new BehaviorAction(GrabItem)
				}),
			
			}),
			/*new BehaviorSequence
			({
				new BehaviorConditional(IsEntityInFOV),
				new BehaviorAction(GoToClosestEntity)
			}),*/

			/*new BehaviorSequence
			({
				new BehaviorConditional(IsHouseInFOV),
				new BehaviorAction(GoToClosestHouse)
			}),*/

			//Fallback to wander
			new BehaviorAction(ChangeToWander)
		}
	)) };

	//3. Set BehaviorTree active on the agent
	m_pDecisionMaking = pBehaviorTree;
}

Elite::Blackboard* ISurvivorAgent::CreateBlackboard(IExamInterface* pInterface)
{
	Blackboard* pBlackboard = new Blackboard();

	pBlackboard->AddData("Survivor", this);
	pBlackboard->AddData("EntitiesInFOV", &m_pEntitiesInFOV);
	pBlackboard->AddData("Interface", pInterface);
	pBlackboard->AddData("SurvivorSteering", &m_pCurrentSteering);
	pBlackboard->AddData("Inventory", m_pInventory);
	pBlackboard->AddData("Target", m_Target);
	pBlackboard->AddData("HousesInFOV", &m_pHousesInFOV);
	pBlackboard->AddData("InfluenceMap", m_pInfluenceMap);

	return pBlackboard;
}

void ISurvivorAgent::InitializeSteering()
{
	m_pWander = std::make_shared<Wander>();
	m_pSeek = std::make_shared<Seek>();
	m_pLookAround = std::make_shared<LookAround>();
	m_pLookAt = std::make_shared<LookAt>();
	m_pFlee = std::make_shared<Flee>();
}

void ISurvivorAgent::UpdateSeenCells(IExamInterface* pInterface) const
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();

	Elite::Vector2 pos{ pInterface->Agent_GetInfo().Position };
	Elite::Vector2 lookDir{ eAgentInfo.GetForward() };
	Elite::Vector2 scanPos{ pos + (lookDir * eAgentInfo.FOV_Range)};
	m_pInfluenceMap->SetInfluenceAtPosition(scanPos, 100);
}
