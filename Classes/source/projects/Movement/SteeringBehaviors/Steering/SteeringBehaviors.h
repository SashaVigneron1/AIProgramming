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
#include "framework\EliteMath\EMatrix2x3.h"

class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
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
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

/////////////////////////
//FLEE
//****
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

protected:
	float m_FleeRadius = 10.f;
};

/////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

/////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

/////////////////////////
//WANDER
//****
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	void SetWanderOffset(float offset);
	void SetWanderRadius(float radius);
	void SetMaxAngleChange(float rad);

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

protected:
	float m_WanderOffset = 6.f;
	float m_Radius = 4.f;
	float m_MaxAngleChange = Elite::ToRadians(45.f);
	float m_WanderAngle = 0.f;
};


/////////////////////////
//PURSUIT
//****
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

protected:
};

/////////////////////////
//EVADE
//****
class Evade : public Pursuit
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	float GetEvadeRange() const;
private: 
	float m_EvadeRange{ 20.f };
};

#endif


