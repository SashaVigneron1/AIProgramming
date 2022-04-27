#include "stdafx.h"
#include "StatesAndTransitions.h"


//-------------------------
// States
//-------------------------

// Wander
void WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	bool succes = pBlackboard->GetData("Agent", pAgent);
	if (!succes) return;

	pAgent->SetToWander();


}

// LookForFood
void SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	bool succes = pBlackboard->GetData("Agent", pAgent);
	if (!succes) return;

	m_pAgent = pAgent;

}

void SeekFoodState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	m_pAgent->SetToSeek(GetNearestFoodPosition());
	//std::cout << "Seeking Food\n";
}

void SeekFoodState::SetApp(App_AgarioGame* app)
{
	m_pAgarioGame = app;
}

Elite::Vector2 SeekFoodState::GetNearestFoodPosition() const
{
	Elite::Vector2 targetPos;
	float distance = FLT_MAX;

	if (m_pAgarioGame) 
	{

		for (AgarioFood* pFood : m_pAgarioGame->GetFoodVector())
		{
			float currDistance = Elite::Distance(m_pAgent->GetPosition(), pFood->GetPosition());
			if (currDistance < distance) 
			{
				distance = currDistance;
				targetPos = pFood->GetPosition();
			}
		}
	}
	return targetPos;
}

// Avoid Big Agents
void AvoidBigAgentState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	bool succes = pBlackboard->GetData("Agent", pAgent);
	if (!succes) return;

	m_pAgent = pAgent;
}

void AvoidBigAgentState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	m_pAgent->SetToFlee(GetNearestBiggerAgent());
	//std::cout << "Avoiding Big guy\n";

}

void AvoidBigAgentState::SetApp(App_AgarioGame* app)
{
	m_pAgarioGame = app;
}

Elite::Vector2 AvoidBigAgentState::GetNearestBiggerAgent() const
{
	Elite::Vector2 targetPos;
	float distance = FLT_MAX;
	float minRadius = m_pAgent->GetRadius() + 1;
	if (m_pAgarioGame)
	{

		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() >= minRadius) 
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < distance)
				{
					distance = currDistance;
					targetPos = pAgent->GetPosition();
				}
			}
		}
	}
	return targetPos;
}

// Chase Smaller Agents
void ChaseSmallerAgentState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	bool succes = pBlackboard->GetData("Agent", pAgent);
	if (!succes) return;

	m_pAgent = pAgent;
}

void ChaseSmallerAgentState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	m_pAgent->SetToSeek(GetNearestSmallerAgent());
	//std::cout << "Chasing small guy\n";
}

void ChaseSmallerAgentState::SetApp(App_AgarioGame* app)
{
	m_pAgarioGame = app;
}

Elite::Vector2 ChaseSmallerAgentState::GetNearestSmallerAgent() const
{
	Elite::Vector2 targetPos;
	float distance = FLT_MAX;
	float maxRadius = m_pAgent->GetRadius() - 2;
	if (m_pAgarioGame)
	{

		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() <= maxRadius)
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < distance)
				{
					distance = currDistance;
					targetPos = pAgent->GetPosition();
				}
			}
		}
	}
	return targetPos;
}

// MoveAwayFromBorders
void MoveAwayFromBordersState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	bool succes = pBlackboard->GetData("Agent", pAgent);
	if (!succes) return;

	m_pAgent = pAgent;
}

void MoveAwayFromBordersState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	Elite::Vector2 pos = m_pAgent->GetPosition();
	float worldSize = m_pAgarioGame->GetWorldSize();

	if (pos.x > 0) 
	{
		if (pos.y > 0) 
		{
			// TopRight Corner
			m_pAgent->SetToFlee(Elite::Vector2{ worldSize, worldSize });
		}
		else
		{
			// BottomRight Corner
			m_pAgent->SetToFlee(Elite::Vector2{ worldSize, -worldSize });
		}
	}
	else 
	{
		if (pos.y > 0)
		{
			// TopLeft Corner
			m_pAgent->SetToFlee(Elite::Vector2{ -worldSize, worldSize });
		}
		else
		{
			// BottomLeft Corner
			m_pAgent->SetToFlee(Elite::Vector2{ -worldSize, -worldSize });
		}
	}
}

void MoveAwayFromBordersState::SetApp(App_AgarioGame* app)
{
	m_pAgarioGame = app;
}

//-------------------------
// Transitions
//-------------------------

