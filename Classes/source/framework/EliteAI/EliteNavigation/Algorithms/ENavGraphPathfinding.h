#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Elite::Vector2> FindPath(Elite::Vector2 startPos, Elite::Vector2 endPos, Elite::NavGraph* pNavGraph, std::vector<Elite::Vector2>& debugNodePositions, std::vector<Elite::Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Elite::Vector2> finalPath{};

			//Get the start and endTriangle

			const Triangle* pFirstTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			const Triangle* pLastTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);
			if (!(pFirstTriangle && pLastTriangle)) return finalPath;
			if (pFirstTriangle == pLastTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto graph = pNavGraph->Clone();

			//Create extra node for the Start Node (Agent's position
			NavGraphNode* pStartNode = new NavGraphNode{ graph->GetNextFreeNodeIndex(), -1, startPos };
			graph->AddNode(pStartNode);
			//Create extra node for the endNode
			NavGraphNode* pEndNode = new NavGraphNode{ graph->GetNextFreeNodeIndex(), -1, endPos };
			graph->AddNode(pEndNode);

			for (size_t i = 0; i < pFirstTriangle->metaData.IndexLines.size(); i++)
			{
				// Add connection from start node to the edges that have a node
				for (NavGraphNode* pNode : graph->GetAllActiveNodes())
				{
					if (pNode->GetLineIndex() == pFirstTriangle->metaData.IndexLines[i])
					{
						graph->AddConnection(new GraphConnection2D{ pStartNode->GetIndex(), pNode->GetIndex(), Elite::Distance(pStartNode->GetPosition(), pNode->GetPosition()) });
					}
				}
			}

			for (size_t i = 0; i < pLastTriangle->metaData.IndexLines.size(); i++)
			{
				// Add connection from start node to the edges that have a node
				for (NavGraphNode* pNode : graph->GetAllActiveNodes())
				{
					if (pNode->GetLineIndex() == pLastTriangle->metaData.IndexLines[i])
					{
						graph->AddConnection(new GraphConnection2D{ pEndNode->GetIndex(), pNode->GetIndex(), Elite::Distance(pEndNode->GetPosition(), pNode->GetPosition()) });
					}
				}
			}

			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(graph.get(), HeuristicFunctions::Euclidean);

			auto nodePath = pathfinder.FindPath(pStartNode, pEndNode);

			// Add node positions to path
			/*for (auto pNode : nodePath) 
			{
				finalPath.push_back(pNode->GetPosition());
			}*/


			//OPTIONAL BUT ADVICED: Debug Visualisation
			debugNodePositions.clear();
			for (NavGraphNode* pNode : nodePath) debugNodePositions.push_back(pNode->GetPosition());

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals = SSFA::FindPortals(nodePath, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(portals);
			debugPortals = portals;


			return finalPath;
		}
	};
}
