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

	void SetTarget(const TargetData& target) { m_Target = target; }
	void SetRadius(const float radius) { m_Radius = radius; };
	void SetAutoOrient(bool enabled) { m_AutoOrient = enabled; };
	void SetRunMode(bool enabled) { m_RunMode = enabled; };
	template<class T_MemoryObject, typename std::enable_if<std::is_base_of<ISteeringBehavior, T_MemoryObject>::value>::type* = nullptr>
	T_MemoryObject* As()
	{ return static_cast<T_MemoryObject*>(this); }

protected:
	TargetData m_Target{};
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
//EXPLORE AREA
//****
class ClearArea final : public InfluenceNavigation {

public:
	ClearArea() = default;
	virtual ~ClearArea() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;
	void SetArea(std::unordered_set<int> area) { m_AreaToClear = area; };
	void AddToArea(std::unordered_set<int> toAdd) { m_AreaToClear.insert(toAdd.begin(), toAdd.end()); };
	std::unordered_set<int> GetArea() { return m_AreaToClear; };
	bool IsExplored() const { return m_AreaToClear.empty(); };
	void SetReachedTarget(bool reached) { m_ReachedTarget = reached; };
protected:
	std::unordered_set<int> m_AreaToClear{};
	bool m_ReachedTarget{ true };
};

class CircularPatrol final : public Wander
{
public:
	CircularPatrol() = default;
	virtual ~CircularPatrol() = default;
	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;

	void InitializeCircularPath(const Elite::Vector2& center, float radius, int nrPoints, float extensionLength);
	std::vector<Elite::Vector2> GetPath() const { return m_PatrolPoints; };
	void SetRadius(float radius) { m_PatrolRadius = radius; };
	void SetCenter(const Elite::Vector2& center) { m_Center = center; };
protected:
	std::vector<Elite::Vector2> m_PatrolPoints{};
	Elite::Vector2 m_Center;
	int m_PatrolIdx{ 0 };
	float m_PatrolRadius{ 100.f };
	float m_ExtensionLength{ 10.f };
};

///////////////////////////////////////
//NAVIGATE INFLUENCE
//****
class NavigateInfluence final : public InfluenceNavigation
{
public:
	NavigateInfluence() = default;
	virtual ~NavigateInfluence() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const IExamInterface* pInterface) override;
};

//==============================================\\
		//===========================\\


////////////////////////////////////////
//LOOK AROUND
//****
class LookAround final : public ISteeringBehavior {
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

private:
	float m_AngleError{ 1.0f };
};


#endif
