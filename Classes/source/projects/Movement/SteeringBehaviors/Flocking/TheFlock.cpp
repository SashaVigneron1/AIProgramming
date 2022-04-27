#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{

	// Partitioning
	m_pCellSpace = new CellSpace{ m_WorldSize, m_WorldSize, 25, 25, m_FlockSize };

	// All Agents
	m_pSeparationBehavior = new Separation{this};
	m_pCohesionBehavior = new Cohesion{this};
	m_pVelMatchBehavior = new VelocityMatch{this};
	m_pSeekBehavior = new Seek{};
	m_pWanderBehavior = new Wander{};

	m_pEvadeBehavior = new Evade{};

	vector<BlendedSteering::WeightedBehavior> weightedSteeringBehaviors;
	weightedSteeringBehaviors.push_back({ m_pSeparationBehavior, 0.5f });
	weightedSteeringBehaviors.push_back({ m_pCohesionBehavior, 0.5f });
	weightedSteeringBehaviors.push_back({ m_pVelMatchBehavior, 0.5f });
	weightedSteeringBehaviors.push_back({ m_pSeekBehavior, 0.5f });
	weightedSteeringBehaviors.push_back({ m_pWanderBehavior, 0.5f });
	m_pBlendedSteering = new BlendedSteering({ weightedSteeringBehaviors });
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });

	// Agent To Evade
	m_pAgentToEvade->SetSteeringBehavior(m_pWanderBehavior);
	m_pAgentToEvade->SetBodyColor(Elite::Color(1, 0, 0));
	m_pAgentToEvade->SetMaxLinearSpeed(55.f);
	m_pAgentToEvade->SetAutoOrient(true);

	for (int i = 0; i < m_FlockSize; i++)
	{
		SteeringAgent* pAgent = new SteeringAgent();
		
		pAgent->SetSteeringBehavior(m_pPrioritySteering);
		pAgent->SetMass(1.0f);
		pAgent->SetMaxAngularSpeed(25.0f * 16.f);
		pAgent->SetMaxLinearSpeed(55.f);
		pAgent->SetAutoOrient(true);
		Elite::Vector2 agentPos{ Elite::randomFloat(m_WorldSize), Elite::randomFloat(m_WorldSize) };
		pAgent->SetPosition(agentPos);

		m_Agents.push_back(pAgent);
		m_OldAgentPos.push_back(agentPos);
		m_pCellSpace->AddAgent(pAgent);

		if (i == 0) m_pCellSpace->SetAgentToDebug(pAgent);
	}

	
}

Flock::~Flock()
{

	SAFE_DELETE(m_pCellSpace);

	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	for (int i = 0; i < m_FlockSize; i++)
	{
		SAFE_DELETE(m_Agents[i]);
	}
	m_Agents.clear();
}

void Flock::Update(float deltaT)
{
	for (size_t i = 0; i < m_Agents.size(); i++)
	{
		if (m_TrimWorld)
		{
			m_Agents[i]->TrimToWorld(Elite::Vector2(0, 0), Elite::Vector2(m_WorldSize, m_WorldSize));
		}

		m_pCellSpace->UpdateAgentCell(m_Agents[i], m_OldAgentPos[i]);

		RegisterNeighbors(m_Agents[i]);
		m_Agents[i]->Update(deltaT);
		m_OldAgentPos[i] = m_Agents[i]->GetPosition();
		
	}

	// Evade Behaviour
	TargetData data = TargetData{};
	data.AngularVelocity = m_pAgentToEvade->GetAngularVelocity();
	data.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	data.Position = m_pAgentToEvade->GetPosition();

	// Agent To Evade
	m_pEvadeBehavior->SetTarget(data);
	m_pAgentToEvade->Update(deltaT);
	if (m_TrimWorld) m_pAgentToEvade->TrimToWorld(Elite::Vector2(0, 0), Elite::Vector2(m_WorldSize, m_WorldSize));

	
}

void Flock::Render(float deltaT)
{
	/*
	for (SteeringAgent* pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
	}
	*/
	m_pAgentToEvade->Render(deltaT);
	
	DebugRender();

}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
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

	ImGui::Text("Debugging");
	ImGui::Spacing();
	ImGui::Checkbox("Enable Debug Rendering", &m_DebugRendering);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Partitioning");
	ImGui::Spacing();
	ImGui::Checkbox("Enable Partitioning", &m_UsePartitioning);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelocityMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	
	if (m_UsePartitioning) 
	{
		m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);
		m_Neighbors = m_pCellSpace->GetNeighbors();
		m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
	}
	else 
	{
		m_Neighbors.clear();
		m_NrOfNeighbors = 0;
		for (SteeringAgent* pOtherAgent : m_Agents)
		{
			float distance{ Elite::Distance(pAgent->GetPosition(), pOtherAgent->GetPosition()) };

			if (distance < m_NeighborhoodRadius)
			{
				m_Neighbors.push_back(pOtherAgent);
				m_NrOfNeighbors++;
			}
		}
	}

	
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	float xSum{};
	float ySum{};

	float counter{};

	if (m_Neighbors.size()) 
	{
		for (SteeringAgent* pAgent : m_Neighbors)
		{
			xSum += pAgent->GetPosition().x;
			ySum += pAgent->GetPosition().y;
			counter++;
		}

		float averageX{ xSum / counter };
		float averageY{ ySum / counter };

		return Elite::Vector2{ averageX, averageY };
	}

	return Elite::Vector2{ 0,0 };

	
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	float xSum{};
	float ySum{};

	float counter{};

	if (m_Neighbors.size())
	{
		for (SteeringAgent* pAgent : m_Neighbors)
		{
			xSum += pAgent->GetLinearVelocity().x;
			ySum += pAgent->GetLinearVelocity().y;
			counter++;
		}

		float averageX{ xSum / counter };
		float averageY{ ySum / counter };

		return Elite::Vector2{ averageX, averageY };
	}

	return Elite::Vector2{ 0,0 };

}

void Flock::SetSeekTarget(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::DebugRender()
{

	if (m_DebugRendering) 
	{
		// Change color of first Agent
		m_Agents[0]->SetBodyColor(Elite::Color{ 0,0,1 });

		// Draw Neighbor Circle
		DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, Elite::Color{ 0,1,0 }, 0);

		// Draw Neighbors
		if (m_UsePartitioning) m_pCellSpace->RegisterNeighbors(m_Agents[0], m_NeighborhoodRadius);
		else RegisterNeighbors(m_Agents[0]);
		if (m_UsePartitioning) m_Neighbors = m_pCellSpace->GetNeighbors();

		for (SteeringAgent* pAgent : m_Neighbors)
		{
			DEBUGRENDERER2D->DrawSolidCircle(pAgent->GetPosition(), pAgent->GetRadius(), Elite::Vector2{ 0,0 }, Elite::Color{ 0,1,0 });
		}

		// Draw Evade Circle
		DEBUGRENDERER2D->DrawCircle(m_pAgentToEvade->GetPosition(), m_pEvadeBehavior->GetEvadeRange(), Elite::Color{ 0,0,1 }, 0);

		if (m_UsePartitioning) 
		{
			// Draw Cells
			if (m_UsePartitioning) m_pCellSpace->DebugRender(m_NeighborhoodRadius);
		}
	}
	else 
	{
		// Revert All Agent Colors
		for (SteeringAgent* pAgent : m_Agents) 
		{
			pAgent->SetBodyColor(Elite::Color{ 1,1,0 });
		}

	}
	
	

}
