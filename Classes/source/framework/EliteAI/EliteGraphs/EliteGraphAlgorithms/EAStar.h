#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		// Variables
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		// 1. Kickstart NodeRecord
		NodeRecord kickStartRecord;
		kickStartRecord.pNode = pStartNode;
		kickStartRecord.pConnection = nullptr;
		kickStartRecord.costSoFar = 0.0f;
		kickStartRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(kickStartRecord);

		while (!openList.empty())
		{
			auto it = std::min_element(openList.begin(), openList.end());
			currentRecord = *(it);
			openList.erase(it);

			// 2.B If endnode was found, stop
			if (currentRecord.pNode == pGoalNode)
			{
				break;
			}

			// 2.C Else, we get all the connections of the NodeRecord's node
			for (auto con : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(con->GetTo());
				float newGCost{ currentRecord.costSoFar + con->GetCost() };

				NodeRecord nextRecord;
				nextRecord.pNode = pNextNode;
				nextRecord.pConnection = con;
				nextRecord.costSoFar = newGCost;
				nextRecord.estimatedTotalCost = newGCost + GetHeuristicCost(pNextNode, pGoalNode);

				// 2.D Check if any of those connections lead to a node already on the closed list.
				bool found{ false };
				int i{};

				while (!found && i < (int)closedList.size())
				{
					if (closedList[i].pNode == nextRecord.pNode) found = true; 
					else i++;
				}

				if (found)
				{
					// If so, continue
					if (closedList[i].costSoFar < nextRecord.costSoFar) continue;
					else
					{
						// Else, remove it from the closedList (so it can be replaced)
						closedList[i] = closedList.back();
						closedList.pop_back();
					}
				}
				else
				{
					// 2.E Check if any of those connections lead to a node already on the open list
					bool found{ false };
					int i{};

					while (!found && i < (int)openList.size())
					{
						if (openList[i].pNode == nextRecord.pNode) found = true;
						else i++;
					}

					if (found)
					{
						// If so, continue
						if (openList[i].costSoFar < nextRecord.costSoFar) continue;
						else
						{
							// Else, remove it from the closedList (so it can be replaced)
							openList[i] = openList.back();
							openList.pop_back();
						}
					}
				}

				// 2.F At this point any expensive connection should be removed (if it existed)
				//     We create a new nodeRecord and add it to the openList
				openList.push_back(nextRecord);
			}
			closedList.push_back(currentRecord);
		}

		// 3 Reconstruct path from last connection to start node
		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);
			bool found{ false };
			int i{};

			while (!found && i < (int)closedList.size()) 
			{
				if (closedList[i].pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom())) found = true;
				else i++;
			}
				
			currentRecord = closedList[i];
		}

		// Add start node
		path.push_back(pStartNode);

		// Reverse path
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}