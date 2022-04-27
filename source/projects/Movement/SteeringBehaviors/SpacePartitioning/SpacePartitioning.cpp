#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

#include "framework/EliteMath/EMath.h"
using namespace Elite;


// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Cells{}
	, m_CellsToCheck{}
	, m_CellHeight{height / rows}
	, m_CellWidth{width / cols}
	, m_Neighbors()
	, m_NrOfNeighbors(0)
	, m_pAgentToDebug{ nullptr }
{
	m_Cells.reserve(rows * cols);
	m_CellsToCheck.resize(rows * cols);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			m_Cells.push_back(Cell{ j * m_CellWidth, i * m_CellHeight, m_CellWidth, m_CellHeight});
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	Vector2 agentPos = agent->GetPosition();
	
	m_Cells[PositionToIndex(agentPos)].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	Vector2 agentPos = agent->GetPosition();
	
	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldIndex != newIndex) 
	{
		// Remove from old cell
		for (size_t i{ 0 }; i < m_Cells[oldIndex].agents.size(); ++i)
		{
			if (agent == m_Cells[oldIndex].agents[i])
			{
				m_Cells[oldIndex].agents[i] = m_Cells[oldIndex].agents.back();
				m_Cells[oldIndex].agents.pop_back();
			}
		}

		// Add to new cell
		m_Cells[newIndex].agents.push_back(agent);
	}

}

void CellSpace::SetAgentToDebug(SteeringAgent* agent)
{
	m_pAgentToDebug = agent;
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{

	GetNeighborCells(agent, queryRadius);

	int index{ PositionToIndex(agent->GetPosition()) };

	if (index < (int)m_Cells.size() && index > 0) 
	{
		m_Neighbors.clear();
		m_NrOfNeighbors = 0;

		for (int i = 0; i < m_NrOfCellsToCheck; i++)
		{
			for (SteeringAgent* pOtherAgent : m_CellsToCheck[i]->agents)
			{
				float distance{ Elite::Distance(agent->GetPosition(), pOtherAgent->GetPosition()) };

				if (distance < queryRadius)
				{
					m_Neighbors.push_back(pOtherAgent);
					m_NrOfNeighbors++;
				}
			}
		}
	}
}

void CellSpace::GetNeighborCells(SteeringAgent* agent, float queryRadius)
{
	m_NrOfCellsToCheck = 0;

	Elite::EPhysicsCircleShape circle;
	circle.position = agent->GetPosition();
	circle.radius = queryRadius;

	float leftPos{ Clamp(agent->GetPosition().x - queryRadius, 0.0f, m_SpaceWidth) };
	float rightPos{ Clamp(agent->GetPosition().x + queryRadius, 0.0f, m_SpaceWidth - 1.f) };
	float upperPos{ Clamp(agent->GetPosition().y + queryRadius, 0.0f, m_SpaceHeight - 1.f) };
	float lowerPos{ Clamp(agent->GetPosition().y - queryRadius, 0.0f, m_SpaceHeight) };

	int leftIndex{ PositionToIndex(Vector2{leftPos, upperPos}) };
	int rightIndex{ PositionToIndex(Vector2{rightPos, upperPos}) };
	int upperIndex{ PositionToIndex(Vector2{leftPos, upperPos}) };
	int lowerIndex{ PositionToIndex(Vector2{leftPos, lowerPos}) };

	int nrCols = rightIndex - leftIndex + 1;
	int nrRows = ((upperIndex - lowerIndex ) / m_NrOfCols) + 1;

	int currRow{ lowerIndex / m_NrOfCols };

	for (int i = 0; i < nrRows; i++)
	{
		int currCol{ leftIndex % m_NrOfRows };
		for (int j = 0; j < nrCols; j++)
		{
			int currIndex{ currRow * m_NrOfCols + currCol };
			if (currIndex < (int)m_Cells.size())
			{
				if (IsOverlapping(circle, m_Cells[currIndex].boundingBox))
				{
					m_CellsToCheck[m_NrOfCellsToCheck] = &m_Cells[currIndex];
					m_NrOfCellsToCheck++;
				}
			}

			currCol++;
		}
		currRow++;
	}
}

const std::vector<SteeringAgent*>& CellSpace::GetNeighbors() const
{
	return m_Neighbors;
}

int CellSpace::GetNrOfNeighbors() const
{
	return m_NrOfNeighbors;
}

void CellSpace::DebugRender(float queryRadius)
{
	for (Cell cell : m_Cells)
	{
		// Draw Bounding Box
		Vector2 topLeft{ m_pAgentToDebug->GetPosition() + Vector2{-queryRadius, queryRadius} };
		Vector2 topRight{ m_pAgentToDebug->GetPosition() + Vector2{queryRadius, queryRadius} };
		Vector2 bottomRight{ m_pAgentToDebug->GetPosition() + Vector2{queryRadius, -queryRadius} };
		Vector2 bottomLeft{ m_pAgentToDebug->GetPosition() + Vector2{-queryRadius, -queryRadius} };
		DEBUGRENDERER2D->DrawSegment(topLeft, topRight, Elite::Color{ 0,0,1 });
		DEBUGRENDERER2D->DrawSegment(topRight, bottomRight, Elite::Color{ 0,0,1 });
		DEBUGRENDERER2D->DrawSegment(bottomRight, bottomLeft, Elite::Color{ 0,0,1 });
		DEBUGRENDERER2D->DrawSegment(bottomLeft, topLeft, Elite::Color{ 0,0,1 });

		// Draw Grid
		Elite::Polygon polygon{ cell.GetRectPoints() };
		DEBUGRENDERER2D->DrawPolygon(&polygon, Color{ 1,0,0 });

		// Draw nrAgents top left of cell
		float xOffset{ m_CellWidth / 10.0f };
		float yOffset{ m_CellHeight / 10.0f };
		Vector2 textPos = cell.GetRectPoints()[1] + Vector2{ xOffset, -yOffset };

		std::string text{ std::to_string(cell.agents.size()) };
		DEBUGRENDERER2D->DrawString(textPos, text.c_str());
	}


	// Get neighbor cells for debug agent
	GetNeighborCells(m_pAgentToDebug, queryRadius);

	for (int i = 0; i < m_NrOfCellsToCheck; i++)
	{
		Elite::Polygon polygon{ m_CellsToCheck[i]->GetRectPoints() };
		DEBUGRENDERER2D->DrawPolygon(&polygon, Color{ 1,1,0 });
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int x = (int)pos.x % (int)m_SpaceWidth;
	int y = (int)pos.y % (int)m_SpaceHeight;

	int col = x / (int)m_CellWidth;
	int row = y / (int)m_CellHeight;

	int index = row * m_NrOfCols + col;

	return index;
}

