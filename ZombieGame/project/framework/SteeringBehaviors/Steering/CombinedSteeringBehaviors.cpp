#include "../../../stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>

BlendedSteering::BlendedSteering(std::vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringPlugin_Output BlendedSteering::CalculateSteering(float deltaT,const IExamInterface* pInterface)
{
	SteeringPlugin_Output blendedSteering = {};
	auto totalWeight = 0.f;

	for (auto weightedBehavior : m_WeightedBehaviors)
	{
		auto steering = weightedBehavior.pBehavior->CalculateSteering(deltaT, pInterface);
		blendedSteering.LinearVelocity += weightedBehavior.weight * steering.LinearVelocity;
		blendedSteering.AngularVelocity += weightedBehavior.weight * steering.AngularVelocity;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		auto scale = 1.f / totalWeight;
		blendedSteering.LinearVelocity *= scale;
		blendedSteering.AngularVelocity *= scale;
	}

	return blendedSteering;
}

//*****************
//PRIORITY STEERING
SteeringPlugin_Output PrioritySteering::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pInterface);

		//if (steering.IsValid)
		//	break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}


//*****************
//ADDITIVE STEERING
SteeringPlugin_Output AdditiveSteering::CalculateSteering(float deltaT, const IExamInterface* pInterface)
{
	SteeringPlugin_Output steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		SteeringPlugin_Output behaviorSteering{ pBehavior->CalculateSteering(deltaT, pInterface) };
		steering.AngularVelocity += behaviorSteering.AngularVelocity;
		steering.LinearVelocity += behaviorSteering.LinearVelocity;
	}


	return steering;
}