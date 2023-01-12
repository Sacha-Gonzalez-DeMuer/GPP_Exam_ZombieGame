#include "stdafx.h"
//=== General Includes ===
#include "EBehaviorTree.h"
#include "Time.h"
using namespace Elite;

//-----------------------------------------------------------------
// BEHAVIOR TREE COMPOSITES (IBehavior)
//-----------------------------------------------------------------
#pragma region COMPOSITES
//SELECTOR
BehaviorState BehaviorSelector::Execute(Blackboard* pBlackBoard)
{
	//TODO: Fill in this code
	// Loop over all children in m_ChildBehaviors
	for (auto& child : m_ChildBehaviors)
	{
		m_CurrentState = child->Execute(pBlackBoard);

		switch (m_CurrentState)
		{
		default:
		case BehaviorState::Failure:
			continue;

		case BehaviorState::Success:
		case BehaviorState::Running:
			return m_CurrentState;
		}

		m_CurrentState = BehaviorState::Failure;
		return m_CurrentState;
	}

	m_CurrentState = BehaviorState::Failure;
	return m_CurrentState;
}
//SEQUENCE
BehaviorState BehaviorSequence::Execute(Blackboard* pBlackBoard)
{
	//TODO: FIll in this code
	//Loop over all children in m_ChildBehaviors
	for (auto& child : m_ChildBehaviors)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = child->Execute(pBlackBoard);

		//Check the currentstate and apply the sequence Logic:
		switch (m_CurrentState)
		{
		case BehaviorState::Running: 
		case BehaviorState::Failure:
			return m_CurrentState;
		}
	}

	//All children succeeded 
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}

//PARTIAL SEQUENCE
BehaviorState BehaviorPartialSequence::Execute(Blackboard* pBlackBoard)
{
	while (m_CurrentBehaviorIndex < m_ChildBehaviors.size())
	{
		m_CurrentState = m_ChildBehaviors[m_CurrentBehaviorIndex]->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			m_CurrentBehaviorIndex = 0;
			return m_CurrentState;
		case BehaviorState::Success:
			++m_CurrentBehaviorIndex;
			m_CurrentState = BehaviorState::Running;
			return m_CurrentState;
		case BehaviorState::Running:
			return m_CurrentState;
		}
	}

	m_CurrentBehaviorIndex = 0;
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}

#pragma endregion
//-----------------------------------------------------------------
// BEHAVIOR TREE CONDITIONAL (IBehavior)
//-----------------------------------------------------------------

BehaviorState BehaviorConditional::Execute(Blackboard* pBlackBoard)
{
	if (m_fpConditional == nullptr)
		return BehaviorState::Failure;

	switch (m_InvertCondition ? !m_fpConditional(pBlackBoard) : m_fpConditional(pBlackBoard))
	{
	case true:
		m_CurrentState = BehaviorState::Success;
		return m_CurrentState;
	case false:
		m_CurrentState = m_CurrentState = BehaviorState::Failure;
		return m_CurrentState;
	}

	return BehaviorState::Failure;
}


//-----------------------------------------------------------------
// BEHAVIOR TREE ACTION (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorAction::Execute(Blackboard* pBlackBoard)
{
	if (m_fpAction == nullptr)
		return BehaviorState::Failure;

	m_CurrentState = m_fpAction(pBlackBoard);
	return m_CurrentState;
}


BehaviorState Elite::BehaviorParallel::Execute(Blackboard* blackboard)
{
	size_t successCount = 0;
	size_t failureCount = 0;

	for (auto& child : m_children) {
		BehaviorState childStatus = child->Execute(blackboard);
		if (childStatus == BehaviorState::Failure) {
			return BehaviorState::Failure;
		}
		else if (childStatus == BehaviorState::Success) {
			++successCount;
			if (successCount >= m_minSuccess) {
				return BehaviorState::Success;
			}
		}
		else if (childStatus == BehaviorState::Running) {
			/*++failureCount;
			if (failureCount >= m_minFailure) {
				return BehaviorState::Failure;
			}*/
			return BehaviorState::Running;
		}
	}

	return BehaviorState::Running;
}
 
BehaviorState Elite::BehaviorWait::Execute(Blackboard* pBlackBoard)
{
	if (m_WaitTimer < m_WaitTime)
	{
		std::cout << "Waiting, " << m_WaitTimer;
		m_WaitTimer += Time::Get()->DeltaTime();
		return BehaviorState::Running;
	}

	m_WaitTimer = 0;
	return BehaviorState::Success;
}
