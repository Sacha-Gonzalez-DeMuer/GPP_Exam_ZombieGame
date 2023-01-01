#include "stdafx.h"
#include "ISurvivorAgent.h"
#include "framework/EliteData/EBlackboard.h"
#include "Behaviors.h"
#include "IExamInterface.h"
#include "Inventory.h"
#include "framework/SteeringBehaviors/Steering/CombinedSteeringBehaviors.h"
#include <memory>
using namespace Elite;
using namespace BT_Conditions;
using namespace BT_Actions;

ISurvivorAgent::ISurvivorAgent(IExamInterface* pInterface, Elite::InfluenceMap<InfluenceGrid>* pInfluenceMap)
	: m_pInventory{new Inventory(pInterface)}
	, m_pInfluenceMap{pInfluenceMap}
	, m_pMemory{new SurvivorAgentMemory()}
{
	Initialize(pInterface);
}

ISurvivorAgent::~ISurvivorAgent()
{
	m_pInfluenceMap = nullptr;

	delete m_pMemory;
	m_pMemory = nullptr;
}

void ISurvivorAgent::Initialize(IExamInterface* pInterface)
{
	InitializeBehaviorTree(pInterface);
	InitializeSteering();
}

void ISurvivorAgent::Update(float deltaTime, IExamInterface* pInterface, SteeringPlugin_Output& steering)
{
	UpdateSeenCells(pInterface);

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaTime);

	if(m_pCurrentSteering)
		steering = m_pCurrentSteering->CalculateSteering(deltaTime, &pInterface->Agent_GetInfo());

	m_pMemory->Update(deltaTime, pInterface, m_pEntitiesInFOV, m_pHousesInFOV);
	UpdateObjectsInFOV(pInterface);
}

void ISurvivorAgent::Render(float deltaTime, IExamInterface* pInterface)
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();

	Elite::Vector2 pos{ pInterface->Agent_GetInfo().Position };
	Elite::Vector2 lookDir{ eAgentInfo.GetForward() };
	Elite::Vector2 scanPos{ pos + (lookDir * eAgentInfo.FOV_Range) };
	pInterface->Draw_Segment(pos, scanPos, {1,1,1});
	//pInterface->Draw_Point(scanPos, 40.0f, { 1,1,1 });

	m_pMemory->DebugRender(pInterface);
}

void ISurvivorAgent::SetToWander()
{
	m_pCurrentSteering = m_pWander;
}

void ISurvivorAgent::SetToSeek(const Vector2& target)
{
	m_pSeek->SetTarget(target);
	m_pCurrentSteering = m_pSeek;
}

void ISurvivorAgent::SetTarget(const Elite::Vector2& target)
{
	m_Target = std::make_shared<Elite::Vector2>(target);
}

void ISurvivorAgent::SetToLookAt()
{
	if (!m_Target) return;

	m_pLookAt->SetTarget(*m_Target);
	m_pCurrentSteering = m_pLookAt;
}

void ISurvivorAgent::SetToLookAt(const Elite::Vector2& target)
{
	m_pLookAt->SetTarget(target);
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
void ISurvivorAgent::SetToFleeLookingAt()
{
	m_pFleeLookingAt->SetTarget(*m_Target);
	m_pCurrentSteering = m_pFleeLookingAt;
}
void ISurvivorAgent::SetToFleeLookingAt(const Elite::Vector2 target)
{
	m_pFleeLookingAt->SetTarget(target);
	m_pCurrentSteering = m_pFleeLookingAt;
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
			new BehaviorSequence //Self Defence Sequence
			({
				new BehaviorConditional(IsDangerNear),
				new BehaviorSelector
					({
						new BehaviorSequence
						({
							new BehaviorConditional(IsEnemyInFOV),
							new BehaviorAction(SetClosestEnemyAsTarget),
							new BehaviorSelector //Fight/Flight Selector
							({
								new BehaviorSequence
								({
									new BehaviorConditional(HasWeapon),
									new BehaviorAction(EquipWeapon),
									new BehaviorWhile(new NotDecorator(AlignedWithTarget), new BehaviorAction(ChangeToLookAt)),
									new BehaviorAction(UseItem)
								}),
							})
						}),
					}),
			}),

			new BehaviorSelector //Looting Selector
			({
				new BehaviorSequence
				({
					new BehaviorConditional(IsRewardNear),
					new BehaviorSelector
					({
						new BehaviorSequence
						({
							new NotDecorator(IsItemInFOV),
							new BehaviorAction(ChangeToLookAround)
						}),

						new BehaviorSequence
						({
							new BehaviorAction(GoToClosestEntity),
							new BehaviorAction(GrabItem)
						}),
					}),
				}),

				new BehaviorSequence
				({
					new NotDecorator(AreaScanned),
					new BehaviorAction(ChangeToLookAround)
				})
			}),

			//new BehaviorSequence
			//({
			//	new BehaviorConditional(IsHouseInFOV),
			//	//new BehaviorWhile(new NotDecorator(AreaScanned), new BehaviorAction(GoToClosestHouse))
			//	
			//}),

			new BehaviorAction(ChangeToWander) //Fallback to wander
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
	pBlackboard->AddData("HousesInFOV", &m_pHousesInFOV);
	pBlackboard->AddData("InfluenceMap", m_pInfluenceMap);
	pBlackboard->AddData("Memory", m_pMemory);
	pBlackboard->AddData("State", &m_SurvivorState);

	return pBlackboard;
}

void ISurvivorAgent::InitializeSteering()
{
	m_pWander = std::make_shared<Wander>();
	m_pSeek = std::make_shared<Seek>();
	m_pLookAround = std::make_shared<LookAround>();
	m_pLookAt = std::make_shared<LookAt>();
	m_pFlee = std::make_shared<Flee>();
	m_pFlee->SetRadius(100);

	m_pFleeLookingAt = std::make_shared<PrioritySteering>(std::vector<ISteeringBehavior*>
	{m_pFlee.get(), m_pLookAt.get()});
}

void ISurvivorAgent::UpdateSeenCells(IExamInterface* pInterface) const
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();
	const auto& indices{ m_pInfluenceMap->GetNodeIndicesInRadius(eAgentInfo.Position + (eAgentInfo.GetForward() * eAgentInfo.FOV_Range / 2.0f), eAgentInfo.FOV_Range/3)};

	m_pInfluenceMap->SetScannedAtPosition(indices, true);

	for (const auto& e : m_pEntitiesInFOV)
	{
		if (e->Type == eEntityType::ENEMY)
			m_pInfluenceMap->SetInfluenceAtPosition(e->Location, -20);

		if (e->Type == eEntityType::ITEM)
			m_pInfluenceMap->SetInfluenceAtPosition(e->Location, 50);
	}

	if (eAgentInfo.WasBitten) m_pInfluenceMap->SetInfluenceAtPosition(eAgentInfo.Position, -100);
}
