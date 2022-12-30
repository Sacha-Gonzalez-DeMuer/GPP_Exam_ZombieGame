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
#include "../SteeringHelpers.h"
#include "../../../ExtendedStructs.h"

class SteeringAgent;
struct AgentInfo;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }
	void SetRadius(const float radius) { m_Radius = radius; };

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
	float m_Radius;
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
	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;
};



///////////////////////////////////////
//FLEE
//****
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;
};



///////////////////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;
};


///////////////////////////////////////
//WANDER
//****
class Wander : public Seek {
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;
	void SetWanderOffset(float offset);

protected:
	float m_OffsetDistance = 6.f;
	float m_Radius = 20.f;
	float m_MaxAngleChange = Elite::ToRadians(45.f);
	float m_WanderAngle = 0.f;
};

///////////////////////////////////////
//EVADE
//****
class Evade : public ISteeringBehavior {
public:
	Evade() = default;
	virtual ~Evade() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;

};


///////////////////////////////////////
//PURSUIT
//****
class Pursuit : public Seek {
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;

	float m_ForesightDepth{ 5.f };
};


////////////////////////////////////////
//LOOK AROUND
//****
class LookAround : public ISteeringBehavior {
public:
	LookAround() = default;
	virtual ~LookAround() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;

	const float m_DegreesPerSecond{ 1.0f };

};

////////////////////////////////////////
//FACE
//****
class LookAt : public ISteeringBehavior {
public:
	LookAt() = default;
	virtual ~LookAt() = default;

	SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo* pAgent) override;

};
#endif