bool SeekFoodToEvadeTransition::ToTransition(Elite::Blackboard* pBlackboard) const
{
	const float minRange = 20.f;

	Elite::Vector2 targetPos;
	float minRadius = m_pAgent->GetRadius() + 2;

	if (m_pAgarioGame)
	{
		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() >= minRadius)
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < minRange)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void SeekFoodToEvadeTransition::SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent)
{
	m_pAgarioGame = pApp;
	m_pAgent = pAgent;
}

bool EvadeToSeekFoodTransition::ToTransition(Elite::Blackboard* pBlackboard) const
{
	const float minRange = 20.f;
	bool isBigGuyInRange{ false };

	Elite::Vector2 targetPos;
	float maxRadius = m_pAgent->GetRadius() + 2;

	if (m_pAgarioGame)
	{
		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() >= maxRadius)
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < minRange)
				{
					isBigGuyInRange = true;
				}
			}
		}
	}
	return !isBigGuyInRange;
}

void EvadeToSeekFoodTransition::SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent)
{
	m_pAgarioGame = pApp;
	m_pAgent = pAgent;
}

bool SeekFoodToChaseTransition::ToTransition(Elite::Blackboard* pBlackboard) const
{
	const float minRange = 20.f;

	bool isSmallGuyInRange{false};

	float maxRadius = m_pAgent->GetRadius() - 2;
	if (m_pAgarioGame)
	{

		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() <= maxRadius)
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < minRange)
				{
					isSmallGuyInRange = true;
					break;
				}
			}
		}
	}
	return isSmallGuyInRange;
}

void SeekFoodToChaseTransition::SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent)
{
	m_pAgarioGame = pApp;
	m_pAgent = pAgent;
}

bool ChaseToSeekFoodTransition::ToTransition(Elite::Blackboard* pBlackboard) const
{
	const float minRange = 20.f;

	bool isSmallGuyInRange{ false };

	float maxRadius = m_pAgent->GetRadius() - 2;
	if (m_pAgarioGame)
	{

		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() <= maxRadius)
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < minRange)
				{
					isSmallGuyInRange = true;
					break;
				}
			}
		}
	}
	return !isSmallGuyInRange;
}

void ChaseToSeekFoodTransition::SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent)
{

	m_pAgarioGame = pApp;
	m_pAgent = pAgent;
}

bool ChaseToEvadeTransition::ToTransition(Elite::Blackboard* pBlackboard) const
{
	const float minRange = 20.f;

	Elite::Vector2 targetPos;
	float minRadius = m_pAgent->GetRadius() + 2;

	if (m_pAgarioGame)
	{
		for (AgarioAgent* pAgent : m_pAgarioGame->GetAgents())
		{
			if (pAgent->GetRadius() >= minRadius)
			{
				float currDistance = Elite::Distance(m_pAgent->GetPosition(), pAgent->GetPosition());
				if (currDistance < minRange)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void ChaseToEvadeTransition::SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent)
{
	m_pAgarioGame = pApp;
	m_pAgent = pAgent;
}

bool ToMoveAwayFromBorderTransition::ToTransition(Elite::Blackboard* pBlackboard) const
{
	Elite::Vector2 pos = m_pAgent->GetPosition();
	float worldSize = m_pAgarioGame->GetWorldSize();
	const float maxRadius = 20.0f;
	bool shouldMoveAway = false;

	if (pos.x > 0)
	{
		if (pos.y > 0)
		{
			// TopRight Corner
			if (Elite::Distance(pos, Elite::Vector2{ worldSize, worldSize }) < maxRadius) 
			{
				shouldMoveAway = true;
			}
		}
		else
		{
			// BottomRight Corner
			if (Elite::Distance(pos, Elite::Vector2{ worldSize, -worldSize }) < maxRadius)
			{
				shouldMoveAway = true;
			}
		}
	}
	else
	{
		if (pos.y > 0)
		{
			// TopLeft Corner
			if (Elite::Distance(pos, Elite::Vector2{ -worldSize, worldSize }) < maxRadius)
			{
				shouldMoveAway = true;
			}
		}
		else
		{
			// BottomLeft Corner
			if (Elite::Distance(pos, Elite::Vector2{ -worldSize, -worldSize }) < maxRadius)
			{
				shouldMoveAway = true;
			}
		}
	}

	return shouldMoveAway;
}

void ToMoveAwayFromBorderTransition::SetParameters(App_AgarioGame* pApp, AgarioAgent* pAgent)
{
	m_pAgarioGame = pApp;
	m_pAgent = pAgent;
}
