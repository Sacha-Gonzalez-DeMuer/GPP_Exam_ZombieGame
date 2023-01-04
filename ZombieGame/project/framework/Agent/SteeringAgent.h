/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringAgent.h: basic agent using steering behaviors
/*=============================================================================*/
#ifndef STEERING_AGENT_H
#define STEERING_AGENT_H

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "BaseAgent.h"
#include <unordered_set>
class ISteeringBehavior;
class SurvivorAgentMemory;


class SteeringAgent : public BaseAgent
{
public:
	//--- Constructor & Destructor ---
	SteeringAgent() = delete;
	SteeringAgent(IExamInterface* pInterface) : BaseAgent(pInterface) 
	{ };
	virtual ~SteeringAgent() = default;

	//--- Agent Functions ---
	void Update(float dt, SteeringPlugin_Output& steering) override;
	void Render(float dt) override;

	float GetMaxLinearSpeed() const { return GetInfo().MaxLinearSpeed; }

	float GetMaxAngularSpeed() const { return GetInfo().MaxAngularSpeed; }

	Elite::Vector2 GetDirection() const { return GetInfo().GetForward(); }
	Elite::Vector2 GetLocation() const { return GetInfo().Location; }

	virtual void SetSteeringBehavior(std::shared_ptr<ISteeringBehavior>pBehavior) { m_pCurrentSteering = pBehavior; }
	std::shared_ptr<ISteeringBehavior> GetSteeringBehavior() const { return m_pCurrentSteering; }


	//Steering setters
	void SetSteeringBehavior(ISteeringBehavior* pBehavior) {}
	void SetToWander(bool autoOrient, bool sprint);
	void SetToSeek(const Elite::Vector2& target);
	void SetTarget(const Elite::Vector2& target);
	void SetToLookAt();
	void SetToLookAt(const Elite::Vector2& target);
	void SetToLookAround();
	void SetToFlee();
	void SetToFleeLookingAt();
	void SetToFleeLookingAt(const Elite::Vector2& target);
	void SetToExplore();
	bool SetToFleeLookingAround();
	void SetToFleeLookingAround(const Elite::Vector2& target);
	void SetRunMode(bool enabled);
	void SetToNavigateInfluenceMap();

	void SetToExploreArea();
	bool IsAreaExplored() const;
	void SetToExploreArea(std::unordered_set<int> area);
	void AddToExploreArea(std::unordered_set<int> toAdd);

	std::shared_ptr<Elite::Vector2> GetTarget() const { return m_Target; };


protected:
	void Initialize(std::shared_ptr<SurvivorAgentMemory> pMemory);

	//--- Datamembers ---
	//Steering
	std::shared_ptr<ISteeringBehavior> m_pCurrentSteering{ nullptr };
	std::shared_ptr<ISteeringBehavior> m_pWander;
	std::shared_ptr<ISteeringBehavior> m_pSeek;
	std::shared_ptr<ISteeringBehavior> m_pLookAround;
	std::shared_ptr<ISteeringBehavior> m_pLookAt;
	std::shared_ptr<ISteeringBehavior> m_pFlee;
	std::shared_ptr<ISteeringBehavior> m_pFleeLookingAt;
	std::shared_ptr<ISteeringBehavior> m_pFleeLookingAround;
	std::shared_ptr<ISteeringBehavior> m_pExplore;
	std::shared_ptr<ISteeringBehavior> m_pExploreArea;
	std::shared_ptr<ISteeringBehavior> m_pNavigateInfluence;

	std::shared_ptr<Elite::Vector2> m_Target{};

private:
};
#endif