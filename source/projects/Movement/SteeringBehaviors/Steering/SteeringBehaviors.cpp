//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

using namespace Elite;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	pAgent->SetAutoOrient(true);

	const float arrivalRadius{ 0.001f };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	const float distance = steering.LinearVelocity.Magnitude();
	if (distance < arrivalRadius) return SteeringOutput(ZeroVector2, 0.f, false);

	steering.IsValid = true;

	if (pAgent->CanRenderBehavior()) DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, {0,1,0,1});

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	pAgent->SetAutoOrient(true);

	float distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);
	if (distanceToTarget > m_FleeRadius) 
	{
		//return SteeringOutput(ZeroVector2, 0.f, false);
	}

	auto steering = Seek::CalculateSteering(deltaT, pAgent);
	steering.LinearVelocity *= -1.0f;
	steering.IsValid = true;

	if (pAgent->CanRenderBehavior()) DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0,1,0,1 });

	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	pAgent->SetAutoOrient(true);

	//Variables
	const float arrivalRadius{ 1.f };
	const float slowRadius{ 3.f };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();

	float distance = steering.LinearVelocity.Magnitude();

	steering.LinearVelocity.Normalize();

	if (distance < slowRadius) steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * distance / (slowRadius - arrivalRadius);
	else steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (distance < arrivalRadius) steering.LinearVelocity = ZeroVector2;

	if (pAgent->CanRenderBehavior()) DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0,1,0,1 });

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	pAgent->SetAutoOrient(false);

	Vector2 toTarget = m_Target.Position - pAgent->GetPosition();
	float angle = std::atan2f(toTarget.y, toTarget.x);
	float currAngle = pAgent->GetRotation();
	
	float diffAngle = angle - currAngle;

	//Offset
	diffAngle += b2_pi/2;

	// Variables 
	float rotateSpeed{ 10000.f };
	float rangeToStopCheckking{ 0.1f };

	steering.LinearVelocity = ZeroVector2;
	if (diffAngle > 0 + rangeToStopCheckking) steering.AngularVelocity = deltaT * rotateSpeed;
	else if (diffAngle < 0 - rangeToStopCheckking) steering.AngularVelocity = deltaT * -rotateSpeed;

	if (pAgent->CanRenderBehavior()) DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0,1,0,1 });

	return steering;
}



//WANDER
//****
void Wander::SetWanderOffset(float offset)
{
	m_WanderOffset = offset;
}

void Wander::SetWanderRadius(float radius)
{
	m_Radius = radius;
}

void Wander::SetMaxAngleChange(float rad)
{
	m_MaxAngleChange = rad;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);

	Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	float currDistance{ toTarget.Magnitude() };

	auto circleCenter{ pAgent->GetPosition() + (pAgent->GetDirection().GetNormalized() * m_WanderOffset) };
	auto angleOffset = randomFloat(-m_MaxAngleChange, m_MaxAngleChange);
	auto newAngle = m_WanderAngle + angleOffset;
	m_WanderAngle = newAngle;
	
	m_Target = circleCenter + Vector2{ cos(newAngle) * m_Radius, sin(newAngle) * m_Radius };
		

	// Move towards target
	auto steering = Seek::CalculateSteering(deltaT, pAgent);

	if (pAgent->CanRenderBehavior()) 
	{
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 0,1,1,1 }, 0.f);
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, 0.5f, { 1,1,1,1 }, 0.f);
	}

	return steering;
}


//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	float distance = Distance(m_Target.Position, pAgent->GetPosition());
	if (distance < m_EvadeRange) 
	{
		auto steering = Pursuit::CalculateSteering(deltaT, pAgent);
		steering.LinearVelocity *= -1.f;
		steering.IsValid = true;
		return steering;
	}
	auto steering = SteeringOutput{};
	steering.LinearVelocity = ZeroVector2;
	steering.IsValid = false;
	return steering;
	
}

float Evade::GetEvadeRange() const
{
	return m_EvadeRange;
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 distance = m_Target.Position - pAgent->GetPosition();
	float normalizeMultiplier = distance.Normalize() / pAgent->GetMaxAngularSpeed();

	m_Target.Position = m_Target.Position + m_Target.LinearVelocity.GetNormalized() * normalizeMultiplier;

	auto steering = Seek::CalculateSteering(deltaT, pAgent);

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, 0.5f, { 1,1,1,1 }, 0.f);
	}

	return steering;
}

