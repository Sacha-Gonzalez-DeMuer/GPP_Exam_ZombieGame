/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviorTree.h: Implementation of a BehaviorTree and the components of a Behavior Tree
/*=============================================================================*/
#ifndef ELITE_BEHAVIOR_TREE
#define ELITE_BEHAVIOR_TREE

//--- Includes ---
#include "stdafx.h"
#include "EBehaviorTree.h"

namespace Elite
{
	//-----------------------------------------------------------------
	// BEHAVIOR TREE HELPERS
	//-----------------------------------------------------------------
	enum class BehaviorState
	{
		Failure,
		Success,
		Running
	};

	//-----------------------------------------------------------------
	// BEHAVIOR INTERFACES (BASE)
	//-----------------------------------------------------------------
	class IBehavior
	{
	public:
		IBehavior() = default;
		virtual ~IBehavior() = default;
		virtual BehaviorState Execute(Blackboard* pBlackBoard) = 0;
		BehaviorState GetCurrentState() const { return m_CurrentState; };
	protected:
		BehaviorState m_CurrentState = BehaviorState::Failure;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE COMPOSITES (IBehavior)
	//-----------------------------------------------------------------
#pragma region COMPOSITES
	//--- COMPOSITE BASE ---
	class BehaviorComposite : public IBehavior
	{
	public:
		explicit BehaviorComposite(std::vector<IBehavior*> childBehaviors)
		{ m_ChildBehaviors = childBehaviors;	}
		virtual ~BehaviorComposite()
		{
			for (auto pb : m_ChildBehaviors)
				SAFE_DELETE(pb);
			m_ChildBehaviors.clear();
		}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override = 0;

	protected:
		std::vector<IBehavior*> m_ChildBehaviors = {};
	};

	//--- SELECTOR ---
	class BehaviorSelector : public BehaviorComposite
	{
	public:
		explicit BehaviorSelector(std::vector<IBehavior*> childBehaviors) :
			BehaviorComposite(childBehaviors) {}
		virtual ~BehaviorSelector() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//--- SEQUENCE ---
	class BehaviorSequence : public BehaviorComposite
	{
	public:
		explicit BehaviorSequence(std::vector<IBehavior*> childBehaviors) :
			BehaviorComposite(childBehaviors) {}
		virtual ~BehaviorSequence() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//--- PARTIAL SEQUENCE ---
	class BehaviorPartialSequence : public BehaviorSequence
	{
	public:
		explicit BehaviorPartialSequence(std::vector<IBehavior*> childBehaviors)
			: BehaviorSequence(childBehaviors) {}
		virtual ~BehaviorPartialSequence() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		unsigned int m_CurrentBehaviorIndex = 0;
	};


	//--- PARALLEL NODE ---
	class BehaviorParallel : public IBehavior {
	public:
		BehaviorParallel(std::vector<IBehavior*> children, size_t minSuccess, size_t minFailure)
			: m_children(children), m_minSuccess(minSuccess), m_minFailure(minFailure) {}

		virtual ~BehaviorParallel() {}

		virtual BehaviorState Execute(Blackboard* blackboard) override;

	private:
		std::vector<IBehavior*> m_children;
		size_t m_minSuccess;
		size_t m_minFailure;
	};

#pragma endregion

	class BehaviorConditional : public IBehavior
	{
	public:
		explicit BehaviorConditional(std::function<bool(Blackboard*)> fp) : m_fpConditional(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};


	template<typename T>
	class TBehaviorConditional : public IBehavior
	{
	public:
		explicit TBehaviorConditional(std::function<bool(Blackboard*, T)> fp, std::function<T(Blackboard*)> objFunction) 
			: m_fpConditionalTemplated(fp), m_ObjFunc(objFunction) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override
		{
				if (m_fpConditionalTemplated == nullptr)
					return BehaviorState::Failure;
			
				switch ( m_fpConditionalTemplated(pBlackBoard, m_ObjFunc(pBlackBoard)) )
				{
				case true:
					m_CurrentState = BehaviorState::Success;
					return m_CurrentState;
				case false:
					m_CurrentState = m_CurrentState = BehaviorState::Failure;
					return m_CurrentState;
				}
			
			
				return BehaviorState::Failure;
		};

