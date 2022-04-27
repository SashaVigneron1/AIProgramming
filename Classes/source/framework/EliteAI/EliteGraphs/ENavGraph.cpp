#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (Elite::Line* pLine : m_pNavMeshPolygon->GetLines()) 
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(pLine->index).size() == 1)
			continue;

		float xAverage{ (pLine->p1.x + pLine->p2.x) / 2.f };
		float yAverage{ (pLine->p1.y + pLine->p2.y) / 2.f };
		NavGraphNode* pNode = new NavGraphNode{ GetNextFreeNodeIndex() , pLine->index, Vector2{xAverage, yAverage} };
		AddNode(pNode);
	}

	//2. Create connections now that every node is created


	for (Elite::Triangle* pTriangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<NavGraphNode*> pFoundNodes;
		for (NavGraphNode* pNode : m_Nodes)
		{
			if (std::find(pTriangle->metaData.IndexLines.begin(), pTriangle->metaData.IndexLines.end(), pNode->GetLineIndex()) != pTriangle->metaData.IndexLines.end())
			{
				pFoundNodes.push_back(pNode);
			}
		}

		if (pFoundNodes.size() == 2)
		{
			AddConnection(new GraphConnection2D{ pFoundNodes[0]->GetIndex(), pFoundNodes[1]->GetIndex() });
		}
		else if (pFoundNodes.size() == 3)
		{
			// GraphConnection2D(GetNextFreeNodeIndex()
			AddConnection(new GraphConnection2D{ pFoundNodes[0]->GetIndex(), pFoundNodes[1]->GetIndex() });
			AddConnection(new GraphConnection2D{ pFoundNodes[1]->GetIndex(), pFoundNodes[2]->GetIndex() });
			AddConnection(new GraphConnection2D{ pFoundNodes[2]->GetIndex(), pFoundNodes[0]->GetIndex() });
		}
	}
	
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();

}

