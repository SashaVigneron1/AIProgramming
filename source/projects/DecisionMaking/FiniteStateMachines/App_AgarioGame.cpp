#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"


//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace Elite;
App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pCustomAgent);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pTransitions)
	{
		SAFE_DELETE(t);
	}
}

void App_AgarioGame::Start()
{
	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	// States
	WanderState* pWanderState = new WanderState();
	m_pStates.push_back(pWanderState);

	//Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize*(2.0f / 3), m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		// ToDo: Add Decision Making Things
		Elite::Blackboard* pBlackBoard = new Elite::Blackboard();
		pBlackBoard->AddData("Agent", newAgent);
		FiniteStateMachine* pFSM = new FiniteStateMachine(pWanderState, pBlackBoard);

		//pFSM->AddTransition();

		// Any kind of decision making structure that inherits from IDecisionMaking
		newAgent->SetDecisionMaking(pFSM);

		m_pAgentVec.push_back(newAgent);
	}

	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize * (2.0f / 3), m_TrimWorldSize * (2.0f / 3));
	Color customColor = Color{ 0.0f, 1.0f, 0.0f };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//1. Create and add the necessary blackboard data
	Elite::Blackboard* pBlackBoard = new Elite::Blackboard();
	pBlackBoard->AddData("Agent", m_pCustomAgent);

	//2. Create the different agent states
	SeekFoodState* pSeekFoodState = new SeekFoodState();
	pSeekFoodState->SetApp(this);

	AvoidBigAgentState* pAvoidBigAgentState = new AvoidBigAgentState();
	pAvoidBigAgentState->SetApp(this);

	ChaseSmallerAgentState* pChaseSmallerAgentState = new ChaseSmallerAgentState();
	pChaseSmallerAgentState->SetApp(this);

	MoveAwayFromBordersState* pMoveAwayFromBorderState = new MoveAwayFromBordersState();
	pMoveAwayFromBorderState->SetApp(this);

	m_pStates.push_back(pSeekFoodState);
	m_pStates.push_back(pAvoidBigAgentState);
	m_pStates.push_back(pChaseSmallerAgentState);
	m_pStates.push_back(pMoveAwayFromBorderState);

	//3. Create the transitions beetween those states
	SeekFoodToEvadeTransition* pSeekFoodToEvadeTransition = new SeekFoodToEvadeTransition();
	pSeekFoodToEvadeTransition->SetParameters(this, m_pCustomAgent);
	EvadeToSeekFoodTransition* pEvadeToSeekFoodTransition = new EvadeToSeekFoodTransition();
	pEvadeToSeekFoodTransition->SetParameters(this, m_pCustomAgent);


	ChaseToSeekFoodTransition* pChaseToSeekFoodTransition = new ChaseToSeekFoodTransition();
	pChaseToSeekFoodTransition->SetParameters(this, m_pCustomAgent);
	SeekFoodToChaseTransition* pSeekFoodToChaseTransition = new SeekFoodToChaseTransition();
	pSeekFoodToChaseTransition->SetParameters(this, m_pCustomAgent);

	ChaseToEvadeTransition* pChaseToEvadeTransition = new ChaseToEvadeTransition();
	pChaseToEvadeTransition->SetParameters(this, m_pCustomAgent);

	ToMoveAwayFromBorderTransition* pToMoveAwayFromBorderTransition = new ToMoveAwayFromBorderTransition();
	pToMoveAwayFromBorderTransition->SetParameters(this, m_pCustomAgent);

	m_pTransitions.push_back(pSeekFoodToEvadeTransition);
	m_pTransitions.push_back(pEvadeToSeekFoodTransition);
	m_pTransitions.push_back(pChaseToSeekFoodTransition);
	m_pTransitions.push_back(pSeekFoodToChaseTransition);
	m_pTransitions.push_back(pChaseToEvadeTransition);
	m_pTransitions.push_back(pToMoveAwayFromBorderTransition);

	//4. Create the finite state machine with a starting state and the blackboard
	FiniteStateMachine* pFSM = new FiniteStateMachine(pSeekFoodState, pBlackBoard);


	//5. Add the transitions for the states to the state machine
	// stateMachine->AddTransition(startState, toState, transition)
	// startState: active state for which the transition will be checked
	// transition: if the transition ToTransition function returns true => transition will fire and move to the toState
	// toState: end state where the agent will move to if the transition fires
	pFSM->AddTransition(pSeekFoodState, pAvoidBigAgentState, pSeekFoodToEvadeTransition);
	pFSM->AddTransition(pAvoidBigAgentState, pSeekFoodState, pEvadeToSeekFoodTransition);
	pFSM->AddTransition(pSeekFoodState, pChaseSmallerAgentState, pSeekFoodToChaseTransition);
	pFSM->AddTransition(pChaseSmallerAgentState, pSeekFoodState, pChaseToSeekFoodTransition);
	pFSM->AddTransition(pChaseSmallerAgentState, pAvoidBigAgentState, pChaseToEvadeTransition);

	pFSM->AddTransition(pSeekFoodState, pMoveAwayFromBorderState, pToMoveAwayFromBorderTransition);
	pFSM->AddTransition(pAvoidBigAgentState, pMoveAwayFromBorderState, pToMoveAwayFromBorderTransition);
	//pFSM->AddTransition(pChaseSmallerAgentState, pMoveAwayFromBorderState, pToMoveAwayFromBorderTransition);

	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pCustomAgent->SetDecisionMaking(pFSM);

}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pCustomAgent->CanBeDestroyed())
	{
		m_GameOver = true;
		return;
	}

	//Update the custom agent
	m_pCustomAgent->Update(deltaTime);
	m_pCustomAgent->LimitToWorld(m_TrimWorldSize);
	//m_pCustomAgent->TrimToWorld(m_TrimWorldSize);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(-m_TrimWorldSize, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	std::vector<Elite::Vector2> points =
	{
		{ -m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, -m_TrimWorldSize },
		{ -m_TrimWorldSize, -m_TrimWorldSize }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pCustomAgent->Render(deltaTime);
}

std::vector<AgarioFood*> App_AgarioGame::GetFoodVector() const
{
	return m_pFoodVec;
}

std::vector<AgarioAgent*> App_AgarioGame::GetAgents() const
{
	return m_pAgentVec;
}

float App_AgarioGame::GetWorldSize() const
{
	return m_TrimWorldSize;
}

Blackboard* App_AgarioGame::CreateBlackboard(AgarioAgent* a)
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("AgentsVec", &m_pAgentVec);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("WorldSize", m_TrimWorldSize);
	pBlackboard->AddData("FoodTarget", static_cast<AgarioFood*>(nullptr)); // Needs the cast for the type
	pBlackboard->AddData("AgentFleeTarget", static_cast<AgarioAgent*>(nullptr)); // Needs the cast for the type
	pBlackboard->AddData("Time", 0.0f); 

	return pBlackboard;
}

void App_AgarioGame::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