	private:
		std::function<bool(Blackboard*, T)> m_fpConditionalTemplated = nullptr;
		std::function<T(Blackboard*)> m_ObjFunc;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE ACTION (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorAction : public IBehavior
	{
	public:
		explicit BehaviorAction(std::function<BehaviorState(Blackboard*)> fp) : m_fpAction(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<BehaviorState(Blackboard*)> m_fpAction = nullptr;
	};

	template<typename T>
	class TBehaviorAction : public IBehavior
	{
	public:
		explicit TBehaviorAction(std::function<BehaviorState(Blackboard*, T)> fp, std::function<T(Blackboard*)> objFunction)
			: m_fpAction(fp), m_ObjFunc(objFunction) {}
		virtual BehaviorState Execute(Blackboard* pBlackboard) override
		{
			if (m_fpAction == nullptr)
				return BehaviorState::Failure;

			m_CurrentState = m_fpAction(pBlackboard, m_ObjFunc(pBlackboard)); // Call m_ObjFunc to get the value of T
			return m_CurrentState;
		}

	private:
		std::function<BehaviorState(Blackboard*, T)> m_fpAction = nullptr;
		std::function<T(Blackboard*)> m_ObjFunc;
	};




	//-----------------------------------------------------------------
// DECORATORS (IBehavior)
//-----------------------------------------------------------------

	class NotDecorator : public BehaviorConditional
	{
	public:
		explicit NotDecorator(std::function<bool(Blackboard*)> fp) : BehaviorConditional(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override
		{
			// Execute the child behavior
			BehaviorConditional::Execute(pBlackBoard);

			// Invert the child's output and return it
			if (GetCurrentState() == BehaviorState::Success)
			{
				m_CurrentState = BehaviorState::Failure;
			}
			else
			{
				m_CurrentState = BehaviorState::Success;
			}

			return m_CurrentState;
		}
	};


	//(std::function<bool(Blackboard*, T)> fp, std::function<T(Blackboard*)> objFunction) 
	//: m_fpConditionalTemplated(fp), m_ObjFunc(objFunction) {}
	template<typename T>
	class TNotDecorator : public TBehaviorConditional<T>
	{
	public:
		explicit TNotDecorator(std::function<bool(Blackboard*, T)> fp, std::function<T(Blackboard*)> objFunction)
			: TBehaviorConditional<T>(fp, objFunction) {} // Update base class here
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override
		{
			// Execute the child behavior
			TBehaviorConditional<T>::Execute(pBlackBoard); // Update base class here

			// Invert the child's output and return it
			if (GetCurrentState() == BehaviorState::Success)
			{
				m_CurrentState = BehaviorState::Failure;
			}
			else
			{
				m_CurrentState = BehaviorState::Success;
			}

			return m_CurrentState;
		}
	};

	class BehaviorWhile : public IBehavior
	{
	public:
		BehaviorWhile(IBehavior* conditional, IBehavior* action) : m_pAction(action), m_pConditional(conditional) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override
		{
			if (m_pConditional->Execute(pBlackBoard) == BehaviorState::Success)
			{
				if (m_pAction->Execute(pBlackBoard) == BehaviorState::Failure)
				{
					return BehaviorState::Failure;
				}
				
				return BehaviorState::Running;
			}
			else
			{
				return BehaviorState::Success;
			}
		}

	private:
		IBehavior* m_pAction = nullptr;
		IBehavior* m_pConditional = nullptr;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE (BASE)
	//-----------------------------------------------------------------
	class BehaviorTree final : public Elite::IDecisionMaking
	{
	public:
		explicit BehaviorTree(Blackboard* pBlackBoard, IBehavior* pRootBehavior)
			: m_pBlackBoard(pBlackBoard), m_pRootBehavior(pRootBehavior) {};
		~BehaviorTree()
		{
			SAFE_DELETE(m_pRootBehavior);
			SAFE_DELETE(m_pBlackBoard); //Takes ownership of passed blackboard!
		};

		virtual void Update(float deltaTime) override
		{
			if (m_pRootBehavior == nullptr)
			{
				m_CurrentState = BehaviorState::Failure;
				return;
			}
				
			m_CurrentState = m_pRootBehavior->Execute(m_pBlackBoard);
		}
		Blackboard* GetBlackboard() const
		{ return m_pBlackBoard;	}

	private:
		BehaviorState m_CurrentState = BehaviorState::Failure;
		Blackboard* m_pBlackBoard = nullptr;
		IBehavior* m_pRootBehavior = nullptr;
	};




}
#endif