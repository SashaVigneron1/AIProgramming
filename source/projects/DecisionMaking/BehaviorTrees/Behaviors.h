/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	pBlackboard->GetData("Agent", pAgent);

	if (!pAgent) return Elite::BehaviorState::Failure;

	pAgent->SetToWander();
	return Elite::BehaviorState::Success;
}
Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	pBlackboard->GetData("Agent", pAgent);
	Elite::Vector2 target = Elite::Vector2();
	pBlackboard->GetData("Target", target);

	if (!pAgent) return Elite::BehaviorState::Failure;

	pAgent->SetToSeek(target);
	return Elite::BehaviorState::Success;
}

Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	pBlackboard->GetData("Agent", pAgent);
	Elite::Vector2 target = Elite::Vector2();
	pBlackboard->GetData("Target", target);

	if (!pAgent) return Elite::BehaviorState::Failure;

	pAgent->SetToFlee(target);
	return Elite::BehaviorState::Success;
}

//-----------------------------------------------------------------
// Condiiotns
//-----------------------------------------------------------------
bool IsFoodCloseBy(Elite::Blackboard* pBlackboard) 
{
	AgarioAgent* pAgent{ nullptr };
	pBlackboard->GetData("Agent", pAgent);
	std::vector<AgarioFood*>* foodVec = nullptr;
	pBlackboard->GetData("FoodVec", foodVec);

	if (!foodVec || !pAgent) return Elite::BehaviorState::Failure;

	if (foodVec->size() < 1) return false;

	// 1. Get closest food
	AgarioFood* pClosestFood = (*foodVec)[0];
	float closestDistance{ DistanceSquared(pAgent->GetPosition(), pClosestFood->GetPosition()) };

	for (size_t i = 1; i < (*foodVec).size(); i++)
	{
		float currDistance{ DistanceSquared(pAgent->GetPosition(), (*foodVec)[i]->GetPosition()) };
		if (currDistance < closestDistance) 
		{
			pClosestFood = (*foodVec)[i];
			closestDistance = currDistance;
		}
	}

	// 2. Check if closest food is in a certain range
	const float maxRange{20.f};
	if (closestDistance <= maxRange * maxRange)
	{
		pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
		return true;
	}
	return false;
}

bool IsBigAgentCloseBy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	pBlackboard->GetData("Agent", pAgent);
	std::vector<AgarioAgent*>* agentVec = nullptr;
	pBlackboard->GetData("AgentsVec", agentVec);

	if (!agentVec || !pAgent) return Elite::BehaviorState::Failure;

	if (agentVec->size() < 1) return false;

	Elite::Vector2 targetPos;
	const float minRange = 50.f;
	float minRadius = pAgent->GetRadius() + 1;

	for (size_t i = 0; i < (*agentVec).size(); i++)
	{
		if ((*agentVec)[i]->GetRadius() >= minRadius)
		{
			float currDistance = Elite::Distance(pAgent->GetPosition(), (*agentVec)[i]->GetPosition());
			if (currDistance <= minRange)
			{
				Elite::Vector2 target{ (*agentVec)[i]->GetPosition() };
				pBlackboard->ChangeData("Target", (*agentVec)[i]->GetPosition());
				return true;
			}
		}
	}
	return false;
}

bool IsSmallAgentCloseBy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	pBlackboard->GetData("Agent", pAgent);
	std::vector<AgarioAgent*>* agentVec = nullptr;
	pBlackboard->GetData("AgentsVec", agentVec);

	if (!agentVec || !pAgent) return Elite::BehaviorState::Failure;

	if (agentVec->size() < 1) return false;

	Elite::Vector2 targetPos;
	const float maxRange = 50.f;
	float maxRadius = pAgent->GetRadius() - 1;

	for (size_t i = 0; i < (*agentVec).size(); i++)
	{
		if ((*agentVec)[i]->GetRadius() <= maxRadius)
		{
			float currDistance = Elite::Distance(pAgent->GetPosition(), (*agentVec)[i]->GetPosition());
			if (currDistance <= maxRange)
			{
				Elite::Vector2 target{ (*agentVec)[i]->GetPosition() };
				pBlackboard->ChangeData("Target", (*agentVec)[i]->GetPosition());
				return true;
			}
		}
	}
	return false;
}








#endif