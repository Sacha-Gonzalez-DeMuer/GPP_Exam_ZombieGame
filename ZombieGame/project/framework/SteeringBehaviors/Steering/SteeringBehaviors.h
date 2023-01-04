/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "..\SteeringHelpers.h"
#include "../../../ExtendedStructs.h"
#include "..\..\EliteAI\EliteGraphs\EGraph2D.h"
#include "..\..\EliteAI\EliteGraphs\EGridGraph.h"
#include "..\..\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "..\..\..\SurvivorAgentMemory.h"

class SteeringAgent;
class IExamInterface;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }
	void SetRadius(const float radius) { m_Radius = radius; };
	void SetAutoOrient(bool enabled) { m_AutoOrient = enabled; };
	void SetRunMode(bool enabled) { m_RunMode = enabled; };
	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
	float m_Radius{10.0f};

	bool m_AutoOrient{true};
	bool m_RunMode{false};
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pAgent) override;
};



///////////////////////////////////////
//FLEE
//****
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pAgent) override;
};



///////////////////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pAgent) override;
};


///////////////////////////////////////
//WANDER
//****
class Wander : public Seek {
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pAgent) override;
	void SetWanderOffset(float offset);

protected:
	float m_OffsetDistance = 10.f;
	float m_Radius = 10.f;
	float m_MaxAngleChange = Elite::ToRadians(45.f);
	float m_WanderAngle = 0.f;
};


//===========================NAVMESH NAVIGATION===================\\

class InfluenceNavigation : public Wander {

protected:
	using InfluenceGrid = Elite::GridGraph<Elite::WorldNode, Elite::GraphConnection>;
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceMap{ nullptr };
	std::shared_ptr<SurvivorAgentMemory> m_pMemory{ nullptr };
	
public:
	InfluenceNavigation() = default;
	virtual ~InfluenceNavigation() {
		m_pInfluenceMap = nullptr;
	};

	void Initialize(std::shared_ptr<SurvivorAgentMemory> pMemory)
	{
		SetMemory(pMemory);
		SetInfluenceMap(pMemory->GetInfluenceMap());
	}


	virtual SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) { return SteeringPlugin_Output(); };

	void SetInfluenceMap(Elite::InfluenceMap<InfluenceGrid>* influenceMap) { m_pInfluenceMap = influenceMap; };
	void SetMemory(std::shared_ptr<SurvivorAgentMemory> pMemory) { m_pMemory = pMemory; };

};


///////////////////////////////////////
//EXPLORE
//****
class SurvivorAgentMemory;
class Explore : public InfluenceNavigation {
	
public:
	Explore() = default;
	virtual ~Explore() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;
};


///////////////////////////////////////
//EXPLORE AREA
//****
class ExploreArea : public Explore {

public:
	ExploreArea() = default;
	virtual ~ExploreArea() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;
	void SetArea(std::unordered_set<int> area) { m_AreaToExplore = area; };
	void AddToArea(std::unordered_set<int> toAdd) { m_AreaToExplore.insert(toAdd.begin(), toAdd.end()); };
	std::unordered_set<int> GetArea() { return m_AreaToExplore; };
	bool IsExplored() { return m_Explored; };
protected:
	std::unordered_set<int> m_AreaToExplore{};
	bool m_Explored{ false };
};

///////////////////////////////////////
//NAVIGATE INFLUENCE
//****
class NavigateInfluence : public InfluenceNavigation
{
public:
	NavigateInfluence() = default;
	virtual ~NavigateInfluence() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;
};

//==============================================\\
		//===========================\\

///////////////////////////////////////
//EVADE
//****
class Evade : public ISteeringBehavior {
public:
	Evade() = default;
	virtual ~Evade() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;
};


///////////////////////////////////////
//PURSUIT
//****
class Pursuit : public Seek {
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;

	float m_ForesightDepth{ 5.f };
};


////////////////////////////////////////
//LOOK AROUND
//****
class LookAround : public ISteeringBehavior {
public:
	LookAround() = default;
	virtual ~LookAround() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;

	const float m_DegreesPerSecond{ 1.0f };

};

////////////////////////////////////////
//FACE
//****
class LookAt : public ISteeringBehavior {
public:
	LookAt() = default;
	virtual ~LookAt() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;

};


#endif
