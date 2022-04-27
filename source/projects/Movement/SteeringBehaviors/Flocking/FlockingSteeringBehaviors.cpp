#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	m_Target = m_pFlock->GetAverageNeighborPos();
	SteeringOutput steering = Seek::CalculateSteering(deltaT, pAgent);
	return steering;
}


//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	std::vector<SteeringAgent*> pNeighbors = m_pFlock->GetNeighbors();

	SteeringOutput steering = SteeringOutput{};

	if (pNeighbors.size()) 
	{
		for (SteeringAgent* pNeighbor : pNeighbors)
		{
			float distance = Elite::Distance(pNeighbor->GetPosition(), pAgent->GetPosition());
			if (distance > 1) steering.LinearVelocity -= pNeighbor->GetLinearVelocity() / distance;
		}

		steering.LinearVelocity = steering.LinearVelocity.GetNormalized() * pAgent->GetMaxLinearSpeed() * deltaT;

		return steering;

	}
	return SteeringOutput(Elite::ZeroVector2, 0.f, false);
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	return steering;
}