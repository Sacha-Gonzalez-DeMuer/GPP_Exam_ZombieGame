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

ISurvivorAgent::ISurvivorAgent(IExamInterface* pInterface)
	: m_pInventory{new Inventory(pInterface, pInterface->Inventory_GetCapacity())}
	, m_pMemory{new SurvivorAgentMemory(pInterface)}
{
	Initialize(pInterface);
}

ISurvivorAgent::~ISurvivorAgent()
{
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
	UpdateObjectsInFOV(pInterface);
	m_pMemory->Update(deltaTime, pInterface, m_pEntitiesInFOV, m_pHousesInFOV);

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaTime);

	if(m_pCurrentSteering)
		steering = m_pCurrentSteering->CalculateSteering(deltaTime, pInterface);

}

void ISurvivorAgent::Render(float deltaTime, IExamInterface* pInterface)
{
	EAgentInfo eAgentInfo = pInterface->Agent_GetInfo();

	Elite::Vector2 pos{ pInterface->Agent_GetInfo().Location };
	Elite::Vector2 lookDir{ eAgentInfo.GetForward() };
	Elite::Vector2 scanPos{ pos + (lookDir * eAgentInfo.FOV_Range) };
	pInterface->Draw_Segment(pos, scanPos, {1,1,1});

	m_pMemory->DebugRender(pInterface);
}

void ISurvivorAgent::SetToWander(bool autoOrient, bool sprint)
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
	m_pFlee->SetTarget(*m_Target);
	m_pLookAt->SetTarget(*m_Target);
	std::cout << "target;" << m_Target->x << ", " << m_Target->y << "\n";
	m_pCurrentSteering = m_pFleeLookingAt;
}

void ISurvivorAgent::SetToFleeLookingAt(const Elite::Vector2& target)
{
	m_pFlee->SetTarget(target);
	m_pLookAt->SetTarget(target);
	std::cout << "target;" << target.x << ", " << target.y << "\n";
	m_pCurrentSteering = m_pFleeLookingAt;
}

void ISurvivorAgent::SetToExplore()
{
	m_pCurrentSteering = m_pExplore;
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
								
								new BehaviorWhile(new BehaviorConditional(IsEnemyInFOV), new BehaviorAction(ChangeToFleeLookingAt))
							})
						}),

						new BehaviorAction(ChangeToLookAround),
					}),
			}),

			new BehaviorSequence	
			({
				new NotDecorator(IsInventoryFull),

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
					}),

					new BehaviorSequence
					({
						new BehaviorConditional(HasSeenItem),
						new BehaviorAction(GoToClosestKnownItem)
					})
				}),
			}),
			

			new BehaviorSequence
			({
				new BehaviorConditional(IsHouseInFOV),
				new TNotDecorator<HouseInfo*>(IsHouseVisited, GetClosestHouseInFOV),
				new BehaviorAction(GoToClosestHouseInFOV)
			}),

			new BehaviorAction(ChangeToExplore)
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
	pBlackboard->AddData("EntitiesInFOV", &m_pEntitiesInFOV);
	pBlackboard->AddData("Interface", pInterface);
	pBlackboard->AddData("SurvivorSteering", &m_pCurrentSteering);
	pBlackboard->AddData("Inventory", m_pInventory);
	pBlackboard->AddData("HousesInFOV", &m_pHousesInFOV);
	pBlackboard->AddData("InfluenceMap", m_pMemory->GetInfluenceMap());
	pBlackboard->AddData("Memory", m_pMemory);
	pBlackboard->AddData("State", &m_SurvivorState);

	return pBlackboard;
}

void ISurvivorAgent::InitializeSteering()
{
	m_pWander = std::make_shared<Wander>();
	m_pWander->SetAutoOrient(true);
	m_pSeek = std::make_shared<Seek>();
	m_pLookAround = std::make_shared<LookAround>();
	m_pLookAt = std::make_shared<LookAt>();
	m_pFlee = std::make_shared<Flee>();
	m_pFlee->SetRadius(1000);

	m_pFleeLookingAt = std::make_shared<AdditiveSteering>(std::vector<ISteeringBehavior*>
	{m_pFlee.get(), m_pLookAt.get()});
	m_pFleeLookingAt->SetRunMode(true);
	m_pFleeLookingAt->SetAutoOrient(false);


	m_pExplore = std::make_shared<Explore>();
	std::dynamic_pointer_cast<Explore>(m_pExplore)->SetInfluenceMap(m_pMemory->GetInfluenceMap());
	m_pExplore->SetRunMode(true);
}

