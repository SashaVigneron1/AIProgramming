#include "stdafx.h"
#include "SteeringBehaviours.h"

#include "IExamInterface.h"

SteeringPlugin_Output_Extended Seek::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	auto steering = SteeringPlugin_Output_Extended();

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = m_Target - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	steering.IsValid = true;


	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	return steering;
}


Flee::Flee(Elite::Blackboard* pBlackboard)
	: m_pBlackboard{ pBlackboard }
{
}

//FLEE
//****
SteeringPlugin_Output_Extended Flee::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	auto steering = SteeringPlugin_Output_Extended();

	Elite::Vector2 toTarget{ m_Target - agentInfo.Position };
	toTarget *= -1.f;
	toTarget.Normalize();
	toTarget *= 20.f;
	m_FleeTarget = agentInfo.Position + toTarget;
	
	IExamInterface* pInterface{ nullptr };
	m_pBlackboard->GetData("m_pInterface", pInterface);
	if (pInterface) 
	{
		m_FleeTarget = pInterface->NavMesh_GetClosestPathPoint(m_FleeTarget);
	}

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = m_FleeTarget - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	steering.IsValid = true;
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	return steering;
}

Wander::Wander(Elite::Blackboard* pBlackboard)
	: m_pBlackboard{ pBlackboard }
{
}

//WANDER
//****
SteeringPlugin_Output_Extended Wander::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	Elite::Vector2 circleCenter{ agentInfo.Position + (agentInfo.LinearVelocity.GetNormalized() * m_WanderOffset) };
	float angleOffset = Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);
	m_WanderAngle = m_WanderAngle + angleOffset;

	m_Target = circleCenter + Elite::Vector2{ cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius };

	IExamInterface* pInterface;
	m_pBlackboard->GetData("m_pInterface", pInterface);
	if (pInterface)
	{
		m_Target = pInterface->NavMesh_GetClosestPathPoint(m_Target);
	}

	// Move towards target
	auto steering = Seek::CalculateSteering(deltaT, agentInfo);
	steering.IsValid = true;

	return steering;
}

//FACE
//****
SteeringPlugin_Output_Extended FaceFlee::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extended steering = {};

	steering.AutoOrient = false;

	// Face
	Elite::Vector2 toTarget = m_Target - agentInfo.Position;
	float angle = std::atan2f(toTarget.y, toTarget.x);
	float currAngle = agentInfo.Orientation;

	angle += b2_pi / 2; // Offset
	if (angle > b2_pi) angle = -b2_pi + (angle - b2_pi);

	float diffAngle = angle - currAngle;
	if (diffAngle > b2_pi) diffAngle = -b2_pi + (diffAngle - b2_pi);
	if (diffAngle < -b2_pi) diffAngle = b2_pi - (diffAngle - b2_pi);


	// Variables 
	float rotateSpeed{ 10000.f };
	float rangeToStopCheckking{ 0.01f };

	// Flee
	steering.LinearVelocity = -toTarget;
	if (diffAngle > rangeToStopCheckking) steering.AngularVelocity = deltaT * rotateSpeed;
	else if (diffAngle < -rangeToStopCheckking) steering.AngularVelocity = deltaT * -rotateSpeed;
	else 
	{
		steering.IsValid = false;
		return steering;
	}
	steering.IsValid = false;
	return steering;
}

//LOOK BEHIND
//****
SteeringPlugin_Output_Extended LookBehind::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extended steering = {};

	steering.AutoOrient = false;

	// Set target behind player
	m_Target = agentInfo.Position - agentInfo.LinearVelocity;

	Elite::Vector2 toTarget = m_Target - agentInfo.Position;
	float angle = std::atan2f(toTarget.y, toTarget.x);
	float currAngle = agentInfo.Orientation;

	angle += b2_pi / 2; // Offset
	if (angle > b2_pi) angle = -b2_pi + (angle - b2_pi);

	float diffAngle = angle - currAngle;
	if (diffAngle > b2_pi) diffAngle = -b2_pi + (diffAngle - b2_pi);
	if (diffAngle < -b2_pi) diffAngle = b2_pi - (diffAngle - b2_pi);


	// Variables 
	float rotateSpeed{ 10000.f };
	float rangeToStopCheckking{ 0.1f };

	steering.LinearVelocity = Elite::Vector2(0.f, 0.f);
	if (diffAngle > rangeToStopCheckking) steering.AngularVelocity = deltaT * rotateSpeed;
	else if (diffAngle < -rangeToStopCheckking) steering.AngularVelocity = deltaT * -rotateSpeed;
	else
	{
		steering.IsValid = false;
		return steering;
	}
	steering.IsValid = false;
	return steering;
}


Pursuit::Pursuit(Elite::Blackboard* pBlackboard)
	: m_pBlackboard{ pBlackboard }
{
}

//PURSUIT
//****
SteeringPlugin_Output_Extended Pursuit::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	Elite::Vector2 distance = m_Target - agentInfo.Position;
	//float normalizeMultiplier = distance.Normalize() / agentInfo.MaxLinearSpeed;
	float distanceMultiplier = 10.f;

	//m_Target = m_EnemyInfo.Location + m_EnemyInfo.LinearVelocity.GetNormalized() * distanceMultiplier;

	IExamInterface* pInterface;
	m_pBlackboard->GetData("m_pInterface", pInterface);
	if (pInterface)
	{
		m_Target = pInterface->NavMesh_GetClosestPathPoint(m_Target);
	}

	auto steering = Seek::CalculateSteering(deltaT, agentInfo);

	steering.IsValid = true;
	return steering;
}

Evade::Evade(Elite::Blackboard* pBlackboard)
	: Pursuit(pBlackboard)
{
}

//EVADE
//****
SteeringPlugin_Output_Extended Evade::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	float distanceMultiplier = 50.f;

	//m_Target = m_EnemyInfo.Location + m_EnemyInfo.LinearVelocity.GetNormalized() * distanceMultiplier;
	IExamInterface* pInterface;
	m_pBlackboard->GetData("m_pInterface", pInterface);
	if (pInterface)
	{
		m_Target = pInterface->NavMesh_GetClosestPathPoint(m_Target);
	}

	auto steering = Pursuit::CalculateSteering(deltaT, agentInfo);
	steering.LinearVelocity *= -1.f;

	steering.IsValid = true;
	return steering;
}

SteeringPlugin_Output_Extended SeekTargetFleeEnemy::CalculateSteering(float deltaT, const AgentInfo& agentInfo)
{
	auto steering = SteeringPlugin_Output_Extended();

	// Seek
	Elite::Vector2 seekLinear{ m_Target - agentInfo.Position }; //Desired Velocity
	seekLinear.Normalize(); //Normalize Desired Velocity

	// Flee
	Elite::Vector2 fleeLinear{ agentInfo.Position - m_Target  }; //Desired Velocity
	fleeLinear.Normalize(); //Normalize Desired Velocity

	// Combined
	steering.LinearVelocity = ((seekLinear + (fleeLinear*0.25f)) / 2);
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	steering.IsValid = true;
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	return steering;
}
