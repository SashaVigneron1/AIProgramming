/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"
#include "App_AgarioGame.h"

//------------
//---STATES---
//------------

// A state that makes the agent go wander
class WanderState : public Elite::FSMState 
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
private:

};


class SeekFoodState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;

	void SetApp(App_AgarioGame* app);
private:
	Elite::Vector2 GetNearestFoodPosition() const;

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class AvoidBigAgentState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;

	void SetApp(App_AgarioGame* app);
private:
	Elite::Vector2 GetNearestBiggerAgent() const;

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class ChaseSmallerAgentState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;

	void SetApp(App_AgarioGame* app);
private:
	Elite::Vector2 GetNearestSmallerAgent() const;

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class MoveAwayFromBordersState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;

	//ToDo: Constructor
	void SetApp(App_AgarioGame* app);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};
//-----------------
//---TRANSITIONS---
//-----------------

class SeekFoodToEvadeTransition : public Elite::FSMTransition 
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;
	void SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class EvadeToSeekFoodTransition : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;

	void SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};

class SeekFoodToChaseTransition : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;

	void SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class ChaseToSeekFoodTransition : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;

	void SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class ChaseToEvadeTransition : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;

	void SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};


class ToMoveAwayFromBorderTransition : public Elite::FSMTransition
{
public:
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override;

	void SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent);
private:

	App_AgarioGame* m_pAgarioGame;
	AgarioAgent* m_pAgent;
};
#endif