/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviourTree.h: Implementation of a BehaviourTree and the components of a Behavior Tree
/*=============================================================================*/
#ifndef ELITE_BEHAVIOR_TREE
#define ELITE_BEHAVIOR_TREE

//--- Includes ---
#include "EBlackboard.h"
#include "EDecisionMaking.h"
#include "stdafx.h"

namespace Elite
{
	//-----------------------------------------------------------------
	// BEHAVIOR TREE HELPERS
	//-----------------------------------------------------------------
	enum BehaviourState
	{
		Failure,
		Success,
		Running
	};

	//-----------------------------------------------------------------
	// BEHAVIOR INTERFACES (BASE)
	//-----------------------------------------------------------------
	class IBehaviour
	{
	public:
		IBehaviour() = default;
		virtual ~IBehaviour() = default;
		virtual BehaviourState Execute(Blackboard* pBlackBoard) = 0;

	protected:
		BehaviourState m_CurrentState = Failure;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE COMPOSITES (IBehaviour)
	//-----------------------------------------------------------------
#pragma region COMPOSITES
	//--- COMPOSITE BASE ---
	class BehaviourComposite : public IBehaviour
	{
	public:
		explicit BehaviourComposite(std::vector<IBehaviour*> childrenBehaviors)
		{
			m_ChildrenBehaviors = childrenBehaviors;
		}
		virtual ~BehaviourComposite()
		{
			for (auto pb : m_ChildrenBehaviors)
				if (pb) delete pb;
			m_ChildrenBehaviors.clear();
		}

		virtual BehaviourState Execute(Blackboard* pBlackBoard) override = 0;

	protected:
		std::vector<IBehaviour*> m_ChildrenBehaviors = {};
	};

	//--- SELECTOR ---
	class BehaviourSelector : public BehaviourComposite
	{
	public:
		explicit BehaviourSelector(std::vector<IBehaviour*> childrenBehaviors) :
			BehaviourComposite(childrenBehaviors) {}
		virtual ~BehaviourSelector() = default;

		virtual BehaviourState Execute(Blackboard* pBlackBoard) override;
	};

	//--- SEQUENCE ---
	class BehaviourSequence : public BehaviourComposite
	{
	public:
		explicit BehaviourSequence(std::vector<IBehaviour*> childrenBehaviors) :
			BehaviourComposite(childrenBehaviors) {}
		virtual ~BehaviourSequence() = default;

		virtual BehaviourState Execute(Blackboard* pBlackBoard) override;
	};

	//--- PARTIAL SEQUENCE ---
	class BehaviourPartialSequence : public BehaviourSequence
	{
	public:
		explicit BehaviourPartialSequence(std::vector<IBehaviour*> childrenBehaviors)
			: BehaviourSequence(childrenBehaviors) {}
		virtual ~BehaviourPartialSequence() = default;

		virtual BehaviourState Execute(Blackboard* pBlackBoard) override;

	private:
		unsigned int m_CurrentBehaviorIndex = 0;
	};
#pragma endregion

	//-----------------------------------------------------------------
	// BEHAVIOR TREE CONDITIONAL (IBehaviour)
	//-----------------------------------------------------------------
	class BehaviourConditional : public IBehaviour
	{
	public:
		explicit BehaviourConditional(std::function<bool(Blackboard*)> fp) : m_fpConditional(fp) {}
		virtual BehaviourState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE ACTION (IBehaviour)
	//-----------------------------------------------------------------
	class BehaviourAction : public IBehaviour
	{
	public:
		explicit BehaviourAction(std::function<BehaviourState(Blackboard*)> fp) : m_fpAction(fp) {}
		virtual BehaviourState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<BehaviourState(Blackboard*)> m_fpAction = nullptr;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE (BASE)
	//-----------------------------------------------------------------
	class BehaviourTree final : public Elite::IDecisionMaking
	{
	public:
		explicit BehaviourTree(Blackboard* pBlackBoard, IBehaviour* pRootComposite)
			: m_pBlackBoard(pBlackBoard), m_pRootComposite(pRootComposite) {};
		~BehaviourTree()
		{
			if (m_pRootComposite) delete m_pRootComposite;
			if (m_pBlackBoard) delete m_pBlackBoard;//Takes ownership of passed blackboard!
		};

		virtual void Update(float deltaTime) override
		{
			if (m_pRootComposite == nullptr)
			{
				m_CurrentState = Failure;
				return;
			}

			m_CurrentState = m_pRootComposite->Execute(m_pBlackBoard);
		}
		Blackboard* GetBlackboard() const
		{
			return m_pBlackBoard;
		}

	private:
		BehaviourState m_CurrentState = Failure;
		Blackboard* m_pBlackBoard = nullptr;
		IBehaviour* m_pRootComposite = nullptr;
	};
}
#endif