#pragma once

#include "EBlackboard.h"
#include "EliteMath/EMath.h"

class IExamInterface;

class ISteeringBehaviour
{
public:
	ISteeringBehaviour() = default;
	virtual ~ISteeringBehaviour() = default;

	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) = 0;

	//Seek Functions
	void SetTarget(const Elite::Vector2& target) { m_Target = target; }
	Elite::Vector2 GetTarget() { return m_Target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehaviour, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	Elite::Vector2 m_Target;
};

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehaviour
{
public:
	Seek() = default;
	virtual ~Seek() = default;


	//Seek Behaviour
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;
protected:
};

///////////////////////////////////////
//SEEK TARGET FLEE ENEMY
//****
class SeekTargetFleeEnemy : public ISteeringBehaviour
{
public:
	SeekTargetFleeEnemy() = default;
	virtual ~SeekTargetFleeEnemy() = default;

	void SetFleeTarget(const Elite::Vector2& target) { m_FleeTarget = target; }

	//Seek Behaviour
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;
protected:
	Elite::Vector2 m_FleeTarget;

};

/////////////////////////
//FLEE
//****
class Flee : public Seek
{
public:
	Flee(Elite::Blackboard* pBlackboard);
	virtual ~Flee() = default;

	//Seek Behavior
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;

protected:
	float m_FleeRadius = 100.f;
	Elite::Blackboard* m_pBlackboard;
	Elite::Vector2 m_FleeTarget;
};


/////////////////////////
//WANDER
//****
class Wander : public Seek
{
public:
	Wander(Elite::Blackboard* pBlackboard);
	virtual ~Wander() = default;

	//Seek Behavior
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;

protected:
	Elite::Blackboard* m_pBlackboard;
	float m_WanderOffset = 12.f;
	float m_Radius = 8.f;
	float m_MaxAngleChange = Elite::ToRadians(45.f);
	float m_WanderAngle = 180;
};


/////////////////////////
//FACE
//****
class FaceFlee : public ISteeringBehaviour
{
public:
	FaceFlee() = default;
	virtual ~FaceFlee() = default;

	//Seek Behavior
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;
};

/////////////////////////
//Look Behind
//****
class LookBehind : public ISteeringBehaviour
{
public:
	LookBehind() = default;
	virtual ~LookBehind() = default;

	//Seek Behavior
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;
};


/////////////////////////
//PURSUIT
//****
class Pursuit : public Seek
{
public:
	Pursuit(Elite::Blackboard* pBlackboard);
	virtual ~Pursuit() = default;

	void SetEnemy(const EnemyInfo& info) { m_EnemyInfo = info; }

	//Seek Behavior
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;

protected:
	EnemyInfo m_EnemyInfo;
	Elite::Blackboard* m_pBlackboard;
};

/////////////////////////
//EVADE
//****
class Evade : public Pursuit
{
public:
	Evade(Elite::Blackboard* pBlackboard);
	virtual ~Evade() = default;

	void SetEnemy(const EnemyInfo& info) { m_EnemyInfo = info; }
	//Seek Behavior
	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, const AgentInfo& pAgent) override;

private:
};

